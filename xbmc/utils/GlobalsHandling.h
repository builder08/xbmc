/*
 *      Copyright (C) 2005-2011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "utils/ReferenceCounting.h"

/**
 * This file contains the pattern for moving "globals" from the BSS Segment to the heap.
 * A note on usage of this pattern for globals replacement:
 *
 * This pattern uses a singleton pattern and some compiler/cpreprocessor sugar to allow 
 * "global" variables to be lazy instantiated and initialized and moved from the BSS segment 
 * to the heap (that is, they are instantiated on the heap when they are first used rather 
 * than relying on the startup code to initialize the BSS segment). This eliminates the 
 * problem associated with global variable dependencies across compilation units.
 *
 * Reference counting from the BSS segment is used to destruct these globals at the time the 
 * last compilation unit that knows about it is finalized by the post-main shutdown. The book 
 * keeping is done by smuggling a smart pointer into every file that references a particular 
 * "global class" through the use of a 'static' declaration of an instance of that smart 
 * pointer in the header file of the global class (did you ever think you'd see a file scope 
 * 'static' variable in a header file - on purpose?)
 *
 * For more details see xbmc/utils/ReferenceCounting.h/cpp.
 *
 * There are two different ways to use this pattern when replacing global variables.
 * The selection of which one to use depends on whether or not there is a possiblity
 * that the code in the .cpp file for the global can be executed from a static method
 * somewhere. This may take some explanation.
 *
 * The (at least) two ways to do this:
 *
 * 1) You can use the reference object Referenced::ref to access the global variable.
 *
 * This would be the preferred means since it is (very slightly) more efficent than
 * the alternative. To use this pattern you replace standard static references to
 * the global with access through the reference. If you use the c-preprocessor to
 * do this for you can put the following code in the header file where the global's
 * class is declared:
 *
 * static xbmcutil::Referenced::ref<GlobalVariableClass> g_globalVariableRef(xbmcutil::GlobalsSingleton<GlobalVariableClass>::getInstance);
 * #define g_globalVariable (g_globalVariableRef.getRef())
 *
 * Note what this does. In every file that includes this header there will be a *static*
 * instance of the Referenced::ref<GlobalVariableClass> smart pointer. This effectively
 * reference counts the singleton from every compilation unit (ie, object code file that
 * results from a compilation of a .c/.cpp file) that references this global directly.
 *
 * There is a problem with this, however. Keep in mind that the instance of the smart pointer
 * (being in the BSS segment of the compilation unit) is ITSELF an object that depends on
 * the BSS segment initialization in order to be initialized with an instance from the
 * Singleton. That means, depending on the code structure, it is possible to get into a
 * circumstance where the above #define could be exercised PRIOR TO the setting of the
 * value of the smart pointer.
 *
 * Some reflection on this should lead you to the conclusion that the only way for this to
 * happen is if access to this global can take place through a static/global method, directly
 * or indirectly (ie, the static/global method can call another method that uses the
 * reference), where that static is called from initialization code exercised prior to
 * the start of 'main.'
 *
 * Because of the "indirectly" in the above statement, this situation can be difficult to
 * determine beforehand.
 *
 * 2) Alternately, when you KNOW that the global variable can suffer from the above described
 * problem, you can restrict all access to the varaible to the singleton by changing
 * the #define to:
 *
 * #define g_globalVariable (*(xbmcutil::Singleton<GlobalVariableClass>::getInstance()))
 *
 * A few things to note about this. First, this separates the reference counting aspect
 * from the access aspect of this solution. The smart pointers are no longer used for
 * reference, only for access. Secondly, all access is through the Singleton directly
 * so there is no reliance on the state of the BSS segment for the code to operate
 * correctly.
 *
 * This solution is required for g_Windowing because it's accessed (both directly and
 * indirectly) from the static methods of CLog which are called repeatedly from
 * code exercised during the initialization of the BSS segment.
 */

namespace xbmcutil
{

  /**
   * Currently THIS IS NOT THREAD SAFE! Why not just add a lock you ask?
   *  Because this singleton is used to initialize global variables and
   *  there is an issue with having the lock used prior to its 
   *  initialization. No matter what, if this class is used as a replacement
   *  for global variables there's going to be a race condition if it's used
   *  anywhere else. So currently this is the only prescribed use.
   *
   * Therefore this hack depends on the fact that compilation unit global/static 
   *  initialization is done in a single thread.
   *
   * In the spirit of making changes incrementally I've opted to not add Atomic*
   *  locking to this class yet. It doesn't need it (yet) as it's only called
   *  from the static initialization thread prior to main.
   */
  template <class T> class GlobalsSingleton
  {
    static T* instance;
  public:
    static T* getInstance()
    {
      if (instance == NULL)
        instance = new T;
      return instance;
    }
  };

  template <class T> T* GlobalsSingleton<T>::instance;
}

/**
 * For pattern (2) above, you can use the following macro. This pattern is safe to
 * use in all cases but may be very slightly less efficent. 
 *
 * Also, you must also use a #define to replace the actual global variable since 
 * there's no way to use a macro to add a #define. An example would be:
 *
 * XBMC_GLOBAL_REF(CWinSystemWin32DX, g_Windowing);
 * #define g_Windowing XBMC_GLOBAL_USE(CWinSystemWin32DX)
 *
 */
#define XBMC_GLOBAL_REF(classname,g_variable) \
  static xbmcutil::Referenced::ref<classname> g_variable##Ref(xbmcutil::GlobalsSingleton<classname>::getInstance)

/**
 * This declares the actual use of the variable. It needs to be used in another #define
 * of the form:
 *
 * #define g_variable XBMC_GLOBAL_USE(classname)
 */
#define XBMC_GLOBAL_USE(classname) (*(xbmcutil::GlobalsSingleton<classname>::getInstance()))

/**
 * For pattern (1) above, you can use the following macro. WARNING: this should only 
 * be used when the global in question is never accessed, directly or indirectly, from
 * a static method called (again, directly or indirectly) durring startup or shutdown.
 */
#define XBMC_GLOBAL(classname,g_variable) \
  XBMC_GLOBAL_REF(classname,g_variable); \
  static classname & g_variable = g_variable##Ref.getRef()

