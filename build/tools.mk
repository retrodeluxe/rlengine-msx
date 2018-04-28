# Rules tools
#
TOOLS_ROOT := $(TOP)/tools

export HEX2BIN := $(BUILD_OUT_TOOLS)/hex2bin
export TGA2H := $(BUILD_OUT_TOOLS)/tga2header
export TMU2H := $(BUILD_OUT_TOOLS)/tmu2header

export BUILT_TOOLS := $(HEX2BIN) $(TGA2H) $(TMU2H)

all: $(TGA2H) $(HEX2BIN) $(TMU2H)

$(TGA2H): $(TOOLS_ROOT)/tga2header.c
	$(hide) $(HOSTCC) $^ -o $@

$(HEX2BIN): $(TOOLS_ROOT)/hex2bin.c
	$(hide) $(HOSTCC) $^ -o $@

$(TMU2H): $(TOOLS_ROOT)/tmu2header.c
	$(hide) $(HOSTCC) $^ -o $@
