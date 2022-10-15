/*
 * target_ruby.c
 *
 * Target language specific functions for openwsman swig plugins
 *
 * Here: Ruby
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

/* load <RB_PLUGIN_FILE>.rb */
#define PLUGIN_FILE "openwsmanplugin"

/* expect 'module <RB_PLUGIN_MODULE>' inside */
#define PLUGIN_MODULE "Openwsman"

/*
 * load_module
 * separate function for rb_require so it can be wrapped into rb_protect()
 */

static VALUE
load_module()
{
  ruby_script(PLUGIN_FILE);
  return rb_require(PLUGIN_FILE);
}


/*
 * create_plugin (called from rb_protect)
 * load Ruby provider and create plugin instance
 * 
 */

static VALUE
create_plugin(VALUE args)
{
  return rb_funcall2(_TARGET_MODULE, rb_intern("create_plugin"), 0, NULL);
}


/*
 * call_plugin
 * call function of plugin instance
 * 
 * I args: pointer to array of at least 3 values
 *         args[0] -> (VALUE) instance
 *         args[1] -> (VALUE) id of function
 *         args[2] -> (int) number of arguments
 *         args[3...n] -> (VALUE) arguments
 */

static VALUE
call_plugin(VALUE args)
{
  VALUE *values = (VALUE *)args;
  return rb_funcall3(values[0], values[1], (int)values[2], values+3);
}



/*
 * get Ruby exception trace -> char *
 * 
 */

#define TB_ERROR(str) {tbstr = str; goto cleanup;}
static char *
get_exc_trace()
{
    VALUE exception = rb_gv_get("$!"); /* get last exception */
    VALUE reason = rb_funcall(exception, rb_intern("to_s"), 0 );
    VALUE trace = rb_gv_get("$@"); /* get last exception trace */
    VALUE backtrace = rb_funcall(trace, rb_intern("join"), 1, rb_str_new("\n\t", 2));

    return fmtstr("%s\n\t%s", StringValuePtr(reason), StringValuePtr(backtrace)); 
}


/*
 * Global Ruby initializer
 * loads the Ruby interpreter
 * init threads
 */

static int
RbGlobalInitialize( )
{
  int error;

  if (_TARGET_INIT)
    {
      return 0; 
    }
  _TARGET_INIT=1;//true
  
  debug("Ruby: Loading");
  
  ruby_init();
  ruby_init_loadpath();
  ruby_script(PLUGIN_FILE);
  extern void SWIG_init();
  SWIG_init();

  /* load module */
  rb_protect(load_module, Qnil, &error);
  if (error)
    {
      char *trace = get_exc_trace();

      error("Ruby: import '%s' failed: %s", PLUGIN_FILE, trace);
      return -1;
    }
  _TARGET_MODULE = rb_const_get(rb_cModule, rb_intern(PLUGIN_MODULE));
  if (NIL_P(_TARGET_MODULE))
    {
      error("Ruby: import '%s' doesn't define module '%s'", PLUGIN_MODULE);
      return -1;
    }  
  debug("RbGlobalInitialize() succeeded -> module %s @ %p", PLUGIN_MODULE, _TARGET_MODULE); 
  return 0; 
}


/*---------------------------------------------------------------*/

/*
 * local (per plugin) Ruby initializer
 * keeps track of reference count
 * 
 * self: handle (from dlopen)
 * data: user data return
 */

static int
TargetInitialize( void *self, void **data )
{
  VALUE args[2];
  int error;

  debug("TargetInitialize(Ruby)");
  /* Set _PLUGIN_INIT, protected by _PLUGIN_INIT_MUTEX
   * so we call ruby_finalize() only once.
   */
  if (pthread_mutex_lock(&_PLUGIN_INIT_MUTEX))
  {
      perror("Can't lock _PLUGIN_INIT_MUTEX");
      abort();
  }
  error = RbGlobalInitialize( ); 
  pthread_mutex_unlock(&_PLUGIN_INIT_MUTEX);
  if (error != 0)
  {
     goto exit;
  }

  debug("TargetInitialize(Ruby) called");

  *data = (void *)rb_protect(create_plugin, (VALUE)args, &error);
  if (error)
    {
      char *trace = get_exc_trace();
      error("Ruby: FAILED creating:", trace);
    }
  debug("Created plugin: klass @ %p", *data);
exit:
  debug("Initialize() %s", (error == 0)? "succeeded":"failed");
  return error;
}


/*
 * TargetCall
 * Call function 'opname' with nargs arguments within instance
 * 
 * doc: in_doc from context, needed for fault generation
 * instance: module
 * opname: name of method to call
 * nargs: number of arguments
 * ...: arguments as varargs
 * 
 */

static int 
TargetCall(WsXmlDocH doc, VALUE instance, const char* opname, int nargs, ...)
{
  int i; 
  VALUE *args, result, op = rb_intern(opname);
  va_list vargs; 
  WsmanStatus status;
  wsman_status_init(&status);

  debug("TargetCall(Ruby): %p.%s", (void *)instance, opname);
  
  /* add instance, op and nargs to the args array, so rb_protect can be called */
  nargs += 3;
  args = (VALUE *)malloc(nargs * sizeof(VALUE));
  if (args == NULL)
    {
      error("Out of memory"); 
      abort();
    }
  args[0] = instance;
  args[1] = op;
  args[2] = (VALUE)(nargs-3);
  if (nargs > 3)
    {
      va_start(vargs, nargs);
      for (i = 3; i < nargs; ++i)
	{
	  args[i] = va_arg(vargs, VALUE);
	}
      va_end(vargs);
    }

  
  /* call the Ruby function
   * possible results:
   *   i nonzero: Exception raised
   *   result == nil: not (or badly) implemented 
   *   result == true: success
   *   result == Array: pair of CMPIStatus rc(int) and msg(string)
   */
  result = rb_protect(call_plugin, (VALUE)args, &i);
  free( args );

  if (i) /* exception ? */
    {
      char *trace = get_exc_trace();
      status.fault_msg = fmtstr("Ruby: calling '%s' failed: %s", opname, trace); 
      status.fault_code = WSMAN_INTERNAL_ERROR;
      status.fault_detail_code = 0;
      error("%s", status.fault_msg);
      return 1;
    }
  
  if (NIL_P(result)) /* not or wrongly implemented */
    {
      status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
      status.fault_detail_code = 0;
      return 1;
    }

  if (result != Qtrue)
    {
      int len;
      VALUE resulta = rb_check_array_type(result);
      
      if (NIL_P(resulta))
	{
	  status.fault_msg = fmtstr("Ruby: calling '%s' returned unknown result", opname);
	  status.fault_code = WSMAN_INTERNAL_ERROR;
	  status.fault_detail_code = 0;
	  return 1;
	}
  
      len = RARRAY_LEN(resulta);
      if (len > 0) 
	{
	  VALUE code = rb_ary_entry(resulta, 0);
	  if (!FIXNUM_P(code))
	    {
	      status.fault_msg = fmtstr("Ruby: calling '%s' returned non-numeric code", opname);
	      status.fault_code = WSMAN_INTERNAL_ERROR;
	      status.fault_detail_code = 0;
	      return 1;
	    }
	  status.fault_code = FIX2LONG(code);
	}
      
      if (len > 1) 
	{
	  VALUE detail = rb_ary_entry(resulta, 1);
	  if (!FIXNUM_P(detail))
	    {
	      status.fault_msg = fmtstr("Ruby: calling '%s' returned non-numeric detail", opname);
	      status.fault_code = WSMAN_INTERNAL_ERROR;
	      status.fault_detail_code = 0;
	      return 1;
	    }
	  status.fault_detail_code = FIX2LONG(detail);
	}
      if (len > 2)
	{
	  VALUE str = rb_ary_entry(resulta, 2);
	  status.fault_msg = StringValuePtr(str);
	}
      wsman_generate_fault( doc, status.fault_code, status.fault_detail_code, status.fault_msg );
    }
  
  /* all is fine */

  return 0;
}


/*
 * TargetCleanup
 */

static void
TargetCleanup( void *self, void *data )
{
  debug("TargetCleanup(Ruby)");
  ruby_finalize();
  _TARGET_MODULE = Qnil;   
  return;
}


static VALUE
call_namespaces(VALUE klass)
{
  return rb_funcall( klass, rb_intern( "namespaces" ), 0 );
}


/*
 * TargetEndpoints
 */

static list_t *
TargetEndpoints( void *self, void *data )
{
    int error;
    VALUE klass = (VALUE)data;
    VALUE rbnamespaces, ary;
    debug("TargetEndpoints(Ruby), data %p, klass %p", data, klass);

    /*
     * Get namespaces
     */
  
    list_t *namespaces = list_create(LISTCOUNT_T_MAX);
    debug("TargetEndpoints(Ruby), calling namespaces");
    rbnamespaces = rb_protect(call_namespaces, klass, &error);
    if (error) {
      char *trace = get_exc_trace();

      error("Ruby: 'namespaces' failed: %s", PLUGIN_FILE, trace);
      return NULL;
    }
    debug("TargetEndpoints(Ruby), called namespaces: %p", rbnamespaces);
    ary = rb_check_array_type( rbnamespaces );
    if (NIL_P(ary)) {
      rb_raise( rb_eArgError, "namespaces is not array");
    }
    int len = RARRAY_LEN(ary);
    if (len <= 0) {
      rb_raise( rb_eArgError, "namespaces returned array with %d elements", len);
    }
    int i;
    for (i = 0; i < len; ++i) {
      lnode_t *node;
      VALUE elem = RARRAY_PTR(ary)[i];
      VALUE pair = rb_check_array_type( elem );
      if (NIL_P(pair)) {
	rb_raise( rb_eArgError, "namespaces must return array of arrays");
      }
      if (RARRAY_LEN(pair) != 2) {
	rb_raise( rb_eArgError, "namespaces must return array of ['<namespace>','<class_prefix>']");
      }
      WsSupportedNamespaces *ns = (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
      ns->ns = StringValuePtr( RARRAY_PTR(pair)[0] );
      ns->class_prefix = StringValuePtr( RARRAY_PTR(pair)[1] );
      node = lnode_create(ns);
      list_append(namespaces, node);
    }

  return namespaces;
}
