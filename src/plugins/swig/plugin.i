%{
 /*
  * swig based openwsman server plugin
  *
  * Written by Klaus Kaempf <kkaempf@suse.de>
  *
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

%}

%module openwsman

#if defined(SWIGJAVA)
%module jwsman
#endif

#if defined(SWIGPYTHON)
%module pywsman
#endif

#if defined(SWIGCSHARP)
%module cswsman
#endif

%feature("autodoc","1");

%include "typemaps.i"
%include exception.i

#define __type

%{

/*
 * type definitions to keep the C code generic
 */
 
#if defined(SWIGPYTHON)
#define Target_Null_p(x) (x == Py_None)
#define Target_INCREF(x) Py_INCREF(x)
#define Target_DECREF(x) Py_DECREF(x)
#define Target_True Py_True
#define Target_False Py_False
#define Target_Null NULL
#define Target_Void Py_None
typedef PyObject * Target_Type;
#define Target_Bool(x) PyBool_FromLong(x)
#define Target_WChar(x) PyInt_FromLong(x)
#define Target_Int(x) PyInt_FromLong(x)
#define Target_String(x) PyString_FromString(x)
#define Target_Real(x) Py_None
#define Target_Array() PyList_New(0)
#define Target_SizedArray(len) PyList_New(len)
#define Target_Append(x,y) PyList_Append(x,y)
#define Target_DateTime(x) Py_None
#include <Python.h>
#define TARGET_THREAD_BEGIN_BLOCK SWIG_PYTHON_THREAD_BEGIN_BLOCK
#define TARGET_THREAD_END_BLOCK SWIG_PYTHON_THREAD_END_BLOCK
#define TARGET_THREAD_BEGIN_ALLOW SWIG_PYTHON_THREAD_BEGIN_ALLOW
#define TARGET_THREAD_END_ALLOW SWIG_PYTHON_THREAD_END_ALLOW
#endif

#if defined(SWIGRUBY)
#define Target_Null_p(x) NIL_P(x)
#define Target_INCREF(x) 
#define Target_DECREF(x) 
#define Target_True Qtrue
#define Target_False Qfalse
#define Target_Null Qnil
#define Target_Void Qnil
typedef VALUE Target_Type;
#define Target_Bool(x) ((x)?Qtrue:Qfalse)
#define Target_WChar(x) INT2FIX(x)
#define Target_Int(x) INT2FIX(x)
#define Target_String(x) rb_str_new2(x)
#define Target_Real(x) rb_float_new(x)
#define Target_Array() rb_ary_new()
#define Target_SizedArray(len) rb_ary_new2(len)
#define Target_Append(x,y) rb_ary_push(x,y)
#define Target_DateTime(x) Qnil
#define TARGET_THREAD_BEGIN_BLOCK do {} while(0)
#define TARGET_THREAD_END_BLOCK do {} while(0)
#define TARGET_THREAD_BEGIN_ALLOW do {} while(0)
#define TARGET_THREAD_END_ALLOW do {} while(0)
#include <ruby.h>
#include <rubyio.h>
#endif

#if defined(SWIGPERL)
#define TARGET_THREAD_BEGIN_BLOCK do {} while(0)
#define TARGET_THREAD_END_BLOCK do {} while(0)
#define TARGET_THREAD_BEGIN_ALLOW do {} while(0)
#define TARGET_THREAD_END_ALLOW do {} while(0)

SWIGINTERNINLINE SV *SWIG_From_long  SWIG_PERL_DECL_ARGS_1(long value);
SWIGINTERNINLINE SV *SWIG_FromCharPtr(const char *cptr);
SWIGINTERNINLINE SV *SWIG_From_double  SWIG_PERL_DECL_ARGS_1(double value);

#define Target_Null_p(x) (x == NULL)
#define Target_INCREF(x) 
#define Target_DECREF(x) 
#define Target_True (&PL_sv_yes)
#define Target_False (&PL_sv_no)
#define Target_Null NULL
#define Target_Void NULL
typedef SV * Target_Type;
#define Target_Bool(x) (x)?Target_True:Target_False
#define Target_WChar(x) NULL
#define Target_Int(x) SWIG_From_long(x)
#define Target_String(x) SWIG_FromCharPtr(x)
#define Target_Real(x) SWIG_From_double(x)
#define Target_Array() (SV *)newAV()
#define Target_SizedArray(len) (SV *)newAV()
#define Target_Append(x,y) av_push(((AV *)(x)), y)
#define Target_DateTime(x) NULL
#include <perl.h>
#include <EXTERN.h>
#endif


#include <stdint.h>

#include <u/libu.h>
#include <wsman-xml.h>
#include <wsman-xml-binding.h>
#include <wsman-soap.h>
#include <wsman-soap-envelope.h>
#include <wsman-faults.h>
#include <wsman-client-api.h>
#include <wsman-client-transport.h>

#if defined(SWIGRUBY)
#include <ruby/helpers.c>
#endif
#if defined(SWIGPYTHON)
#include <python/helpers.c>
#endif
#if defined(SWIGJAVA)
#include <java/helpers.c>
#endif

/* fool swig into aliasing WsManClient and WsManTransport */
struct _WsManTransport { };
typedef struct _WsManTransport WsManTransport;

#include "../src/swig-plugin.c"

%}

/* get type declarations from openwsman client bindings */
%include "wsman-xml.i"
%include "wsman-soap.i"
%include "wsman-names.i"
%include "wsman-filter.i"
%include "wsman-client.i"
%include "wsman-transport.i"
