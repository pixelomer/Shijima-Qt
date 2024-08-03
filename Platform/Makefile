include ../common.mk

all::
	$(MAKE) -C $(BUILD_PLATFORM) || { rm -f Platform.a && false; }
	$(call copy_changed,$(BUILD_PLATFORM)/$(BUILD_PLATFORM).a,Platform.a)

clean::
	$(MAKE) -C $(BUILD_PLATFORM) clean
	rm -f Platform.a