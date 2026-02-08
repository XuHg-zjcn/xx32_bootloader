sinclude $(TOOL_DIR)/model.mk

CSRC += $(shell find $(COM_DIR)/Drivers/STM32F1xx_HAL_Driver/Src -name '*.c' | awk -F '_' '{if($$(NF)!="template.c")print $$0}')

SDIR += $(TOP_DIR)/main/Src/
ASRC += $(COM_DIR)/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_$(shell echo $(device_family)$(product_type)$(device_subfamily)x$(flash_size_code2) | tr A-Z a-z).s
