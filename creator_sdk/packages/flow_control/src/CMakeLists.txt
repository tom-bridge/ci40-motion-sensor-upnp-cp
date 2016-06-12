# Add executable targets
########################
ADD_EXECUTABLE(	flow_control 
				flow_button_gateway.c 
				flow_interface.c
				control_point.c
				timeout.c)

# Add library targets
#####################
find_library(LIB_FLOWCORE libflowcore.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_FLOWMESSAGING libflowmessaging.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_FLOWDEVMGMT libflowdevmgmtlow.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_XML libxml.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_CYASSL libcyassl.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_CONFIG libconfig.so PATHS ${STAGING_DIR}/usr/lib)
find_library(LIB_GUPNP libgupnp-1.0.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_GUPNP_AV libgupnp-av-1.0.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_GSSDP libgssdp-1.0.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_GLIB libglib-2.0.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_SOUP libsoup-2.4.so PATHS ${STAGING_DIR}/usr/lib/)
#find_library(LIB_XML libxml.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_GO libgobject-2.0.so PATHS ${STAGING_DIR}/usr/lib/)
find_library(LIB_PTHREAD libpthread.so PATHS ${STAGING_DIR}/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/lib/)
find_library(LIB_XML_2 libxml2.so PATHS ${STAGING_DIR}/usr/lib/)

TARGET_LINK_LIBRARIES(	flow_control 
						${LIB_GUPNP} 
						${LIB_GUPNP_AV} 
						${LIB_GSSDP} 
						${LIB_GLIB} 
						${LIB_SOUP} 
					#	${LIB_XML} 
						${LIB_GO} 
						${LIB_PTHREAD} 
						${LIB_XML_2}
						${LIB_FLOWCORE} 
						${LIB_FLOWMESSAGING} 
						${LIB_FLOWDEVMGMT} 
						${LIB_XML} 
						${LIB_CYASSL} 
						${LIB_CONFIG}
					)

TARGET_LINK_LIBRARIES(flow_control )

# Add install targets
######################
INSTALL(TARGETS flow_control RUNTIME DESTINATION bin)

