#
# Build resources
#
include $(BUILD_SYSTEM)/tools.mk
include $(BUILD_SYSTEM)/util.mk

SPR_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SPR)/*.tga)
SPR_RES_FILES_PNG := $(wildcard $(LOCAL_BUILD_RES_SPR)/*.png)
MAP_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_MAP)/*.json)
TIL_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_TIL)/*.tga)
TIL_RES_FILES_PNG := $(wildcard $(LOCAL_BUILD_RES_TIL)/*.png)
RAW_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_RAW)/*.tga)
RAW_RES_FILES_PNG := $(wildcard $(LOCAL_BUILD_RES_RAW)/*.png)
PT3_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_PT3)/*.pt3)
SFX_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SFX)/*.afb)
PAT_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SCR)/*.pat)
COL_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SCR)/*.col)
VDA_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_SCR)/*.vda)
FNT_RES_FILES := $(wildcard $(LOCAL_BUILD_RES_FNT)/*.tga)
FNT_RES_FILES_PNG := $(wildcard $(LOCAL_BUILD_RES_FNT)/*.png)
built_spr_res := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(SPR_RES_FILES))
built_spr_res_png := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.png,$(LOCAL_BUILD_OUT_GEN)/%.h,$(SPR_RES_FILES_PNG))
built_map_res := $(patsubst $(LOCAL_BUILD_RES_MAP)/%.json,$(LOCAL_BUILD_OUT_GEN)/%.h,$(MAP_RES_FILES))
built_til_res := $(patsubst $(LOCAL_BUILD_RES_TIL)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(TIL_RES_FILES))
built_til_res_png := $(patsubst $(LOCAL_BUILD_RES_TIL)/%.png,$(LOCAL_BUILD_OUT_GEN)/%.h,$(TIL_RES_FILES_PNG))
built_raw_res := $(patsubst $(LOCAL_BUILD_RES_RAW)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(RAW_RES_FILES))
built_raw_res_png := $(patsubst $(LOCAL_BUILD_RES_RAW)/%.png,$(LOCAL_BUILD_OUT_GEN)/%.h,$(RAW_RES_FILES_PNG))
built_pt3_res := $(patsubst $(LOCAL_BUILD_RES_PT3)/%.pt3,$(LOCAL_BUILD_OUT_GEN)/%.h,$(PT3_RES_FILES))
built_sfx_res := $(patsubst $(LOCAL_BUILD_RES_SFX)/%.afb,$(LOCAL_BUILD_OUT_GEN)/%.h,$(SFX_RES_FILES))
built_pat_res := $(patsubst $(LOCAL_BUILD_RES_SCR)/%.pat,$(LOCAL_BUILD_OUT_GEN)/%.pat.h,$(PAT_RES_FILES))
built_col_res := $(patsubst $(LOCAL_BUILD_RES_SCR)/%.col,$(LOCAL_BUILD_OUT_GEN)/%.col.h,$(COL_RES_FILES))
built_vda_res := $(patsubst $(LOCAL_BUILD_RES_SCR)/%.vda,$(LOCAL_BUILD_OUT_GEN)/%.vda.h,$(VDA_RES_FILES))
built_fnt_res := $(patsubst $(LOCAL_BUILD_RES_FNT)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%.h,$(FNT_RES_FILES))
built_fnt_res_png := $(patsubst $(LOCAL_BUILD_RES_FNT)/%.png,$(LOCAL_BUILD_OUT_GEN)/%.h,$(FNT_RES_FILES_PNG))

built_spr_ext_res := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(SPR_RES_FILES))
built_til_ext_res := $(patsubst $(LOCAL_BUILD_RES_TIL)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(TIL_RES_FILES))
built_raw_ext_res := $(patsubst $(LOCAL_BUILD_RES_RAW)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(RAW_RES_FILES))
built_fnt_ext_res := $(patsubst $(LOCAL_BUILD_RES_FNT)/%.tga,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(FNT_RES_FILES))
built_spr_ext_res_png := $(patsubst $(LOCAL_BUILD_RES_SPR)/%.png,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(SPR_RES_FILES_PNG))
built_til_ext_res_png:= $(patsubst $(LOCAL_BUILD_RES_TIL)/%.png,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(TIL_RES_FILES_PNG))
built_raw_ext_res_png := $(patsubst $(LOCAL_BUILD_RES_RAW)/%.png,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(RAW_RES_FILES_PNG))
built_fnt_ext_res_png := $(patsubst $(LOCAL_BUILD_RES_FNT)/%.png,$(LOCAL_BUILD_OUT_GEN)/%_ext.h,$(FNT_RES_FILES_PNG))

all: $(built_spr_res) $(built_map_res) $(built_til_res) $(built_spr_ext_res) $(built_til_ext_res) \
	$(built_pt3_res) $(built_sfx_res) $(built_pat_res) $(built_col_res) $(built_fnt_res) \
	$(built_vda_res) $(built_fnt_ext_res) $(built_raw_res) $(built_raw_ext_res) \
	$(built_spr_ext_res_png) $(built_til_ext_res_png) $(built_raw_ext_res_png) \
	$(built_fnt_ext_res_png) $(built_spr_res_png) $(built_til_res_png) \
	$(built_raw_res_png) $(built_fnt_res_png) | $(TGA2H) $(TILED2H)

$(built_map_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_MAP)/%.json
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
		$(call print_res, map, $^)
	$(hide) $(TILED2H) -s $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h

$(built_til_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_TIL)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
		$(call print_res, tile, $^)
	$(hide) $(TGA2H) -t TILE -z -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles.h

$(built_til_res_png) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_TIL)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
		$(call print_res, tile, $^)
	$(hide) $(PNG2H) -t TILE -z -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles.h

$(built_raw_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_RAW)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
		$(call print_res, raw, $^)
	$(hide) $(TGA2H) -t TILE -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_raw_tiles.h

$(built_raw_res_png) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_RAW)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, raw, $^)
	$(hide) $(PNG2H) -t TILE -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_raw_tiles.h

$(built_spr_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_SPR)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, sprite, $^)
	$(hide) $(TGA2H) -t SPRITE -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites.h

$(built_spr_res_png) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_SPR)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, sprite, $^)
	$(hide) $(PNG2H) -t SPRITE -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME).h
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites.h

$(built_pt3_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_PT3)/%.pt3
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, pt3, $^)
	$(hide) cd $(LOCAL_BUILD_RES_PT3) && $(XXD) -i $(notdir $^) > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_music.h

$(built_sfx_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_SFX)/%.afb
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, sfx, $^)
	$(hide) cd $(LOCAL_BUILD_RES_SFX) && $(XXD) -i $(notdir $^) > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sfx.h

$(built_til_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_TIL)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles_ext.h

$(built_raw_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_RAW)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_raw_tiles_ext.h

$(built_spr_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_SPR)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t SPRITEH -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites_ext.h

$(built_til_ext_res_png) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_TIL)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(PNG2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_tiles_ext.h

$(built_raw_ext_res_png) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_RAW)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(PNG2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_raw_tiles_ext.h

$(built_spr_ext_res_png) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_SPR)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(PNG2H) -t SPRITEH -p $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_sprites_ext.h

$(built_pat_res) : $(LOCAL_BUILD_OUT_GEN)/%.pat.h: $(LOCAL_BUILD_RES_SCR)/%.pat
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, scr, $^)
	$(hide) cd $(LOCAL_BUILD_RES_SCR) && $(XXD) -i $(notdir $^) > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_pat.h

$(built_col_res) : $(LOCAL_BUILD_OUT_GEN)/%.col.h: $(LOCAL_BUILD_RES_SCR)/%.col
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, scr, $^)
	$(hide) cd $(LOCAL_BUILD_RES_SCR) && $(XXD) -i $(notdir $^) > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_col.h

$(built_vda_res) : $(LOCAL_BUILD_OUT_GEN)/%.vda.h: $(LOCAL_BUILD_RES_SCR)/%.vda
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, vda, $^)
	$(hide) cd $(LOCAL_BUILD_RES_SCR) && $(XXD) -i $(notdir $^) > $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_vda.h

$(built_fnt_res) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_FNT)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, font, $^)
	$(hide) $(TGA2H) -t TILE -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_fonts.h

$(built_fnt_ext_res) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_FNT)/%.tga
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(TGA2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_fonts_ext.h

$(built_fnt_res_png) : $(LOCAL_BUILD_OUT_GEN)/%.h: $(LOCAL_BUILD_RES_FNT)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(call print_res, font, $^)
	$(hide) $(PNG2H) -t TILE -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_fonts.h

$(built_fnt_ext_res_png) : $(LOCAL_BUILD_OUT_GEN)/%_ext.h: $(LOCAL_BUILD_RES_FNT)/%.png
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_GEN)
	$(hide) $(PNG2H) -t TILEH -f $^ -o $@
	$(hide) @echo '#include "$@"' >> $(LOCAL_BUILD_OUT_GEN)/$(LOCAL_ROM_NAME)_fonts_ext.h
