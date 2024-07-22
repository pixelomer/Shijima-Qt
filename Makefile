include common.mk

SOURCES = main.cc ShijimaContextMenu.cc ShijimaManager.cc ShijimaWidget.cc
PKG_LIBS = Qt6Widgets

ifeq ($(PLATFORM),Linux)
PKG_LIBS += Qt6DBus
endif

CXXFLAGS += -Ilibshijima
CMAKEFLAGS += -DSHIJIMA_BUILD_EXAMPLES=NO

all: shijima-qt

shijima-qt: Platform/Platform.a libshijima/build/libshijima.a shijima-qt.a
	$(CXX) $(LDFLAGS) -o $@ -Wl,--whole-archive $^ -Wl,--no-whole-archive
	[ $(CONFIG) = "release" ] && strip $@

libshijima/build/libshijima.a: libshijima/build/Makefile
	$(MAKE) -C libshijima/build

libshijima/build/Makefile: FORCE
	mkdir -p libshijima/build && cd libshijima/build && cmake $(CMAKEFLAGS) ..

clean:
	rm -rf libshijima/build
	rm -f $(OBJECTS) shijima-qt.a shijima-qt
	$(MAKE) -C Platform clean

Platform/Platform.a: FORCE
	$(MAKE) -C Platform

shijima-qt.a: $(OBJECTS)
	ar rcs $@ $^

FORCE: ;