include common.mk

SOURCES = main.cc Asset.cc MascotFinder.cc AssetLoader.cc ShijimaContextMenu.cc ShijimaManager.cc ShijimaWidget.cc
QT_LIBS = Widgets Core Gui

ifeq ($(PLATFORM),Linux)
QT_LIBS += DBus
endif

CXXFLAGS += -Ilibshijima
CMAKEFLAGS += -DSHIJIMA_BUILD_EXAMPLES=NO
PUBLISH_DLL = $(addprefix Qt6,$(QT_LIBS)) 

all:: publish/$(PLATFORM)/$(CONFIG)

publish/Windows/$(CONFIG): shijima-qt$(EXE) FORCE
	mkdir -p $@
	@$(call copy_changed,$<,$@)
	@$(call copy_exe_dlls,$<,$@)
	@$(call copy_qt_plugin_dlls,$@)

publish/macOS/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	$(call copy_changed,$<,$@)

publish/Linux/$(CONFIG): shijima-qt$(EXE)
	mkdir -p $@
	$(call copy_changed,$<,$@)

shijima-qt$(EXE): Platform/Platform.a libshijima/build/libshijima.a shijima-qt.a
	$(CXX) $(LDFLAGS) -o $@ $(LD_WHOLE_ARCHIVE) $^ $(LD_NO_WHOLE_ARCHIVE)
	if [ $(CONFIG) = "release" ]; then $(STRIP) $@; fi

libshijima/build/libshijima.a: libshijima/build/Makefile
	$(MAKE) -C libshijima/build

libshijima/build/Makefile: libshijima/CMakeLists.txt FORCE
	mkdir -p libshijima/build && cd libshijima/build && $(CMAKE) $(CMAKEFLAGS) ..

clean::
	rm -rf publish/$(PLATFORM)/$(CONFIG) libshijima/build
	rm -f $(OBJECTS) shijima-qt.a shijima-qt$(EXE)
	$(MAKE) -C Platform clean

Platform/Platform.a: FORCE
	$(MAKE) -C Platform

shijima-qt.a: $(OBJECTS)
	ar rcs $@ $^

FORCE: ;