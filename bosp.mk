
ifdef CONFIG_CONTRIB_HESTONFOUR

# Targets provided by this project
.PHONY: hestonfour clean_hestonfour

# Add this to the "contrib_testing" target
testing: hestonfour
clean_testing: clean_hestonfour

MODULE_CONTRIB_USER_HESTONFOUR=contrib/user/HestonFour

hestonfour: external
	@echo
	@echo "==== Building HestonFour ($(BUILD_TYPE)) ===="
	@echo " Using GCC    : $(CC)"
	@echo " Target flags : $(TARGET_FLAGS)"
	@echo " Sysroot      : $(PLATFORM_SYSROOT)"
	@echo " BOSP Options : $(CMAKE_COMMON_OPTIONS)"
	@[ -d $(MODULE_CONTRIB_USER_HESTONFOUR)/build/$(BUILD_TYPE) ] || \
		mkdir -p $(MODULE_CONTRIB_USER_HESTONFOUR)/build/$(BUILD_TYPE) || \
		exit 1
	@cd $(MODULE_CONTRIB_USER_HESTONFOUR)/build/$(BUILD_TYPE) && \
		CC=$(CC) CFLAGS="$(TARGET_FLAGS)" \
		CXX=$(CXX) CXXFLAGS="$(TARGET_FLAGS)" \
		cmake $(CMAKE_COMMON_OPTIONS) ../.. || \
		exit 1
	@cd $(MODULE_CONTRIB_USER_HESTONFOUR)/build/$(BUILD_TYPE) && \
		make -j$(CPUS) install || \
		exit 1

clean_hestonfour:
	@echo
	@echo "==== Clean-up HestonFour Application ===="
	@[ ! -f $(BUILD_DIR)/usr/bin/hestonfour ] || \
		rm -f $(BUILD_DIR)/etc/bbque/recipes/HestonFour*; \
		rm -f $(BUILD_DIR)/usr/bin/hestonfour*
	@rm -rf $(MODULE_CONTRIB_USER_HESTONFOUR)/build
	@echo

else # CONFIG_CONTRIB_HESTONFOUR

hestonfour:
	$(warning contib module HestonFour disabled by BOSP configuration)
	$(error BOSP compilation failed)

endif # CONFIG_CONTRIB_HESTONFOUR

