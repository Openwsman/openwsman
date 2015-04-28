#
# CMakeLists.txt for openwsman/include
#

add_subdirectory(u)
add_subdirectory(cim)

SET( WSMANINCLUDE_HEADERS wsman-types.h wsman-names.h wsman-debug.h
wsman-client.h wsman-client-api.h wsman-xml-api.h wsman-xml.h
wsman-xml-binding.h wsman-client-transport.h wsman-xml-serializer.h
wsman-xml-serialize.h wsman-server-api.h wsman-faults.h
wsman-soap-message.h wsman-api.h wsman-xml-api.h wsman-client.h
wsman-declarations.h wsman-soap.h wsman-epr.h wsman-filter.h
wsman-soap-envelope.h wsman-subscription-repository.h
wsman-event-pool.h wsman-cimindication-processor.h wsman-key-value.h)

install(FILES ${WSMANINCLUDE_HEADERS} DESTINATION ${INCLUDE_DIR}/openwsman)

#
#EXTRA_DIST = wsman-xml.h \
#	     wsman-xml-binding.h \
#	     wsman-dispatcher.h \
#	     wsman-xml-serialize.h  \
#	     wsman-server.h \
#	     wsman-plugins.h
#
#
