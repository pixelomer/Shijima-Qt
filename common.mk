PLATFORM :=
PLATFORM_CFLAGS :=
PLATFORM_CXXFLAGS :=
PLATFORM_LDFLAGS :=
ifeq ($(OS),Windows_NT)
    Platform := Windows
    $(error Windows is not supported)
else
    ifeq ($(shell echo $(CC) | grep mingw >/dev/null && echo 1),1)
        PLATFORM := Windows
        $(error MinGW is not supported)
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Linux)
            PLATFORM := Linux
        endif
        ifeq ($(PLATFORM),)
            $(error Unsupported platform)
        endif
    endif
endif

CONFIG ?= release
ifeq ($(CONFIG),release)
    CONFIG_CFLAGS := -O3 -flto -DNDEBUG
    CONFIG_CXXFLAGS := -O3 -flto -DNDEBUG
    CONFIG_LDFLAGS := -flto
    CONFIG_CMAKEFLAGS := -DCMAKE_BUILD_TYPE=Release
endif
ifeq ($(CONFIG),debug)
    CONFIG_CFLAGS := -g
    CONFIG_CXXFLAGS := -g
    CONFIG_LDFLAGS :=
    CONFIG_CMAKEFLAGS := -DCMAKE_BUILD_TYPE=Debug
endif

PKG_CFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || pkg-config --cflags $(PKG_LIBS))
PKG_LDFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || pkg-config --libs $(PKG_LIBS))

OBJECTS = $(patsubst %.cc,%.o,$(SOURCES))
CFLAGS = $(CONFIG_CFLAGS) $(PLATFORM_CFLAGS) $(PKG_CFLAGS)
CXXFLAGS = $(CONFIG_CXXFLAGS) $(PLATFORM_CXXFLAGS) $(PKG_CFLAGS)
LDFLAGS = $(CONFIG_LDFLAGS) $(PLATFORM_LDFLAGS) $(PKG_LDFLAGS)
CMAKEFLAGS = $(CONFIG_CMAKEFLAGS)