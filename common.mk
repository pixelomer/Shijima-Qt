QT_VERSION := 6

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
		ifeq ($(UNAME_S),Darwin)
			PLATFORM := macOS
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

ifeq ($(PLATFORM),macOS)
	PLATFORM_LDFLAGS := -lobjc -framework AppKit -framework ApplicationServices
endif

ifeq ($(PLATFORM),macOS)
	QT_MACOS_PATH := /opt/local/libexec/qt$(QT_VERSION)/lib
	QT_FRAMEWORKS = $(addsuffix .framework,$(addprefix -I$(QT_MACOS_PATH)/Qt,$(QT_LIBS)))
	QT_CFLAGS = -F$(QT_MACOS_PATH) $(addsuffix /Versions/Current/Headers,$(QT_FRAMEWORKS))
	QT_LDFLAGS = -F$(QT_MACOS_PATH) $(addprefix -framework Qt,$(QT_LIBS))
else
	PREFIXED_QT_LIBS = $(addprefix Qt$(QT_VERSION),$(QT_LIBS))
	QT_CFLAGS = $(shell [ -z "$(QT_LIBS)" ] || pkg-config --cflags $(PREFIXED_QT_LIBS))
	QT_LDFLAGS = $(shell [ -z "$(QT_LIBS)" ] || pkg-config --libs $(PREFIXED_QT_LIBS))
endif

ifeq ($(PLATFORM),Linux)
	LD_WHOLE_ARCHIVE := -Wl,--whole-archive
	LD_NO_WHOLE_ARCHIVE := -Wl,--no-whole-archive
else
	LD_WHOLE_ARCHIVE :=
	LD_NO_WHOLE_ARCHIVE :=
endif

STD_CFLAGS := -Wall
STD_CXXFLAGS := -Wall -std=c++17

PKG_CFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || pkg-config --cflags $(PKG_LIBS))
PKG_LDFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || pkg-config --libs $(PKG_LIBS))

OBJECTS = $(patsubst %.mm,%.o,$(patsubst %.cc,%.o,$(SOURCES)))
CFLAGS = $(STD_CFLAGS) $(CONFIG_CFLAGS) $(PLATFORM_CFLAGS) $(QT_CFLAGS) $(PKG_CFLAGS)
CXXFLAGS = $(STD_CXXFLAGS) $(CONFIG_CXXFLAGS) $(PLATFORM_CXXFLAGS) $(QT_CFLAGS) $(PKG_CFLAGS)
LDFLAGS = $(CONFIG_LDFLAGS) $(PLATFORM_LDFLAGS) $(QT_LDFLAGS) $(PKG_LDFLAGS)
CMAKEFLAGS = $(CONFIG_CMAKEFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.cc
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.cpp
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.m
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.mm
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@