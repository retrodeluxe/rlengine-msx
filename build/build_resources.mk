#
# Build resources
#
include $(BUILD_SYSTEM)/tools.mk

SPR_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SPR)/*.tga)
MAP_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_MAP)/*.json)
TIL_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_TIL)/*.tga)
built_spr_res := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(SPR_RES_FILES))
built_map_res := $(patsubst $(LOCAL_BUILD_RES_MAP)/%.json,$(LOCAL_BUILD_OUT_GEN)/%.h,$(MAP_RES_FILES))
built_til_res := $(patsubst $(LOCAL_BUILD_RES_TIL)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(TIL_RES_FILES))
built_spr_ext_res := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(SPR_RES_FILES))
built_til_ext_res := $(patsubst $(LOCAL_BUILD_RES_TIL)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(TIL_RES_FILES))

all: $(built_spr_res) $(built_map_res) $(built_til_res) $(built_spr_ext_res) $(built_til_ext_res) | $(TGA2H) $(TILED2H)

$(built_map_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_MAP)/%.json
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TILED2H) -s $^ -b > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_maps.h

$(built_til_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_TIL)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t TILE -z -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles.h

$(built_spr_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_SPR)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t SPRITE -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites.h

$(built_til_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_TIL)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles_ext.h

$(built_spr_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_SPR)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t SPRITEH -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites_ext.h
