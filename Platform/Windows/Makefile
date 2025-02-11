include ../../common.mk

SOURCES = Platform.cc ActiveWindowObserver.cc PrivateActiveWindowObserver.cc
QT_LIBS = Core Widgets

all:: Windows.a

Windows.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

clean::
	rm -f $(OBJECTS) Windows.a