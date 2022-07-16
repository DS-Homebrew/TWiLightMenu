

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRIT    		:= grit
GRAPHICS 		:= grit
BG_GRAPHICS 	:= background_grit
GRF 			:= grf
BG 				:= background

GRFOUT	:=	$(CURDIR)/$(GRF)


NAMES			:= bips box brace bubble button_arrow cornerbutton dialogbox folder 
NAMES 			+= icon_a26 icon_col icon_int icon_gb icon_gba icon_gbamode icon_gg icon_img icon_m5 icon_md icon_nes icon_ngp icon_plg icon_settings 
NAMES 			+= icon_sg icon_sms icon_snes icon_pce icon_unk icon_vid icon_ws launch_dot moving_arrow progress icon_manual
NAMES 			+= scroll_window small_cart start_border start_text wirelessicons

BG_NAMES 		:= bottom bottom_bubble bottom_bubble_macro bottom_moving top
TARGETS			:= $(NAMES:%=$(GRF)/%.grf) $(BG_NAMES:%=$(BG)/%.grf)

.PHONY: clean all directories

# With matching grit-file
$(GRF)/%.grf: $(GRAPHICS)/%.png $(GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@
# With matching grit-file
$(GRF)/%.grf: $(GRAPHICS)/%.bmp $(GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@

# With matching grit-file
$(BG)/%.grf: $(BG_GRAPHICS)/%.png $(BG_GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@
# With matching grit-file
$(BG)/%.grf: $(BG_GRAPHICS)/%.bmp $(BG_GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@

all: $(TARGETS)
	
clean:
	@echo Clean ...
	@rm -rf $(GRFOUT)