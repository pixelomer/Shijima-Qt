include ../common.mk

all:
	$(MAKE) -C $(PLATFORM) || { rm -f Platform.a && false; }
	cp $(PLATFORM)/$(PLATFORM).a Platform.a

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f Platform.a