/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <queue>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <wayland-client.h>

#ifndef WAYLAND_GUILIB_RESOLUTION_H_INCLUDED
#define WAYLAND_GUILIB_RESOLUTION_H_INCLUDED
#include "guilib/Resolution.h"
#endif

#ifndef WAYLAND_GUILIB_GUI3D_H_INCLUDED
#define WAYLAND_GUILIB_GUI3D_H_INCLUDED
#include "guilib/gui3d.h"
#endif


#ifndef WAYLAND_UTILS_STRINGUTILS_H_INCLUDED
#define WAYLAND_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif


#ifndef WAYLAND_WINDOWING_DLLWAYLANDCLIENT_H_INCLUDED
#define WAYLAND_WINDOWING_DLLWAYLANDCLIENT_H_INCLUDED
#include "windowing/DllWaylandClient.h"
#endif

#ifndef WAYLAND_WINDOWING_DLLXKBCOMMON_H_INCLUDED
#define WAYLAND_WINDOWING_DLLXKBCOMMON_H_INCLUDED
#include "windowing/DllXKBCommon.h"
#endif


#ifndef WAYLAND_CALLBACK_H_INCLUDED
#define WAYLAND_CALLBACK_H_INCLUDED
#include "Callback.h"
#endif

#ifndef WAYLAND_COMPOSITOR_H_INCLUDED
#define WAYLAND_COMPOSITOR_H_INCLUDED
#include "Compositor.h"
#endif

#ifndef WAYLAND_DISPLAY_H_INCLUDED
#define WAYLAND_DISPLAY_H_INCLUDED
#include "Display.h"
#endif

#ifndef WAYLAND_OUTPUT_H_INCLUDED
#define WAYLAND_OUTPUT_H_INCLUDED
#include "Output.h"
#endif

#ifndef WAYLAND_REGISTRY_H_INCLUDED
#define WAYLAND_REGISTRY_H_INCLUDED
#include "Registry.h"
#endif

#ifndef WAYLAND_REGION_H_INCLUDED
#define WAYLAND_REGION_H_INCLUDED
#include "Region.h"
#endif

#ifndef WAYLAND_SHELL_H_INCLUDED
#define WAYLAND_SHELL_H_INCLUDED
#include "Shell.h"
#endif


#ifndef WAYLAND_WINDOWING_WAYLANDPROTOCOL_H_INCLUDED
#define WAYLAND_WINDOWING_WAYLANDPROTOCOL_H_INCLUDED
#include "windowing/WaylandProtocol.h"
#endif

#ifndef WAYLAND_XBMCCONNECTION_H_INCLUDED
#define WAYLAND_XBMCCONNECTION_H_INCLUDED
#include "XBMCConnection.h"
#endif


#ifndef WAYLAND_WINDOWING_WAYLAND_WAYLAND11EVENTQUEUESTRATEGY_H_INCLUDED
#define WAYLAND_WINDOWING_WAYLAND_WAYLAND11EVENTQUEUESTRATEGY_H_INCLUDED
#include "windowing/wayland/Wayland11EventQueueStrategy.h"
#endif

#ifndef WAYLAND_WINDOWING_WAYLAND_WAYLAND12EVENTQUEUESTRATEGY_H_INCLUDED
#define WAYLAND_WINDOWING_WAYLAND_WAYLAND12EVENTQUEUESTRATEGY_H_INCLUDED
#include "windowing/wayland/Wayland12EventQueueStrategy.h"
#endif


namespace xbmc
{
namespace wayland
{
/* A RemoteGlobalInterface just describes a pure virtual class
 * which is an observer for Private::OnGlobalInterfaceAvailable
 * when a new global interface appears on the compositor for the
 * client to bind to */
class RemoteGlobalInterface
{
public:

  virtual ~RemoteGlobalInterface() {}
  
  struct Constructor
  {
    const char *interfaceName;
    RemoteGlobalInterface *interface;
  };
  
  virtual void OnObjectAvailable(uint32_t name, uint32_t version) = 0;
};

/* A GlobalInterface is a simple implementation of a
 * RemoteGlobalInterface with OnObjectAvailable already implemented.
 * 
 * Users of this class are able to inject a custom function to be
 * observe whenever an object becomes avilable to this class and can
 * get a queue of all names available for the particular interface
 * this class is asked to observe. The object name is effectively
 * just a number referring to the global object ID on the compositor
 * side that can be bound to by a client. */ 
class GlobalInterface :
  public RemoteGlobalInterface
{
public:

  typedef boost::function<void(uint32_t version)> AvailabilityHook;

protected:

  GlobalInterface(const AvailabilityHook &hook) :
    m_hook(hook)
  {
  }
  
  GlobalInterface()
  {
  }

  std::queue<uint32_t> & ObjectsAvailable(uint32_t minimum);

private:

  virtual void OnObjectAvailable(uint32_t name, uint32_t version);

  std::queue<uint32_t> m_availableNames;
  uint32_t m_version;
  AvailabilityHook m_hook;
};

/* A WaylandGlobalObject is a more complete implementation of
 * GlobalInterface. It observes for when objects become available
 * and provides a method to fetch-and-bind names on-demand.
 * 
 * Once FetchPending is called, the name is removed from the pending
 * queue of GlobalInterface, bound to the interface provided and
 * returned as an Implementation */
template <typename Implementation>
class WaylandGlobalObject :
  public GlobalInterface
{
public:

  WaylandGlobalObject(uint32_t minimum,
                      struct wl_interface **interface) :
    GlobalInterface(),
    m_minimum(minimum),
    m_interface(interface)
  {
  }
  
  WaylandGlobalObject(uint32_t minimum,
                      struct wl_interface **interface,
                      const AvailabilityHook &hook) :
    GlobalInterface(hook),
    m_minimum(minimum),
    m_interface(interface)
  {
  }
  
  Implementation * FetchPending(Registry &registry);

private:

  uint32_t m_minimum;
  struct wl_interface **m_interface;
};

/* A StoredGlobalInterface is an implementation of RemoteGlobalInterface
 * which composes a WaylandGlobalObject internally.
 * 
 * This class takes a factory function to produce an Implementation *
 * (usually a wrapper class of some sort) from a WaylandImplementation *
 * (usually the defined wayland proxy object in the autogenerated
 *  protocol).  It also has an alternate constructor that allows
 * outside users to observe when an object has initially become available
 * which is useful if it needs to be registered right away.
 */
template <typename Implementation, typename WaylandImplementation>
class StoredGlobalInterface :
  public RemoteGlobalInterface
{
public:

  typedef boost::function<Implementation * (WaylandImplementation *)> Factory;
  typedef std::vector<boost::shared_ptr<Implementation> > Implementations;
  
  /* Factory must be capable of returning a new Implementation *
   * corresponding to a WaylandImplementation *. This is usually
   * a wrapper class around that wayland object */
  StoredGlobalInterface(const Factory &factory,
                        uint32_t minimum,
                        struct wl_interface **interface) :
    m_waylandObject(minimum, interface),
    m_factory(factory)
  {
  }

  StoredGlobalInterface(const Factory &factory,
                        uint32_t minimum,
                        struct wl_interface **interface,
                        const GlobalInterface::AvailabilityHook &hook) :
    m_waylandObject(minimum, interface, hook),
    m_factory(factory)
  {
  }
  
  ~StoredGlobalInterface()
  {
  }

  /* These two functions always return constant values, although
   * they might be required to create new Implementation objects
   * by binding a global wayland object and wrapping it when they
   * are initially called.
   * 
   * The first function always returns the first-available interface,
   * the second function always returns the list of available global
   * objects which have that interface */ 
  Implementation & GetFirst(Registry &registry);
  Implementations & Get(Registry &registry);

private:

  void OnObjectAvailable(uint32_t name,
                         uint32_t version);

  WaylandGlobalObject<WaylandImplementation> m_waylandObject;
  Factory m_factory;
  Implementations m_implementations;
};

class XBMCConnection::Private :
  public IWaylandRegistration
{
public:

  Private(IDllWaylandClient &clientLibrary,
          IDllXKBCommon &xkbCommonLibrary,
          EventInjector &eventInjector);
  ~Private();

  /* Synchronization entry point - call this function to issue a
   * wl_display.sync request to the server. All this does is cause
   * the server to send back an event that acknowledges the receipt
   * of the request. However, it is useful in a number of circumstances
   * - all request processing in wayland is sequential and guarunteed
   * to be in the same order as requests were made. That means that
   * once the event is received from the server, it is guarunteed
   * that all requests made prior to the sync request have finished
   * processing on the server and events have been issued to us.
   * 
   * Do not call this from a non-main thread. The main thread may be
   * waiting for a wl_display.sync event to be coming through and this
   * function will merely spin until synchronized == true, for which
   * a non-main thread may be responsible for setting as true */
  void WaitForSynchronize();
  
  wayland::Display & Display();
  wayland::Compositor & Compositor();
  wayland::Shell & Shell();
  wayland::Output & Output();
  
private:

  IDllWaylandClient &m_clientLibrary;
  IDllXKBCommon &m_xkbCommonLibrary;
  
  EventInjector m_eventInjector;

  /* Do not call this from a non-main thread. The main thread may be
   * waiting for a wl_display.sync event to be coming through and this
   * function will merely spin until synchronized == true, for which
   * a non-main thread may be responsible for setting as true */
  void Synchronize();

  /* Synchronization logic - these variables should not be touched
   * outside the scope of WaitForSynchronize() */
  bool synchronized;
  boost::scoped_ptr<Callback> synchronizeCallback;
  
  bool OnGlobalInterfaceAvailable(uint32_t name,
                                  const char *interface,
                                  uint32_t version);

  void InjectSeat();

  boost::scoped_ptr<wayland::Display> m_display;
  boost::scoped_ptr<wayland::Registry> m_registry;
  
  StoredGlobalInterface<wayland::Compositor, struct wl_compositor> m_compositor;
  StoredGlobalInterface<wayland::Shell, struct wl_shell> m_shell;
  WaylandGlobalObject<struct wl_seat> m_seat;
  StoredGlobalInterface<wayland::Output, struct wl_output> m_outputs;
  
  boost::scoped_ptr<events::IEventQueueStrategy> m_eventQueue;
};
}
}

namespace xw = xbmc::wayland;
namespace xwe = xbmc::wayland::events;
namespace xwe = xbmc::wayland::events;

void
xw::GlobalInterface::OnObjectAvailable(uint32_t name,
                                       uint32_t version)
{
  m_availableNames.push(name);
  m_version = version;
  
  if (!m_hook.empty())
    m_hook(m_version);
}

std::queue<uint32_t> &
xw::GlobalInterface::ObjectsAvailable(uint32_t minimum)
{
  if (m_version < minimum)
  {
    std::stringstream ss;
    ss << "Interface version at least "
       << minimum
       << " is not available"
       << " (less than version: "
       << m_version
       << ")";
    throw std::runtime_error(ss.str());
  }
  
  return m_availableNames;
}

template<typename Implementation>
Implementation *
xw::WaylandGlobalObject<Implementation>::FetchPending(Registry &registry)
{
  /* Pop any new names and bind them */
  std::queue<uint32_t> &availableObjects(ObjectsAvailable(m_minimum));
  if (!availableObjects.empty())
  {
    uint32_t name = availableObjects.front();
    Implementation *proxy =
      registry.Bind<Implementation *>(name,
                                      m_interface,
                                      m_minimum);
    availableObjects.pop();
    return proxy;
  }
  
  return NULL;
}

template<typename Implementation, typename WaylandImplementation>
void
xw::StoredGlobalInterface<Implementation, WaylandImplementation>::OnObjectAvailable(uint32_t name, uint32_t version)
{
  RemoteGlobalInterface &rgi =
    static_cast<RemoteGlobalInterface &>(m_waylandObject);
  rgi.OnObjectAvailable(name, version);
}

template <typename Implementation, typename WaylandImplementation>
typename xw::StoredGlobalInterface<Implementation, WaylandImplementation>::Implementations &
xw::StoredGlobalInterface<Implementation, WaylandImplementation>::Get(Registry &registry)
{
  /* Instantiate any pending objects with this interface and then
   * return the available implementations */
  WaylandImplementation *proxy =
    m_waylandObject.FetchPending(registry);
  
  while (proxy)
  {
    boost::shared_ptr<Implementation> instance(m_factory(proxy));
    m_implementations.push_back(instance);
    proxy = m_waylandObject.FetchPending(registry);
  }

  /* Calling Get() before we've received any notification that
   * objects are available is a runtime_error and will be thrown as
   * such.
   * 
   * Calling code that wishes to avoid this error should either
   * insert a synchronization point right after creating the object
   * registry or register a callback using the second constructor
   * to observe when the object has become available before calling
   * Get(). */
  if (m_implementations.empty())
    throw std::runtime_error("Remote interface not available");
  
  return m_implementations;
}

template <typename Implementation, typename WaylandImplementation>
Implementation &
xw::StoredGlobalInterface<Implementation, WaylandImplementation>::GetFirst(xw::Registry &registry)
{
  return *(Get(registry)[0]);
}

namespace
{
const std::string CompositorName("wl_compositor");
const std::string ShellName("wl_shell");
const std::string SeatName("wl_seat");
const std::string OutputName("wl_output");

/* These are functions that satisfy the definition of a "Factory"
 * for the purposes of StoredGlobalInterface */
xw::Compositor * CreateCompositor(struct wl_compositor *compositor,
                                  IDllWaylandClient *clientLibrary)
{
  return new xw::Compositor(*clientLibrary, compositor);
}

xw::Output * CreateOutput(struct wl_output *output,
                          IDllWaylandClient *clientLibrary)
{
  return new xw::Output(*clientLibrary, output);
}

xw::Shell * CreateShell(struct wl_shell *shell,
                        IDllWaylandClient *clientLibrary)
{
  return new xw::Shell(*clientLibrary, shell);
}

bool ConstructorMatchesInterface(const xw::RemoteGlobalInterface::Constructor &constructor,
                                 const char *interface)
{
  return std::strcmp(constructor.interfaceName,
                     interface) < 0;
}

const unsigned int RequestedCompositorVersion = 1;
const unsigned int RequestedShellVersion = 1;
const unsigned int RequestedOutputVersion = 1;
const unsigned int RequestedSeatVersion = 1;

/* A deficiency in the client library in wayland versions prior to
 * 1.2 means that there is divergent behaviour between versions here
 * and this is explicitly expressed and encapsulated in these two
 * strategies.
 * 
 * Because xbmc uses a game-loop, it is expected that no operation
 * should block the main thread. This includes any operations to
 * read the window system event queue. The main thread might be blocked
 * for a prolonged period in the situation where the main xbmc surface
 * is not visible, because the screen lock is active or another
 * surface is obstructing it. When the main thread becomes blocked,
 * it means that xbmc isn't able to start or stop any background jobs,
 * which could interrupt library updates which occurr on idle or
 * other such operations.
 * 
 * However, wayland versions prior to 1.2 had the expectation that
 * clients expected to block if there were no incoming compositor
 * events because it is part of wayland's design that the compositor
 * is responsible for sending the events to drive a client's render
 * and input loop. As such, on wayland <= 1.1, the expectation is that
 * compositor event read and dispatch occurrs in the same thread and
 * on wayland >= 1.2 the expectation is that these operations can
 * occurr in multiple threads.
 * 
 * The following table illustrates these differences:
 * 
 * ---------------------------------------------------------------------
 * | Wayland | Thread that  | Thread that | Thread that   | Strategy   |
 * | Version | Reads happen | wrappers    | flush happens | Object     |
 * |         | in           | operate in  |               |            |
 * |         |              | in          |               |            |
 * ---------------------------------------------------------------------
 * | <= 1.1  | Poll Thread  | Poll Thread | Main Thread   | xw::versio-|
 * |         |              |             |               | n11::Event-|
 * |         |              |             |               | QueueStrat-|
 * |         |              |             |               | egy        |
 * ---------------------------------------------------------------------
 * | >= 1.2  | Poll Thread  | Main Thread | Main Thread   | xw::versio-|
 * |         |              |             |               | n12::Event-|
 * |         |              |             |               | QueueStrat-|
 * |         |              |             |               | egy        |
 * ---------------------------------------------------------------------
 * 
 * The reason why it is different between the two versions it that it
 * is generally desirable that the operation of all the wrapper objects
 * occurr in the main thread, because there's less overhead in having
 * to allocate temporary storage for their results in a queue so that
 * they can be re-dispatched later. The plan is to eventually deprecate
 * and remove support for wayland versions <= 1.1.
 */
xwe::IEventQueueStrategy *
EventQueueForClientVersion(IDllWaylandClient &clientLibrary,
                           struct wl_display *display)
{
  /* TODO: Test for wl_display_read_events / wl_display_prepare_read */
  const bool version12 =
    clientLibrary.wl_display_read_events_proc() &&
    clientLibrary.wl_display_prepare_read_proc();
  if (version12)
    return new xw::version_12::EventQueueStrategy(clientLibrary,
                                                  display);
  else
    return new xw::version_11::EventQueueStrategy(clientLibrary,
                                                  display);
}
}

/* Creating a new xbmc::wayland::XBMCConnection effectively creates
 * a new xbmc::wayland::Display object, which in turn will connect
 * to the running wayland compositor and encapsulate the return value
 * from the client library. Then it creates a new
 * xbmc::wayland::Registry object which is responsible for managing
 * all of the global objects on the wayland connection that we might
 * want to use. On creation of this object, a request is sent to
 * the compositor to send back an event for every available global
 * object. Once we know which objects exist, we can easily
 * bind to them.
 * 
 * The WaitForSynchronize call at the end of the constructor is
 * important. Once we make a request to the server for all of the
 * available global objects, we need to know what they all are
 * by the time this constructor finishes running so that the
 * object will be complete. The only way to do that is to know
 * when our wl_registry.add_listener request has finished processing
 * on both the server and client side
 */
xw::XBMCConnection::Private::Private(IDllWaylandClient &clientLibrary,
                                     IDllXKBCommon &xkbCommonLibrary,
                                     EventInjector &eventInjector) :
  m_clientLibrary(clientLibrary),
  m_xkbCommonLibrary(xkbCommonLibrary),
  m_eventInjector(eventInjector),
  m_display(new xw::Display(clientLibrary)),
  m_registry(new xw::Registry(clientLibrary,
                              m_display->GetWlDisplay(),
                              *this)),
  m_compositor(boost::bind(CreateCompositor, _1, &m_clientLibrary),
               RequestedCompositorVersion,
               clientLibrary.Get_wl_compositor_interface()),
  m_shell(boost::bind(CreateShell, _1, &m_clientLibrary),
          RequestedShellVersion,
          clientLibrary.Get_wl_shell_interface()),
  m_seat(RequestedSeatVersion,
         clientLibrary.Get_wl_seat_interface(),
         boost::bind(&Private::InjectSeat, this)),
  m_outputs(boost::bind(CreateOutput, _1, &m_clientLibrary),
            RequestedOutputVersion,
            clientLibrary.Get_wl_output_interface()),
  m_eventQueue(EventQueueForClientVersion(m_clientLibrary,
                                          m_display->GetWlDisplay()))
{
  /* Tell CWinEvents what our event queue is. That way
   * CWinEvents::MessagePump is now able to dispatch events from
   * the display whenever it is called */ 
  (*m_eventInjector.setEventQueue)(*(m_eventQueue.get()));
	
  /* Wait only for the globals to appear, we will wait for
   * initialization upon binding them */
  WaitForSynchronize();
}

void
xw::XBMCConnection::Private::InjectSeat()
{
  /* When the seat becomes available and bound, let CWinEventsWayland
   * know about it so that it can wrap it and query it for more
   * information about input devices */
  struct wl_seat *seat = m_seat.FetchPending(*m_registry);
  (*m_eventInjector.setWaylandSeat)(m_clientLibrary,
                                    m_xkbCommonLibrary,
                                    seat);
}

xw::XBMCConnection::Private::~Private()
{
  (*m_eventInjector.destroyWaylandSeat)();
  (*m_eventInjector.destroyEventQueue)();
}

xw::XBMCConnection::XBMCConnection(IDllWaylandClient &clientLibrary,
                                   IDllXKBCommon &xkbCommonLibrary,
                                   EventInjector &eventInjector) :
  priv(new Private (clientLibrary, xkbCommonLibrary, eventInjector))
{
}

/* A defined destructor is required such that
 * boost::scoped_ptr<Private>::~scoped_ptr is generated here */
xw::XBMCConnection::~XBMCConnection()
{
}

xw::Display &
xw::XBMCConnection::Private::Display()
{
  return *m_display;
}

xw::Compositor &
xw::XBMCConnection::Private::Compositor()
{
  return m_compositor.GetFirst(*m_registry);
}

xw::Shell &
xw::XBMCConnection::Private::Shell()
{
  return m_shell.GetFirst(*m_registry);
}

xw::Output &
xw::XBMCConnection::Private::Output()
{
  xw::Output &output(m_outputs.GetFirst(*m_registry));
  
  /* Wait for synchronize upon lazy-binding the first output
   * and then check if we got any modes */
  WaitForSynchronize();
  if (output.AllModes().empty())
  {
    std::stringstream ss;
    ss << "No modes detected on first output";
    throw std::runtime_error(ss.str());
  }
  return output;
}

/* Once an object becomes available, we need to take note of that
 * fact and store its interface information somewhere. We then
 * call a function to indicate to any interested observer that the
 * object is available and can be bound to. Callers might not do this
 * right away. */
bool
xw::XBMCConnection::Private::OnGlobalInterfaceAvailable(uint32_t name,
                                                        const char *interface,
                                                        uint32_t version)
{
  /* A boost::array is effectively immutable so we can leave out
   * const here */
  typedef boost::array<RemoteGlobalInterface::Constructor, 4> ConstructorArray;

  
  /* Not static, as the pointers here may change in cases where
   * Private is re-constructed.
   * 
   * These are sorted into alphabetical order so that we can do
   * a simple binary search for them. */
  ConstructorArray constructors =
  {
    {
      { CompositorName.c_str(), &m_compositor },
      { OutputName.c_str(), &m_outputs },
      { SeatName.c_str(), &m_seat },
      { ShellName.c_str(), &m_shell }
    }
  };

  /* Simple binary search for a known object constructor that matches
   * this interface */
  ConstructorArray::iterator it(std::lower_bound(constructors.begin(),
                                                 constructors.end(),
                                                 interface,
                                                 ConstructorMatchesInterface));
  if (it != constructors.end() &&
      strcmp(it->interfaceName, interface) == 0)
  {
    it->interface->OnObjectAvailable(name, version);
    return true;
  }
  
  return false;
}

void xw::XBMCConnection::Private::WaitForSynchronize()
{
  boost::function<void(uint32_t)> func(boost::bind(&Private::Synchronize,
                                                   this));
  
  synchronized = false;
  synchronizeCallback.reset(new xw::Callback(m_clientLibrary,
                                             m_display->Sync(),
                                             func));

  /* For version 1.1 event queues the effect of this is going to be
   * a spin-wait. That's not exactly ideal, but we do need to
   * continuously flush the event queue */
  while (!synchronized)
    (*m_eventInjector.messagePump)();
}

void xw::XBMCConnection::Private::Synchronize()
{
  synchronized = true;
  synchronizeCallback.reset();
}

namespace
{
void ResolutionInfoForMode(const xw::Output::ModeGeometry &mode,
                           RESOLUTION_INFO &res)
{
  res.iWidth = mode.width;
  res.iHeight = mode.height;
  
  /* The refresh rate is given as an integer in the second exponent
   * so we need to divide by 100.0f to get a floating point value */
  res.fRefreshRate = mode.refresh / 100.0f;
  res.dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
  res.iScreen = 0;
  res.bFullScreen = true;
  res.iSubtitles = static_cast<int>(0.965 * res.iHeight);
  res.fPixelRatio = 1.0f;
  res.iScreenWidth = res.iWidth;
  res.iScreenHeight = res.iHeight;
  res.strMode = StringUtils::Format("%dx%d @ %.2fp",
                     res.iScreenWidth,
                     res.iScreenHeight,
                     res.fRefreshRate);
}
}

void
xw::XBMCConnection::CurrentResolution(RESOLUTION_INFO &res) const
{
  /* Supporting only the first output device at the moment */
  const xw::Output::ModeGeometry &current(priv->Output().CurrentMode());
  
  ResolutionInfoForMode(current, res);
}

void
xw::XBMCConnection::PreferredResolution(RESOLUTION_INFO &res) const
{
  /* Supporting only the first output device at the moment */
  const xw::Output::ModeGeometry &preferred(priv->Output().PreferredMode());
  ResolutionInfoForMode(preferred, res);
}

void
xw::XBMCConnection::AvailableResolutions(std::vector<RESOLUTION_INFO> &resolutions) const
{
  /* Supporting only the first output device at the moment */
  xw::Output &output(priv->Output());
  const std::vector<xw::Output::ModeGeometry> &m_modes(output.AllModes());

  for (std::vector<xw::Output::ModeGeometry>::const_iterator it = m_modes.begin();
       it != m_modes.end();
       ++it)
  {
    resolutions.push_back(RESOLUTION_INFO());
    RESOLUTION_INFO &back(resolutions.back());
    
    ResolutionInfoForMode(*it, back);
  }
}

EGLNativeDisplayType *
xw::XBMCConnection::NativeDisplay() const
{
  return priv->Display().GetEGLNativeDisplay();
}

xw::Compositor &
xw::XBMCConnection::GetCompositor()
{
  return priv->Compositor();
}

xw::Shell &
xw::XBMCConnection::GetShell()
{
  return priv->Shell();
}

xw::Output &
xw::XBMCConnection::GetFirstOutput()
{
  return priv->Output();
}
