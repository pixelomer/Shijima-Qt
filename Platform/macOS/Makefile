include ../../common.mk

SOURCES = Platform.cc ActiveWindowObserver.cc PrivateActiveWindowObserver.mm
QT_LIBS = Core
CXXFLAGS += -x objective-c++ -fobjc-arc

all: macOS.a

macOS.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) macOS.a