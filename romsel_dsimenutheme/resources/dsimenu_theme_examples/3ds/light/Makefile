

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
BGOUT	:=	$(CURDIR)/$(BG)

NAMES			:= box_empty box_full bubble dialogbox folder cursor
NAMES 			+= icon_gb icon_gba icon_gbamode icon_gg icon_md icon_nes icon_plg icon_settings 
NAMES 			+= icon_sms icon_snes icon_unk progress small_cart wirelessicons

BG_NAMES 		:= bottom bottom_bubble bottom_ds bottom_bubble_ds top
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
	@rm -rf $(BGOUT)