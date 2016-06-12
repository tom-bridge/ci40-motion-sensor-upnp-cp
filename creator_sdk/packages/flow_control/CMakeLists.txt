# Project Properties
####################
CMAKE_MINIMUM_REQUIRED (VERSION 2.6.2)
PROJECT(flow_control)

# Options
###################
SET(CMAKE_VERBOSE_MAKEFILE 1)
SET(CMAKE_BUILD_TYPE DEBUG) # Options MINSIZEREL, RELEASE, DEBUG

INCLUDE_DIRECTORIES(	${STAGING_DIR}/usr/include/flow_dm 
						${STAGING_DIR}/usr/include/
						${STAGING_DIR}/usr/include/gupnp-1.0 
						${STAGING_DIR}/usr/include/gupnp-av-1.0 
						${STAGING_DIR}/usr/include/gssdp-1.0
						${STAGING_DIR}/usr/include/glib-2.0
						${STAGING_DIR}/usr/include/libsoup-2.4
						${STAGING_DIR}/usr/include/libxml2
						${STAGING_DIR}/usr/include/libxml2/libxml
						${STAGING_DIR}/usr/include/glib-2.0/goject
						${STAGING_DIR}/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/include/
					)

# Paths
########
ADD_SUBDIRECTORY(src)

