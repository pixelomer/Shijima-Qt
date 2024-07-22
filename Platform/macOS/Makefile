include ../../common.mk

SOURCES = ActiveWindowObserver.cc PrivateActiveWindowObserver.mm
QT_LIBS = Core
CXXFLAGS += -x objective-c++

all: macOS.a

macOS.a: $(OBJECTS)
	rm -f $@
	ar rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) macOS.a