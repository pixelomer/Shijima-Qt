QT_VERSION := 6

ifeq ($(QT_VERSION),6)
QMAKE ?= qmake
else
QMAKE ?= qmake-qt5
endif

export QMAKE

CONFIG ?= release
STRIP ?= strip
PKG_CONFIG ?= pkg-config
WINDRES ?= $(patsubst %-windres,%-gcc,$(CC))
AR ?= ar
CMAKE ?= cmake

PLATFORM :=
PLATFORM_CFLAGS :=
PLATFORM_CXXFLAGS :=
PLATFORM_LDFLAGS :=
ifeq ($(OS),Windows_NT)
	PLATFORM := Windows
	BUILD_PLATFORM := Windows
else
	ifeq ($(shell echo $(CC) | grep mingw >/dev/null && echo 1),1)
		PLATFORM := Windows
		BUILD_PLATFORM := Windows
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			PLATFORM := Linux
			BUILD_PLATFORM := Linux
		endif
		ifeq ($(UNAME_S),Darwin)
			PLATFORM := macOS
			BUILD_PLATFORM := macOS
		endif
		ifeq ($(PLATFORM),)
			$(error Unsupported platform)
		endif
	endif
endif

CONFIG_VALID := 0
ifeq ($(CONFIG),release)
	CONFIG_VALID := 1
	CONFIG_CFLAGS := -O3 -flto -DNDEBUG
	CONFIG_CXXFLAGS := -O3 -flto -DNDEBUG
	CONFIG_LDFLAGS := -flto
	CONFIG_CMAKEFLAGS := -DCMAKE_BUILD_TYPE=Release
endif
ifeq ($(CONFIG),debug)
	CONFIG_VALID := 1
	CONFIG_CFLAGS := -g -O0
	CONFIG_CXXFLAGS := -g -O0
	CONFIG_LDFLAGS :=
	CONFIG_CMAKEFLAGS := -DCMAKE_BUILD_TYPE=Debug
endif
ifeq ($(CONFIG_VALID),0)
$(error Invalid CONFIG. Try CONFIG=debug or CONFIG=release)
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
	QT_CFLAGS = $(shell [ -z "$(QT_LIBS)" ] || $(PKG_CONFIG) --cflags $(PREFIXED_QT_LIBS))
	QT_LDFLAGS = $(shell [ -z "$(QT_LIBS)" ] || $(PKG_CONFIG) --libs $(PREFIXED_QT_LIBS))
endif

ifeq ($(PLATFORM),macOS)
	LD_WHOLE_ARCHIVE :=
	LD_NO_WHOLE_ARCHIVE :=
	LD_COPY_NEEDED :=
else
	LD_COPY_NEEDED := -Wl,--copy-dt-needed-entries
	LD_WHOLE_ARCHIVE := -Wl,--whole-archive
	LD_NO_WHOLE_ARCHIVE := -Wl,--no-whole-archive
endif

ifeq ($(PLATFORM),macOS)
define copy_changed
set -e; \
if [[ -d $(2) ]]; then \
	for file in $(1); do \
		out="$(2)/$$(basename $${file})"; \
		if [[ ! -f "$${out}" || "$${file}" -nt "$${out}" ]]; then cp -v "$${file}" "$${out}"; fi; \
	done; \
else \
	if [[ ! -f "$(2)" || "$(1)" -nt "$(2)" ]]; then cp -v "$(1)" "$(2)"; fi; \
fi
endef
else
define copy_changed
cp -uv $(1) $(2)
endef
endif

EXE := 
ifeq ($(PLATFORM),Windows)
	#FIXME: --allow-multiple-definition should not be necessary

	PLATFORM_CFLAGS := -mwindows -msse2
	PLATFORM_CXXFLAGS := -mwindows -msse2
	PLATFORM_LDFLAGS := -Wl,--allow-multiple-definition -mwindows -msse2
	ifeq ($(bindir),)
		bindir := $(shell [ "$${PATH:0:6}" = /mingw ] && echo "$${PATH%%:*}")
		ifeq ($(bindir),)
$(error bindir is not set)
		endif
	endif
	WINDLL_PATH := $(bindir)
	QT_PLUGIN_PATH := $(WINDLL_PATH)/../lib/qt$(QT_VERSION)/plugins
	ifeq ($(shell [ -d "$(QT_PLUGIN_PATH)" ] && echo 1 ]),)
		QT_PLUGIN_PATH := $(WINDLL_PATH)/../share/qt$(QT_VERSION)/plugins
		ifeq ($(shell [ -d "$(QT_PLUGIN_PATH)" ] && echo 1 ]),)
$(error could not find Qt plugin path)
		endif
	endif
	export OBJDUMP
define exe_dlls
$(shell ./find_dlls.sh "$(1)" "$(WINDLL_PATH)")
endef
define copy_exe_dlls
$(call copy_changed,$(call exe_dlls,"$(1)"),$(2))
endef
define copy_qt_plugin_dlls
mkdir -p "$(1)/platforms" "$(1)/styles"
$(call copy_changed,$(QT_PLUGIN_PATH)/platforms/*.dll,$(1)/platforms/)
$(call copy_changed,$(QT_PLUGIN_PATH)/styles/*.dll,$(1)/styles/)
endef
	EXE := .exe
endif

STD_CFLAGS := -Wall
STD_CXXFLAGS := -Wall -std=c++17

PKG_CFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || $(PKG_CONFIG) --cflags $(PKG_LIBS))
PKG_LDFLAGS = $(shell [ -z "$(PKG_LIBS)" ] || $(PKG_CONFIG) --libs $(PKG_LIBS))

ifneq ($(PLATFORM),Windows)
SOURCES_FILTERED = $(filter-out %.rc,$(SOURCES))
else
SOURCES_FILTERED = $(SOURCES)
endif

OBJECTS = $(patsubst %.rc,%.o,$(patsubst %.c,%.o,$(patsubst %.mm,%.o,$(patsubst %.cc,%.o,$(SOURCES_FILTERED)))))
CFLAGS = $(STD_CFLAGS) $(CONFIG_CFLAGS) $(PLATFORM_CFLAGS) $(QT_CFLAGS) $(PKG_CFLAGS)
CXXFLAGS = $(STD_CXXFLAGS) $(CONFIG_CXXFLAGS) $(PLATFORM_CXXFLAGS) $(QT_CFLAGS) $(PKG_CFLAGS)
LDFLAGS = $(CONFIG_LDFLAGS) $(PLATFORM_LDFLAGS) $(QT_LDFLAGS) $(PKG_LDFLAGS)
CMAKEFLAGS = $(CONFIG_CMAKEFLAGS)

%.o: %.c
	$(CC) -MMD -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.cc
	$(CC) -MMD -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.cpp
	$(CC) -MMD -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.m
	$(CC) -MMD -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.mm
	$(CC) -MMD -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.rc
	$(WINDRES) $< -o $@

all::

clean::
	rm -f *.d

FORCE: ;

.PHONY: all clean FORCE

-include *.d
