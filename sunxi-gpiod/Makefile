#
# Copyright (C) 2014 Nicolas thill <nico@openwrt.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=sunxi-gpiod
PKG_VERSION:=0.1

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/sunxi-gpiod
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GPIO Daemon for A10-Olinuxino-LIME
  URL:=
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Package/sunxi-gpiod/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpiod $(1)/usr/bin/
endef

$(eval $(call BuildPackage,sunxi-gpiod))
