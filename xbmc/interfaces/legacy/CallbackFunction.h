/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "AddonClass.h"

namespace XBMCAddon
{
  /**
   * <p>This is the parent class for the class templates that hold
   *   a callback. A callback is essentially a templatized
   *   functor (functoid?) for a call to a member function.</p>
   *
   * <p>This class combined with the attending CallbackHandlers should make
   *  sure that the AddonClass isn't in the midst of deallocating when 
   *  the callback executes. In this way the Callback class acts as
   *  a weak reference.</p>
   */
  class Callback : public AddonClass 
  { 
  protected:
    AddonClass* addonClassObject;
    Callback(AddonClass* _object, const char* classname) : AddonClass(classname), addonClassObject(_object) {}

  public:
    virtual void executeCallback() = 0;
    virtual ~Callback();

    AddonClass* getObject() { return addonClassObject; }
  };

  struct cb_null_type {};

  // stub type template to be partial specialized
  template<typename M = cb_null_type, typename T1 = cb_null_type, 
           typename T2 = cb_null_type, typename T3 = cb_null_type, 
           typename T4 = cb_null_type, typename Extraneous = cb_null_type> 
  class CallbackFunction {};
  
  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes no parameters.
   */
  template<class M> class CallbackFunction<M, cb_null_type, cb_null_type, cb_null_type, cb_null_type, cb_null_type> : public Callback
  { 
  public:
    typedef void (M::*MemberFunction)();

  protected:
    MemberFunction meth;
    M* obj;

  public:
    CallbackFunction(M* object, MemberFunction method) : 
      Callback(object, "CallbackFunction<M>"), meth(method), obj(object) {}

    virtual ~CallbackFunction() { deallocating(); }

    virtual void executeCallback() { TRACE; ((*obj).*(meth))(); }
  };

  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes one parameter.
   */
  template<class M, typename P1> class CallbackFunction<M,P1, cb_null_type, cb_null_type, cb_null_type, cb_null_type> : public Callback
  { 
  public:
    typedef void (M::*MemberFunction)(P1);

  protected:
    MemberFunction meth;
    M* obj;
    P1 param;

  public:
    CallbackFunction(M* object, MemberFunction method, P1 parameter) : 
      Callback(object, "CallbackFunction<M,P1>"), meth(method), obj(object),
      param(parameter) {}

    virtual ~CallbackFunction() { deallocating(); }

    virtual void executeCallback() { TRACE; ((*obj).*(meth))(param); }
  };

  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes one parameter
   *  that can be held in an AddonClass::Ref
   */
  template<class M, typename P1> class CallbackFunction<M,AddonClass::Ref<P1>, cb_null_type, cb_null_type, cb_null_type, cb_null_type> : public Callback
  { 
  public:
    typedef void (M::*MemberFunction)(P1*);

  protected:
    MemberFunction meth;
    M* obj;
    AddonClass::Ref<P1> param;

  public:
    CallbackFunction(M* object, MemberFunction method, P1* parameter) : 
      Callback(object, "CallbackFunction<M,P1>"), meth(method), obj(object),
      param(parameter) {}

    virtual ~CallbackFunction() { deallocating(); }

    virtual void executeCallback() { TRACE; ((*obj).*(meth))(param); }
  };


  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes two parameters.
   */
  template<class M, typename P1, typename P2> class CallbackFunction<M,P1,P2, cb_null_type, cb_null_type, cb_null_type> : public Callback
  { 
  public:
    typedef void (M::*MemberFunction)(P1,P2);

  protected:
    MemberFunction meth;
    M* obj;
    P1 param1;
    P2 param2;

  public:
    CallbackFunction(M* object, MemberFunction method, P1 parameter, P2 parameter2) : 
      Callback(object, "CallbackFunction<M,P1,P2>"), meth(method), obj(object),
      param1(parameter), param2(parameter2) {}

    virtual ~CallbackFunction() { deallocating(); }

    virtual void executeCallback() { TRACE; ((*obj).*(meth))(param1,param2); }
  };


}


