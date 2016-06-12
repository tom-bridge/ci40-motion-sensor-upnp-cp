include $(TOPDIR)/rules.mk
PKG_NAME:=flow_control
PKG_VERSION:=0.2.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

CMAKE_INSTALL:=1

COPY_COMMAND = $(CP) -R $(TOPDIR)/../../packages/$(PKG_NAME)/* $(PKG_BUILD_DIR)

CMAKE_OPTIONS= \
	-DSTAGING_DIR=$(STAGING_DIR)/

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Build/Prepare
	$(COPY_COMMAND)
endef

define Package/flow_control
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:= +lwm2md-external +libflowmessaging-external +libflowcore-external +libconfig +libxml2 +libxml +libgupnp +libgupnp-av +gssdp +glib +soup +go +pthread
  TITLE:=Flow button gateway app
endef

define Package/flow_control/description
  Imagination Technologies flow_control
endef

define Build/Compile
	$(call Build/Compile/Default,all)
endef

define Package/flow_control/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
