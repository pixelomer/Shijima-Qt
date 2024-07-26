include ../common.mk

all:
	$(MAKE) -C $(PLATFORM) || { rm -f Platform.a && false; }
	$(call copy_changed,$(PLATFORM)/$(PLATFORM).a,Platform.a)

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f Platform.a