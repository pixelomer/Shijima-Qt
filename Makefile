include common.mk

SOURCES = main.cc Asset.cc MascotData.cc AssetLoader.cc ShijimaContextMenu.cc ShijimaManager.cc ShijimaWidget.cc SoundEffectManager.cc
QT_LIBS = Widgets Core Gui Multimedia

ifeq ($(PLATFORM),Linux)
QT_LIBS += DBus
endif

CXXFLAGS += -Ilibshijima -Ilibshimejifinder
PKG_LIBS = libarchive
PUBLISH_DLL = $(addprefix Qt6,$(QT_LIBS)) 

all:: publish/$(PLATFORM)/$(CONFIG)

publish/Windows/$(CONFIG): shijima-qt$(EXE) FORCE
	mkdir -p $@
	@$(call copy_changed,$<,$@)
	@$(call copy_exe_dlls,$<,$@)
	@$(call copy_qt_plugin_dlls,$@)

linuxdeploy-plugin-appimage-x86_64.AppImage:
	wget -O $@ -c --no-verbose https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/1-alpha-20230713-1/linuxdeploy-plugin-appimage-x86_64.AppImage
	touch $@
	chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage

linuxdeploy-plugin-qt-x86_64.AppImage:
	wget -O $@ -c --no-verbose https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/2.0.0-alpha-1-20250119/linuxdeploy-plugin-qt-x86_64.AppImage
	touch $@
	chmod +x linuxdeploy-plugin-qt-x86_64.AppImage

linuxdeploy-x86_64.AppImage: linuxdeploy-plugin-qt-x86_64.AppImage linuxdeploy-plugin-appimage-x86_64.AppImage
	wget -O $@ -c --no-verbose https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	touch $@
	chmod +x linuxdeploy-x86_64.AppImage

publish/macOS/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	$(call copy_changed,$<,$@)

publish/Linux/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	$(call copy_changed,$<,$@)

publish/Linux/$(CONFIG)/Shijima-Qt-x86_64.AppImage: publish/Linux/$(CONFIG) linuxdeploy-x86_64.AppImage
	NO_STRIP=1 ./linuxdeploy-x86_64.AppImage --appdir AppDir --executable publish/Linux/$(CONFIG)/shijima-qt \
		--desktop-file shijima-qt.desktop --output appimage --plugin qt --icon-file shijima-qt.png
	mv Shijima-Qt-x86_64.AppImage publish/Linux/$(CONFIG)/

appimage: publish/Linux/$(CONFIG)/Shijima-Qt-x86_64.AppImage

shijima-qt$(EXE): Platform/Platform.a libshimejifinder/build/libshimejifinder.a \
	libshijima/build/libshijima.a shijima-qt.a
	$(CXX) -o $@ -Llibshimejifinder/build/unarr $(LD_COPY_NEEDED) \
		$(LD_WHOLE_ARCHIVE) $^ $(LD_NO_WHOLE_ARCHIVE) $(LDFLAGS)
	if [ $(CONFIG) = "release" ]; then $(STRIP) $@; fi

libshijima/build/libshijima.a: libshijima/build/Makefile
	$(MAKE) -C libshijima/build

libshijima/build/Makefile: libshijima/CMakeLists.txt FORCE
	mkdir -p libshijima/build && cd libshijima/build && $(CMAKE) $(CMAKEFLAGS) -DSHIJIMA_BUILD_EXAMPLES=NO ..

libshimejifinder/build/Makefile: libshimejifinder/CMakeLists.txt FORCE
	mkdir -p libshimejifinder/build && cd libshimejifinder/build && $(CMAKE) $(CMAKEFLAGS) \
		-DSHIMEJIFINDER_USE_LIBUNARR=NO -DSHIMEJIFINDER_BUILD_LIBARCHIVE=NO \
		-DSHIMEJIFINDER_BUILD_EXAMPLES=NO ..

libshimejifinder/build/libshimejifinder.a: libshimejifinder/build/Makefile
	$(MAKE) -C libshimejifinder/build

clean::
	rm -rf publish/$(PLATFORM)/$(CONFIG) libshijima/build libshimejifinder/build
	rm -f $(OBJECTS) shijima-qt.a shijima-qt$(EXE)
	$(MAKE) -C Platform clean

Platform/Platform.a: FORCE
	$(MAKE) -C Platform

shijima-qt.a: $(OBJECTS)
	ar rcs $@ $^
