#include "..\..\..\application.h"
#include "..\python.h"

#pragma code_seg("PY_TEXT")
#pragma data_seg("PY_DATA")
#pragma bss_seg("PY_BSS")
#pragma const_seg("PY_RDATA")

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	extern PyTypeObject WindowType;
	extern PyTypeObject DialogType;
	extern PyTypeObject DialogProgressType;

/*****************************************************************
 * start of xbmc methods
 *****************************************************************/

	PyObject* XBMC_Output(PyObject *self, PyObject *args)
	{
		char *s_line;
		if (!PyArg_ParseTuple(args, "s:xb_output", &s_line))	return NULL;

		OutputDebugString(s_line);
		ThreadMessage tMsg = {TMSG_WRITE_SCRIPT_OUTPUT};
		tMsg.strParam = s_line;
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_MediaPlay(PyObject *self, PyObject *args)
	{
		char *cLine;
		if (!PyArg_ParseTuple(args, "s", &cLine))	return NULL;

		ThreadMessage tMsg = {TMSG_MEDIA_PLAY};
		tMsg.strParam = cLine;
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_MediaStop(PyObject *self, PyObject *args)
	{
		ThreadMessage tMsg = {TMSG_MEDIA_STOP};
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_MediaPause(PyObject *self, PyObject *args)
	{
		ThreadMessage tMsg = {TMSG_MEDIA_PAUSE};
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_Shutdown(PyObject *self, PyObject *args)
	{
		ThreadMessage tMsg = {TMSG_SHUTDOWN};
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_Dashboard(PyObject *self, PyObject *args)
	{
		ThreadMessage tMsg = {TMSG_DASHBOARD};
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* XBMC_Restart(PyObject *self, PyObject *args)
	{
		ThreadMessage tMsg = {TMSG_RESTART};
		g_applicationMessenger.SendMessage(tMsg);

		Py_INCREF(Py_None);
		return Py_None;
	}

	// define c functions to be used in python here
	PyMethodDef xbmcMethods[] = {
		{"output", (PyCFunction)XBMC_Output, METH_VARARGS, "output(line) writes a message to debug terminal"},
		{"mediaplay", (PyCFunction)XBMC_MediaPlay, METH_VARARGS, ""},
		{"mediastop", (PyCFunction)XBMC_MediaStop, METH_VARARGS, ""},
		{"mediapause", (PyCFunction)XBMC_MediaPause, METH_VARARGS, ""},
		{"shutdown", (PyCFunction)XBMC_Shutdown, METH_VARARGS, ""},
		{"dashboard", (PyCFunction)XBMC_Dashboard, METH_VARARGS, ""},
		{"restart", (PyCFunction)XBMC_Restart, METH_VARARGS, ""},
		{NULL, NULL, 0, NULL}
	};

/*****************************************************************
 * end of methods and python objects
 * initxbmc(void);
 *****************************************************************/

	PyMODINIT_FUNC
	initxbmc(void) 
	{
		PyObject* module;

		//WindowType.tp_new = PyType_GenericNew;
		DialogType.tp_new = PyType_GenericNew;
		DialogProgressType.tp_new = PyType_GenericNew;

		if (PyType_Ready(&WindowType) < 0 ||
				PyType_Ready(&DialogType) < 0 ||
				PyType_Ready(&DialogProgressType) < 0)
			return;

		Py_INCREF(&WindowType);
		Py_INCREF(&DialogType);
		Py_INCREF(&DialogProgressType);

		module = Py_InitModule("xbmc", xbmcMethods);
		if (module == NULL) return;

    PyModule_AddObject(module, "Window", (PyObject*)&WindowType);
		PyModule_AddObject(module, "Dialog", (PyObject*)&DialogType);
		PyModule_AddObject(module, "DialogProgress", (PyObject *)&DialogProgressType);
	}
}

#ifdef __cplusplus
}
#endif
