/*
 * target_python.c
 *
 * Target language specific functions for openwsman swig plugins
 *
 * Here: Python
 */

/*****************************************************************************
* Copyright (C) 2008 Novell Inc. All rights reserved.
* Copyright (C) 2008 SUSE Linux Products GmbH. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*   - Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
* 
*   - Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
*   - Neither the name of Novell Inc. nor of SUSE Linux Products GmbH nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell Inc. OR SUSE Linux Products GmbH OR
* THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <Python.h>

#define PLUGIN_FILE "pywsmanplugin"
#define PLUGIN_MODULE "WsmanPlugin"

static PyThreadState* pluginMainPyThreadState = NULL; 

/*
 * get Python exception trace -> char
 * 
 */

#define TB_ERROR(str) {tbstr = str; goto cleanup;}
static char *
get_exc_trace()
{
    char *tbstr = NULL; 

    PyObject *iostrmod = NULL;
    PyObject *tbmod = NULL;
    PyObject *iostr = NULL;
    PyObject *obstr = NULL;
    PyObject *args = NULL;
    PyObject *newstr = NULL;
    PyObject *func = NULL;
    char* rv = NULL; 

    PyObject *type, *value, *traceback;
    TARGET_THREAD_BEGIN_BLOCK; 
    PyErr_Fetch(&type, &value, &traceback);
    debug("** type %p, value %p, traceback %p", type, value, traceback); 
    PyErr_Print(); 
    PyErr_Clear(); 
    PyErr_NormalizeException(&type, &value, &traceback);
    debug("** type %p, value %p, traceback %p", type, value, traceback); 

    iostrmod = PyImport_ImportModule("StringIO");
    if (iostrmod==NULL)
        TB_ERROR("can't import StringIO");

    iostr = PyObject_CallMethod(iostrmod, "StringIO", NULL);

    if (iostr==NULL)
        TB_ERROR("cStringIO.StringIO() failed");

    tbmod = PyImport_ImportModule("traceback");
    if (tbmod==NULL)
        TB_ERROR("can't import traceback");

    obstr = PyObject_CallMethod(tbmod, "print_exception",
        "(OOOOO)",
        type ? type : Py_None, 
        value ? value : Py_None,
        traceback ? traceback : Py_None,
        Py_None,
        iostr);

    if (obstr==NULL) 
    {
        PyErr_Print(); 
        TB_ERROR("traceback.print_exception() failed");
    }

    Py_DecRef(obstr);

    obstr = PyObject_CallMethod(iostr, "getvalue", NULL);
    if (obstr==NULL) 
        TB_ERROR("getvalue() failed.");

    if (!PyString_Check(obstr))
        TB_ERROR("getvalue() did not return a string");

    debug("%s", PyString_AsString(obstr)); 
    args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, string2target("\n")); 
    PyTuple_SetItem(args, 1, string2target("<br>")); 
    
    func = PyObject_GetAttrString(obstr, "replace"); 
    //newstr = PyObject_CallMethod(obstr, "replace", args); 
    newstr = PyObject_CallObject(func, args); 

    tbstr = PyString_AsString(newstr); 

    rv = fmtstr("plugin:%s", tbstr); 

cleanup:
    PyErr_Restore(type, value, traceback);

    if (rv == NULL)
    {
        rv = tbstr ? tbstr : "";
    }

    Py_DecRef(func);
    Py_DecRef(args);
    Py_DecRef(newstr);
    Py_DecRef(iostr);
    Py_DecRef(obstr);
    Py_DecRef(iostrmod);
    Py_DecRef(tbmod);


    TARGET_THREAD_END_BLOCK; 
    return rv;
}


/*
 * Global Python initializer
 * 
 * load the Python interpreter
 * init threads
 */

static int
PyGlobalInitialize()
{
/*  debug("<%d/0x%x> PyGlobalInitialize() called", getpid(), pthread_self()); */
  
  if (_TARGET_INIT)
    {
/*      debug("<%d/0x%x> PyGlobalInitialize() returning: already initialized", getpid(), pthread_self()); */
      return 0; 
    }
  _TARGET_INIT=1;//true
  
  debug("<%d/0x%x> Python: Loading", getpid(), pthread_self());
  
  Py_SetProgramName(PLUGIN_FILE);
  Py_Initialize();
  SWIGEXPORT void SWIG_init(void);
  SWIG_init();
  pluginMainPyThreadState = PyGILState_GetThisThreadState();
  PyEval_ReleaseThread(pluginMainPyThreadState); 
  
  debug("<%d/0x%x> PyGlobalInitialize() succeeded", getpid(), pthread_self()); 
  return 0; 
}


/*---------------------------------------------------------------*/

/*
 * TargetCall
 * 
 * ** must be called while holding the threads lock **
 */

static int 
TargetCall(WsXmlDocH doc, PyObject* instance, 
                 const char* opname, int nargs, ...)
{
    va_list vargs; 
    PyObject *pyargs = NULL; 
    PyObject *pyfunc = NULL; 
    PyObject *result = NULL; 
    WsmanStatus status;
    wsman_status_init(&status);

    pyargs = PyTuple_New(nargs); 
    pyfunc = PyObject_GetAttrString(instance, opname); 
    if (pyfunc == NULL)
    {
        PyErr_Print(); 
        PyErr_Clear(); 
        char* str = fmtstr("Python module does not contain \"%s\"", opname); 
        debug("%s", str); 
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;

        free(str); 
        goto cleanup; 
    }
    if (! PyCallable_Check(pyfunc))
    {
        char* str = fmtstr("Python module attribute \"%s\" is not callable", 
                opname); 
        debug("%s", str); 
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        free(str); 
        goto cleanup; 
    }
    
    va_start(vargs, nargs); 
    int i; 
    for (i = 0; i < nargs; ++i)
    {
        PyObject* arg = va_arg(vargs, PyObject*); 
        if (arg == NULL)
        {
            arg = Py_None; 
            Py_IncRef(arg); 
        }
        PyTuple_SET_ITEM(pyargs, i, arg); 
    }
    va_end(vargs); 
    result = PyObject_CallObject(pyfunc, pyargs);
    if (PyErr_Occurred())
    {
        status.fault_code = WSMAN_INTERNAL_ERROR;
	status.fault_detail_code = 0;
        PyErr_Clear(); 
        goto cleanup; 
    }

    if (! PyTuple_Check(result) || 
            (PyTuple_Size(result) != 2 && PyTuple_Size(result) != 1))
    {
        TARGET_THREAD_BEGIN_ALLOW;
        status.fault_msg = fmtstr("Python function \"%s\" didn't return a two-tuple", opname); 
	status.fault_code = WSMAN_INTERNAL_ERROR;
	status.fault_detail_code = 0;
        TARGET_THREAD_END_ALLOW; 
        goto cleanup; 
    }
    PyObject* code = PyTuple_GetItem(result, 0);
    PyObject* detail = Py_None; 
    if (PyTuple_Size(result) == 2)
    {
        detail = PyTuple_GetItem(result, 1); 
    }

    if (! PyInt_Check(code) || (! PyInt_Check(detail) && detail != Py_None))
    {
        TARGET_THREAD_BEGIN_ALLOW;
        status.fault_msg = fmtstr("Python function \"%s\" didn't return a {<int>, <int>) two-tuple", opname); 
	status.fault_code = WSMAN_INTERNAL_ERROR;
	status.fault_detail_code = 0;
        TARGET_THREAD_END_ALLOW; 
        goto cleanup; 
    }
    status.fault_code = PyInt_AsLong(code); 
    if (detail == Py_None)
    {
	status.fault_code = WSMAN_INTERNAL_ERROR;
	status.fault_detail_code = 0;
    }
    else
    {
        status.fault_detail_code = PyInt_AsLong(detail);
    }
cleanup:
    if (status.fault_code != WSMAN_RC_OK)
      wsman_generate_fault( doc, status.fault_code, status.fault_detail_code, status.fault_msg );
    Py_DecRef(pyargs);
    Py_DecRef(pyfunc);
    Py_DecRef(result);
 
    return status.fault_code != WSMAN_RC_OK; 
}


/*
 * import PLUGIN_FILE
 * local (per plugin) Python initializer
 * keeps track of reference count
 */

static int
TargetInitialize(void *self, void **data)
{
  int rc = 0; 
  WsmanStatus status;
  wsman_status_init(&status);

  /* Set _PLUGIN_INIT, protected by _PLUGIN_INIT_MUTEX
   * so we call Py_Finalize() only once.
   */
  if (pthread_mutex_lock(&_PLUGIN_INIT_MUTEX))
  {
      perror("Can't lock _PLUGIN_INIT_MUTEX");
      abort();
  }

  /* load Python */
  rc = PyGlobalInitialize(); 
  if (rc != 0)
  {
      pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);
      return rc; 
  }

  debug("<%d/0x%x> TargetInitialize(Python) called", getpid(), pthread_self());
  
  TARGET_THREAD_BEGIN_BLOCK;
  
  /*
   * import PLUGIN_FILE
   */
  
  if (_TARGET_MODULE == NULL)
  {
    _TARGET_MODULE = PyImport_ImportModule(PLUGIN_FILE);
    if (_TARGET_MODULE == NULL)
    {
      char *trace = get_exc_trace();
      debug("<%d/0x%x> Python: import %s failed", getpid(), pthread_self(), PLUGIN_FILE);
      message(trace);
      PyErr_Clear(); 
      TARGET_THREAD_END_BLOCK; 
      debug("<%d/0x%x> %s", getpid(), pthread_self(), trace);
      status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
      status.fault_detail_code = 0;
      pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);
      return -1; 
    }
    *data = _TARGET_MODULE;
  }
  pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);
  debug("<%d/0x%x> Python: _TARGET_MODULE at %p", getpid(), pthread_self(), _TARGET_MODULE);
  
 
  TARGET_THREAD_END_BLOCK; 
  debug("<%d/0x%x> TargetInitialize(Python) succeeded", getpid(), pthread_self()); 
  return 0; 
}


/*
 * TargetCleanup
 */

static void
TargetCleanup(void *self, void *data)
{
    /* Decrement _PLUGIN_COUNT, protected by _PLUGIN_INIT_MUTEX
     * call Py_Finalize when _PLUGIN_COUNT drops to zero
     */
    if (pthread_mutex_lock(&_PLUGIN_INIT_MUTEX))
    {
        perror("Can't lock _PLUGIN_INIT_MUTEX");
        abort();
    }
    if (--_PLUGIN_COUNT > 0) 
    {
        pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);
        return;
    }

    TARGET_THREAD_BEGIN_BLOCK;
    Py_DecRef(_TARGET_MODULE);
    TARGET_THREAD_END_BLOCK;
  
    PyEval_AcquireLock(); 
    PyThreadState_Swap(pluginMainPyThreadState); 
    if (_TARGET_INIT)  // if Python is initialized and _PLUGIN_COUNT == 0, call Py_Finalize
    {
        debug("Calling Py_Finalize()");
        Py_Finalize();
        _TARGET_INIT=0; // false
    }
    pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);

}


/*
 * TargetEndpoints
 */

static list_t *
TargetEndpoints( void *self, void *data )
{
    int len, i;
    PyObject *instance = (PyObject *)data;
    PyObject *pyfunc, *pynamespaces = NULL;
    WsmanStatus status;
    wsman_status_init(&status);
    debug("TargetEndpoints(Python), data %p, instance %p", data, instance);

    /*
     * Get namespaces
     */
  
    list_t *namespaces = list_create(LISTCOUNT_T_MAX);

    pyfunc = PyObject_GetAttrString(instance, "namespaces"); 
    if (pyfunc == NULL)
    {
        PyErr_Print(); 
        PyErr_Clear(); 
        debug("Python module does not contain \"namespaces\""); 
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        goto cleanup; 
    }
    if (! PyCallable_Check(pyfunc))
    {
        debug("Python module attribute \"namespaces\" is not callable"); 
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        goto cleanup; 
    }
    pynamespaces = PyObject_CallObject(pyfunc, NULL);
    if (PyErr_Occurred())
    {
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        PyErr_Clear();
        goto cleanup; 
    }

    if (! PyTuple_Check(pynamespaces))
    {
        TARGET_THREAD_BEGIN_ALLOW;
        debug("Python function \"namespaces\" didn't return a two-tuple"); 
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        TARGET_THREAD_END_ALLOW; 
        goto cleanup; 
    }

    len = PyTuple_Size(pynamespaces);

    for (i = 0; i < len; ++i) {
      lnode_t *node;
      PyObject *ns, *prefix;
      PyObject* elem = PyTuple_GetItem(pynamespaces, i);
      if (! PyTuple_Check(elem)
	  || ! (PyTuple_Size(elem) == 2)) {
        TARGET_THREAD_BEGIN_ALLOW;
        debug("Python function \"namespaces\" didn't return a list of two-tuple");
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        TARGET_THREAD_END_ALLOW; 
        goto cleanup; 
      }
      ns = PyTuple_GetItem(elem, 0);
      prefix = PyTuple_GetItem(elem, 1);
      if (!PyString_Check(ns)
	  || !PyString_Check(prefix)) {
        TARGET_THREAD_BEGIN_ALLOW;
        debug("Python function \"namespaces\" didn't return a list of [<string>,<string>] tuples");
	status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
	status.fault_detail_code = 0;
        TARGET_THREAD_END_ALLOW; 
        goto cleanup; 
      }
      
      WsSupportedNamespaces *sup_ns = (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
      sup_ns->ns = PyString_AsString(ns);
      sup_ns->class_prefix = PyString_AsString(prefix);
      node = lnode_create(ns);
      list_append(namespaces, node);
    }
cleanup:
    if (pyfunc) Py_DecRef(pyfunc);
    if (pynamespaces) Py_DecRef(pynamespaces);
  
    return namespaces;
}
