

SUBDIRS = lib cpp


if DISABLE_PLUGINS
else
SUBDIRS += plugins 
endif


if DISABLE_SERVER
else
SUBDIRS += server
SUBDIRS += authenticators
endif

