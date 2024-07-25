include ../../common.mk

SOURCES = Platform.cc ActiveWindowObserver.cc
QT_LIBS = Core

all: Windows.a

Windows.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) Windows.a