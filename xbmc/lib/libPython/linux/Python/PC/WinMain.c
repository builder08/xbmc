/* Minimal main program -- everything is loaded from the library. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Python.h"

int WINAPI WinMain(
    HINSTANCE hInstance,      /* handle to current instance */
    HINSTANCE hPrevInstance,  /* handle to previous instance */
    LPSTR lpCmdLine,          /* pointer to command line */
    int nCmdShow              /* show state of window */
)
{
    return Py_Main(__argc, __argv);
}
