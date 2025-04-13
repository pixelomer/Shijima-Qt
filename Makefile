include common.mk

SHIJIMA_USE_QTMULTIMEDIA ?= 1

PREFIX ?= /usr/local

SOURCES = main.cc \
	Asset.cc \
	MascotData.cc \
	AssetLoader.cc \
	ForcedProgressDialog.cc \
	ShijimaContextMenu.cc \
	ShijimaManager.cc \
	ShijimaWidget.cc \
	SoundEffectManager.cc \
	ShijimaLicensesDialog.cc \
	ShimejiInspectorDialog.cc \
	DefaultMascot.cc \
	ShijimaHttpApi.cc \
	cli.cc \
	resources.rc \
	MascotBackend.cc \
	MascotBackendWidgets.cc \
	ActiveMascot.cc \
	MascotBackendWindowed.cc \
	WindowedShimeji.cc

DEFAULT_MASCOT_FILES := $(addsuffix .png,$(addprefix DefaultMascot/img/shime,$(shell seq -s ' ' 1 1 46))) \
	DefaultMascot/behaviors.xml DefaultMascot/actions.xml

LICENSE_FILES := Shijima-Qt.LICENSE.txt \
	duktape.LICENSE.txt \
	duktape.AUTHORS.rst \
	libarchive.LICENSE.txt \
	libshijima.LICENSE.txt \
	libshimejifinder.LICENSE.txt \
	unarr.LICENSE.txt \
	unarr.AUTHORS.txt \
	Qt.LICENSE.txt \
	rapidxml.LICENSE.txt

LICENSE_FILES := $(addprefix licenses/,$(LICENSE_FILES))

QT_LIBS = Widgets Core Gui Concurrent

TARGET_LDFLAGS := -Llibshimejifinder/build/unarr -lunarr

ifeq ($(PLATFORM),Linux)
QT_LIBS += DBus
PKG_LIBS := x11 wayland-client wayland-cursor
TARGET_LDFLAGS += -Wl,-R -Wl,$(shell pwd)/publish/Linux/$(CONFIG)
endif

ifeq ($(PLATFORM),Windows)
TARGET_LDFLAGS += -lws2_32
endif

ifeq ($(SHIJIMA_USE_QTMULTIMEDIA),1)
QT_LIBS += Multimedia
CXXFLAGS += -DSHIJIMA_USE_QTMULTIMEDIA=1
else
CXXFLAGS += -DSHIJIMA_USE_QTMULTIMEDIA=0
endif

CXXFLAGS += -Ilibshijima -Ilibshimejifinder -Icpp-httplib
PKG_LIBS += libarchive
PUBLISH_DLL = $(addprefix Qt6,$(QT_LIBS))

define download_linuxdeploy
@uname_m="$$(uname -m)"; \
if [ "$${uname_m}" = "$(1)" -o "$${uname_m}" = "$(2)" ]; then \
	url="https://github.com/linuxdeploy/$(3)/releases/latest/download/$(3)-$(2).AppImage"; \
	name="$${url##*/}"; \
	echo "==> $${url}"; \
	wget -O "$${name}" -c --no-verbose "$${url}"; \
	touch "$${name}"; \
	chmod +x "$${name}"; \
	name2="$${name%-$(2).AppImage}.AppImage"; \
	rm -f "$${name2}"; \
	ln -s "$${name}" "$${name2}"; \
fi
endef

all:: publish/$(PLATFORM)/$(CONFIG)

publish/Windows/$(CONFIG): shijima-qt$(EXE) FORCE
	mkdir -p $@
	@$(call copy_changed,libshimejifinder/build/unarr/libunarr.so.1.1.0,$@)
	@$(call copy_changed,$<,$@)
	@$(call copy_exe_dlls,$<,$@)
	@$(call copy_qt_plugin_dlls,$@)
	if [ $(CONFIG) = release ]; then find $@ -name '*.dll' -exec $(STRIP) -S '{}' \;; fi
	if [ $(CONFIG) = release ]; then $(STRIP)  -S $@/libunarr.so.1.1.0; fi

linuxdeploy-plugin-appimage-x86_64.AppImage:
	$(call download_linuxdeploy,x86_64,x86_64,linuxdeploy-plugin-appimage)

linuxdeploy-plugin-qt-x86_64.AppImage:
	$(call download_linuxdeploy,x86_64,x86_64,linuxdeploy-plugin-qt)

linuxdeploy-x86_64.AppImage: linuxdeploy-plugin-qt-x86_64.AppImage linuxdeploy-plugin-appimage-x86_64.AppImage
	$(call download_linuxdeploy,x86_64,x86_64,linuxdeploy)

linuxdeploy-plugin-appimage-aarch64.AppImage:
	$(call download_linuxdeploy,arm64,aarch64,linuxdeploy-plugin-appimage)

linuxdeploy-plugin-qt-aarch64.AppImage:
	$(call download_linuxdeploy,arm64,aarch64,linuxdeploy-plugin-qt)

linuxdeploy-aarch64.AppImage: linuxdeploy-plugin-qt-aarch64.AppImage linuxdeploy-plugin-appimage-aarch64.AppImage
	$(call download_linuxdeploy,arm64,aarch64,linuxdeploy)

linuxdeploy.AppImage: linuxdeploy-aarch64.AppImage linuxdeploy-x86_64.AppImage

publish/macOS/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	$(call copy_changed,libshimejifinder/build/unarr/libunarr.1.dylib,$@)
	$(call copy_changed,$<,$@)
	if [ $(CONFIG) = release ]; then $(STRIP) -S $@/libunarr.1.dylib; fi
	install_name_tool -add_rpath "$$(realpath $@)" $@/$<

publish/Linux/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	@$(call copy_changed,libshimejifinder/build/unarr/libunarr.so.1,$@)
	if [ $(CONFIG) = release ]; then $(STRIP) -S $@/libunarr.so.1; fi
	@$(call copy_changed,$<,$@)

publish/macOS/$(CONFIG)/Shijima-Qt.app: publish/macOS/$(CONFIG)
	rm -rf $@ && [ ! -d $@ ]
	cp -r Shijima-Qt.app $@
	mkdir -p $@/Contents/MacOS
	cp $^/shijima-qt $@/Contents/MacOS/
	/opt/local/libexec/qt6/bin/macdeployqt $@

publish/Linux/$(CONFIG)/Shijima-Qt.AppImage: publish/Linux/$(CONFIG) linuxdeploy.AppImage
	rm -rf AppDir
	NO_STRIP=1 ./linuxdeploy.AppImage --appdir AppDir --executable publish/Linux/$(CONFIG)/shijima-qt \
		--desktop-file com.pixelomer.ShijimaQt.desktop --output appimage --plugin qt --icon-file \
		com.pixelomer.ShijimaQt.png
	mv Shijima-Qt-*.AppImage Shijima-Qt.AppImage
	cp Shijima-Qt.AppImage publish/Linux/$(CONFIG)/

appimage: publish/Linux/$(CONFIG)/Shijima-Qt.AppImage

macapp: publish/macOS/$(CONFIG)/Shijima-Qt.app

shijima-qt$(EXE): Platform/Platform.a libshimejifinder/build/libshimejifinder.a \
	libshijima/build/libshijima.a shijima-qt.a
	$(CXX) -o $@ $(LD_COPY_NEEDED) $(LD_WHOLE_ARCHIVE) $^ $(LD_NO_WHOLE_ARCHIVE) \
		$(TARGET_LDFLAGS) $(LDFLAGS)
	if [ $(CONFIG) = "release" ]; then $(STRIP) $@; fi

libshijima/build/libshijima.a: libshijima/build/Makefile
	$(MAKE) -C libshijima/build

DefaultMascot.cc: $(DEFAULT_MASCOT_FILES) Makefile bundle-default.sh
	./bundle-default.sh $(DEFAULT_MASCOT_FILES) > '$@-'
	mv '$@-' '$@'

ShijimaLicensesDialog.cc: licenses_generated.hpp
	touch ShijimaLicensesDialog.cc

licenses_generated.hpp: $(LICENSE_FILES) Makefile
	echo 'static const char *shijima_licenses = R"(' > licenses_generated.hpp
	echo 'Licenses for the software components used in Shijima-Qt are listed below.' >> licenses_generated.hpp
	for file in $^; do \
		[ "$$file" != "Makefile" ] || continue; \
		(echo; echo) >> licenses_generated.hpp; \
		echo "~~~~~~~~~~ $$(basename $$file) ~~~~~~~~~~" >> licenses_generated.hpp; \
		echo >> licenses_generated.hpp; \
		cat $$file >> licenses_generated.hpp; \
	done
	echo ')";' >> licenses_generated.hpp

libshijima/build/Makefile: libshijima/CMakeLists.txt FORCE
	mkdir -p libshijima/build && cd libshijima/build && $(CMAKE) $(CMAKEFLAGS) -DSHIJIMA_BUILD_EXAMPLES=NO ..

libshimejifinder/build/Makefile: libshimejifinder/CMakeLists.txt FORCE
	mkdir -p libshimejifinder/build && cd libshimejifinder/build && $(CMAKE) $(CMAKEFLAGS) \
		-DSHIMEJIFINDER_BUILD_LIBARCHIVE=NO -DSHIMEJIFINDER_BUILD_EXAMPLES=NO ..

libshimejifinder/build/libshimejifinder.a: libshimejifinder/build/Makefile
	$(MAKE) -C libshimejifinder/build
	if [ $(PLATFORM) = "Windows" ]; then cp libshimejifinder/build/unarr/libunarr.so.1.1.0 \
		libshimejifinder/build/unarr/libunarr.dll; fi

clean::
	rm -rf publish/$(PLATFORM)/$(CONFIG) libshijima/build libshimejifinder/build
	rm -f $(OBJECTS) shijima-qt.a shijima-qt$(EXE) Shijima-Qt.AppImage
	$(MAKE) -C Platform clean

install:
	install -Dm755 publish/Linux/$(CONFIG)/shijima-qt $(PREFIX)/bin/shijima-qt
	install -Dm755 publish/Linux/$(CONFIG)/libunarr.so.1 $(PREFIX)/lib/libunarr.so.1
	install -Dm644 com.pixelomer.ShijimaQt.desktop $(PREFIX)/share/applications/com.pixelomer.ShijimaQt.desktop
	install -Dm644 com.pixelomer.ShijimaQt.metainfo.xml $(PREFIX)/share/metainfo/com.pixelomer.ShijimaQt.metainfo.xml
	install -Dm644 com.pixelomer.ShijimaQt.png $(PREFIX)/share/icons/hicolor/512x512/apps/com.pixelomer.ShijimaQt.png

uninstall:
	rm -f $(PREFIX)/bin/shijima-qt
	rm -f $(PREFIX)/lib/libunarr.so.1
	rm -f $(PREFIX)/share/applications/com.pixelomer.ShijimaQt.desktop
	rm -f $(PREFIX)/share/metainfo/com.pixelomer.ShijimaQt.metainfo.xml
	rm -f $(PREFIX)/share/icons/hicolor/512x512/apps/com.pixelomer.ShijimaQt.png

Platform/Platform.a: FORCE
	$(MAKE) -C Platform

shijima-qt.a: $(OBJECTS) Makefile
	ar rcs $@ $(filter %.o,$^)

.PHONY: install uninstall
