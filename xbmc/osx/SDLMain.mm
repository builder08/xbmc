/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/
/*
  SDLMain.m and SDLMain.h carry neither a copyright or license. They are in the
  public domain.
*/

#import "SDL/SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

#import "CocoaInterface.h"
//hack around problem with xbmc's typedef int BOOL
// and obj-c's typedef unsigned char BOOL
#define BOOL XBMC_BOOL 
#import "utils/StdString.h"
#import "PlatformDefs.h"
#import "ApplicationMessenger.h"
#import "storage/osx/DarwinStorageProvider.h"
#undef BOOL

// For some reaon, Apple removed setAppleMenu from the headers in 10.4,
// but the method still is there and works. To avoid warnings, we declare
// it ourselves here.
@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end

// Use this flag to determine whether we use CPS (docking) or not
#define		SDL_USE_CPS		1
#ifdef SDL_USE_CPS
// Portions of CPS.h
typedef struct CPSProcessSerNum
{
	UInt32		lo;
	UInt32		hi;
} CPSProcessSerNum;

extern "C" {
extern OSErr	CPSGetCurrentProcess(CPSProcessSerNum *psn);
extern OSErr 	CPSEnableForegroundOperation(CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr	CPSSetFrontProcess(CPSProcessSerNum *psn);
}
#endif /* SDL_USE_CPS */

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;
static BOOL   gCalledAppMainline = FALSE;

static NSString *getApplicationName(void)
{
  NSDictionary *dict;
  NSString *appName = 0;

  // Determine the application name
  dict = (NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
  if (dict)
    appName = [dict objectForKey: @"CFBundleName"];

  if (![appName length])
    appName = [[NSProcessInfo processInfo] processName];

  return appName;
}
static void setupApplicationMenu(void)
{
  // warning: this code is very odd
  NSMenu *appleMenu;
  NSMenuItem *menuItem;
  NSString *title;
  NSString *appName;

  appName = getApplicationName();
  appleMenu = [[NSMenu alloc] initWithTitle:@""];

  // Add menu items
  title = [@"About " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Hide " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

  menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
  [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

  [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Quit " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];


  // Put menu into the menubar
  menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:appleMenu];
  [[NSApp mainMenu] addItem:menuItem];

  // Tell the application object that this is now the application menu
  [NSApp setAppleMenu:appleMenu];

  // Finally give up our references to the objects
  [appleMenu release];
  [menuItem release];
}

// Create a window menu
static void setupWindowMenu(void)
{
  NSMenu      *windowMenu;
  NSMenuItem  *windowMenuItem;
  NSMenuItem  *menuItem;

  windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

  // "Full/Windowed Toggle" item
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Full/Windowed Toggle" action:@selector(fullScreenToggle:) keyEquivalent:@"f"];
  [windowMenu addItem:menuItem];
  [menuItem release];

  // "Full/Windowed Toggle" item
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Float on Top" action:@selector(floatOnTopToggle:) keyEquivalent:@"t"];
  [windowMenu addItem:menuItem];
  [menuItem release];

  // "Minimize" item
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
  [windowMenu addItem:menuItem];
  [menuItem release];

  // Put menu into the menubar
  windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
  [windowMenuItem setSubmenu:windowMenu];
  [[NSApp mainMenu] addItem:windowMenuItem];

  // Tell the application object that this is now the window menu
  [NSApp setWindowsMenu:windowMenu];

  // Finally give up our references to the objects
  [windowMenu release];
  [windowMenuItem release];
}

@interface XBMCApplication : NSApplication
@end

@implementation XBMCApplication

// Called before the internal event loop has started running.
- (void) finishLaunching
{
  [super finishLaunching];
}

// Invoked from the Quit menu item
- (void)terminate:(id)sender
{
  // remove any notification handlers
  [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
  [[NSNotificationCenter defaultCenter] removeObserver:self];

  // Post a SDL_QUIT event
  SDL_Event event;
  event.type = SDL_QUIT;
  SDL_PushEvent(&event);
}

- (void)fullScreenToggle:(id)sender
{
  // Post an toggle full-screen event to the application thread.
  SDL_Event event;
  memset(&event, 0, sizeof(event));
  event.type = SDL_USEREVENT;
  event.user.code = TMSG_TOGGLEFULLSCREEN;
  SDL_PushEvent(&event);
}

- (void)floatOnTopToggle:(id)sender
{
  NSWindow* window = [[[NSOpenGLContext currentContext] view] window];
  if ([window level] == NSFloatingWindowLevel)
  {
    [window setLevel:NSNormalWindowLevel];
    [sender setState:NSOffState];
  }
  else
  {
    [window setLevel:NSFloatingWindowLevel];
    [sender setState:NSOnState];
  }
}

@end

// The main class of the application, the application's delegate
@implementation XBMCDelegate

// Set the working directory to the .app's parent directory
- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
  if (shouldChdir)
  {
    char parentdir[MAXPATHLEN];
    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, true, (UInt8 *)parentdir, MAXPATHLEN))
    {
      assert( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
		}
		CFRelease(url);
		CFRelease(url2);
  }
}

- (void) applicationWillTerminate: (NSNotification *) note
{
}

- (void) applicationWillResignActive:(NSNotification *) note
{
}

- (void) applicationWillBecomeActive:(NSNotification *) note
{
}

// To use Cocoa on secondary POSIX threads, your application must first detach
// at least one NSThread object, which can immediately exit. Some info says this
// is not required anymore, who knows ?
- (void) kickstartMultiThreaded:(id)arg;
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  // empty
  [pool release];
}

// Called after the internal event loop has started running.
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
  // enable multithreading, we should NOT have to do this but as we are mixing NSThreads/pthreads...
  if (![NSThread isMultiThreaded])
    [NSThread detachNewThreadSelector:@selector(kickstartMultiThreaded:) toTarget:self withObject:nil];

  // Set the working directory to the .app's parent directory
  [self setupWorkingDirectory:gFinderLaunch];

  [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
    selector:@selector(deviceDidMountNotification:)
    name:NSWorkspaceDidMountNotification
    object:nil];

  [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
    selector:@selector(deviceDidUnMountNotification:)
    name:NSWorkspaceDidUnmountNotification
    object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
    selector:@selector(windowDidMoveNotification:)
    name:NSWindowDidMoveNotification
    object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
    selector:@selector(windowDidReSizeNotification:)
    name:NSWindowDidResizeNotification
    object:nil];
      
  // We're going to manually manage the screensaver.
  setenv("SDL_VIDEO_ALLOW_SCREENSAVER", "1", true);

  // Hand off to main application code
  gCalledAppMainline = TRUE;

  // stop the main loop so we return to main (below) and can
  // call SDL_main there.
  [NSApp stop:nil];

  //post a NOP event, so the run loop actually stops
  //see http://www.cocoabuilder.com/archive/cocoa/219842-nsapp-stop.html
  NSEvent* event = [NSEvent otherEventWithType: NSApplicationDefined
    location: NSMakePoint(0,0)
    modifierFlags: 0
    timestamp: 0.0
    windowNumber: 0
    context: nil
    subtype: 0
    data1: 0
    data2: 0];
  //
  [NSApp postEvent: event atStart: true];
}

/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
  const char *temparg;
  size_t arglen;
  char *arg;
  char **newargv;

  // MacOS is passing command line args.
  if (!gFinderLaunch)
    return FALSE;

  // app has started, ignore this document.
  if (gCalledAppMainline)
    return FALSE;

  temparg = [filename UTF8String];
  arglen = SDL_strlen(temparg) + 1;
  arg = (char *) SDL_malloc(arglen);
  if (arg == NULL)
    return FALSE;

  newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
  if (newargv == NULL)
  {
    SDL_free(arg);
    return FALSE;
  }
  gArgv = newargv;

  SDL_strlcpy(arg, temparg, arglen);
  gArgv[gArgc++] = arg;
  gArgv[gArgc] = NULL;

  return TRUE;
}

- (void) deviceDidMountNotification:(NSNotification *) note 
{
  // calling into c++ code, need to use autorelease pools
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  CDarwinStorageProvider::SetEvent();
  [pool release];
}

- (void) deviceDidUnMountNotification:(NSNotification *) note 
{
  // calling into c++ code, need to use autorelease pools
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  CDarwinStorageProvider::SetEvent();
  [pool release];
}

- (void) windowDidMoveNotification:(NSNotification*) note
{
  Cocoa_CVDisplayLinkUpdate();
}

- (void) windowDidReSizeNotification:(NSNotification*) note
{
  /* 
  // SDL_PushEvent is not working but
  // this is how one would pass update events in response to grow events.
  NSOpenGLContext* context = [NSOpenGLContext currentContext];
  if (context)
  {
    NSView *view = [context view];
    if (view)
    {
      XBMC_Event event;
      memset(&event, 0, sizeof(event));
      event.resize.type = XBMC_VIDEORESIZE;
      event.resize.w = [view frame].size.width;
      event.resize.h = [view frame].size.height;
      SDL_PushEvent(&event);
    }
  }
  */
}
@end

#ifdef main
#  undef main
#endif
/* Main entry point to executable - should *not* be SDL_main! */
int main(int argc, char *argv[])
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  XBMCDelegate *xbmc_delegate;

  // Block SIGPIPE
  // SIGPIPE repeatably kills us, turn it off
  {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);
  }

  /* Copy the arguments into a global variable */
  /* This is passed if we are launched by double-clicking */
  if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
    gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
    gArgv[0] = argv[0];
    gArgv[1] = NULL;
    gArgc = 1;
    gFinderLaunch = YES;
  } else {
    gArgc = argc;
    gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
    for (int i = 0; i <= argc; i++)
        gArgv[i] = argv[i];
    gFinderLaunch = NO;
  }

  // Ensure the application object is initialised
  [XBMCApplication sharedApplication];

#ifdef SDL_USE_CPS
  {
    CPSProcessSerNum PSN;
    /* Tell the dock about us */
    if (!CPSGetCurrentProcess(&PSN))
      if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
        if (!CPSSetFrontProcess(&PSN))
          [XBMCApplication sharedApplication];
  }
#endif

  // Set up the menubars
  [NSApp setMainMenu:[[NSMenu alloc] init]];
  setupApplicationMenu();
  setupWindowMenu();

  // Create XBMCDelegate and make it the app delegate
  xbmc_delegate = [[XBMCDelegate alloc] init];
  [NSApp setDelegate:xbmc_delegate];

  // Start the main event loop
  [NSApp run];

  // call SDL_main which calls our real main in xbmc.cpp
  // we never return from here as quiting xbmc will call exit() directly.
  // see http://lists.libsdl.org/pipermail/sdl-libsdl.org/2008-September/066542.html
  SDL_main(gArgc, gArgv);

  [xbmc_delegate release];
  [pool release];

  return 0;
}

