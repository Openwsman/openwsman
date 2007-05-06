

SUBDIRS = libu lib 


if DISABLE_PLUGINS
else
if BUILD_LIBCIM 
SUBDIRS += libcim
endif
SUBDIRS += plugins 
endif


if DISABLE_SERVER
else
SUBDIRS += server
SUBDIRS += authenticators
endif

