include common.mk

SHIJIMA_USE_QTMULTIMEDIA ?= 1

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
	resources.rc

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
PKG_LIBS := x11
TARGET_LDFLAGS += -Wl,-R -Wl,$(shell pwd)/publish/Linux/$(CONFIG)
endif

ifeq ($(SHIJIMA_USE_QTMULTIMEDIA),1)
QT_LIBS += Multimedia
CXXFLAGS += -DSHIJIMA_USE_QTMULTIMEDIA=1
else
CXXFLAGS += -DSHIJIMA_USE_QTMULTIMEDIA=0
endif

CXXFLAGS += -Ilibshijima -Ilibshimejifinder
PKG_LIBS += libarchive
PUBLISH_DLL = $(addprefix Qt6,$(QT_LIBS))

all:: publish/$(PLATFORM)/$(CONFIG)

publish/Windows/$(CONFIG): shijima-qt$(EXE) FORCE
	mkdir -p $@
	@$(call copy_changed,libshimejifinder/build/unarr/libunarr.so.1.1.0,$@)
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
	$(call copy_changed,libshimejifinder/build/unarr/libunarr.1.dylib,$@)
	$(call copy_changed,$<,$@)
	install_name_tool -add_rpath "$$(realpath $@)" $@/$<

publish/Linux/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	@$(call copy_changed,libshimejifinder/build/unarr/libunarr.so.1,$@)
	@$(call copy_changed,$<,$@)

publish/macOS/$(CONFIG)/Shijima-Qt.app: publish/macOS/$(CONFIG)
	rm -rf $@ && [ ! -d $@ ]
	cp -r Shijima-Qt.app $@
	mkdir -p $@/Contents/MacOS
	cp $^/shijima-qt $@/Contents/MacOS/
	/opt/local/libexec/qt6/bin/macdeployqt $@

publish/Linux/$(CONFIG)/Shijima-Qt-x86_64.AppImage: publish/Linux/$(CONFIG) linuxdeploy-x86_64.AppImage
	rm -rf AppDir
	NO_STRIP=1 ./linuxdeploy-x86_64.AppImage --appdir AppDir --executable publish/Linux/$(CONFIG)/shijima-qt \
		--desktop-file shijima-qt.desktop --output appimage --plugin qt --icon-file shijima-qt.png
	cp Shijima-Qt-x86_64.AppImage publish/Linux/$(CONFIG)/

appimage: publish/Linux/$(CONFIG)/Shijima-Qt-x86_64.AppImage

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
	rm -f $(OBJECTS) shijima-qt.a shijima-qt$(EXE) Shijima-Qt-x86_64.AppImage
	$(MAKE) -C Platform clean

Platform/Platform.a: FORCE
	$(MAKE) -C Platform

shijima-qt.a: $(OBJECTS) Makefile
	ar rcs $@ $(filter %.o,$^)
