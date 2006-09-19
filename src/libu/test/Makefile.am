AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
INCLUDES = \
	   -I$(top_srcdir)/src/libu

LIBS = -lpthread \
       $(top_builddir)/src/libu/libwsman_utils.la 


test_list_SOURCES = test_list.c
test_string_SOURCES = test_string.c

noinst_PROGRAMS =  test_list \
		   test_string
