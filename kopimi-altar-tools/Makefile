#
# Copyright (C) 2014 Nicolas thill <nico@openwrt.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=kopimi-altar-tools
PKG_VERSION:=0.1

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/BlackBoxe/kopimi-altar-tools.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=3316e56fe07ead85a4f2bd09b6f1bf2e93dffbf5
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/kopimi-altar-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Kopimi Altar tools
  URL:=
endef

define Build/Compile
endef

define Package/kopimi-altar-tools/install
	$(INSTALL_DIR) $(1)/opt/kopimi
	$(CP) $(PKG_BUILD_DIR)/kopimi.conf $(1)/opt/kopimi/
	$(CP) $(PKG_BUILD_DIR)/kopimi*.sh $(1)/opt/kopimi/
	$(INSTALL_DIR) $(1)/opt/kopimi/lib
	$(CP) $(PKG_BUILD_DIR)/lib/*.sh $(1)/opt/kopimi/lib/
	$(INSTALL_DIR) $(1)/opt/kopimi/data
	$(INSTALL_DIR) $(1)/opt/kopimi/data/incoming
	$(INSTALL_DIR) $(1)/opt/kopimi/data/outgoing
	$(INSTALL_DIR) $(1)/opt/kopimi/data/outgoing/always
	$(INSTALL_DIR) $(1)/opt/kopimi/data/outgoing/random
	$(INSTALL_DIR) $(1)/opt/kopimi/log
	$(INSTALL_DIR) $(1)/opt/kopimi/run
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(CP) $(PKG_BUILD_DIR)/lib/openwrt/kopimi.hotplug $(1)/etc/hotplug.d/block/99-kopimi
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/lib/openwrt/kopimi.init $(1)/etc/init.d/kopimi
endef

$(eval $(call BuildPackage,kopimi-altar-tools))
