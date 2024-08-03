include ../../common.mk

SOURCES = Platform.cc ActiveWindowObserver.cc
QT_LIBS = Core

all:: Stub.a

Stub.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

clean::
	rm -f $(OBJECTS) Stub.a