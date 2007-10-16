AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
INCLUDES = -I$(top_srcdir)/include

LIBS = -lpthread \
       $(top_builddir)/src/lib/libwsman.la 


test_list_SOURCES = test_list.c
test_string_SOURCES = test_string.c
test_md5_SOURCES = test_md5.c

noinst_PROGRAMS =  test_list \
		   test_string \
		   test_md5 
