

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRIT    		:= grit
GRAPHICS 		:= grit
GRF 			:= grf

GRFOUT	:=	$(CURDIR)/$(GRF)


NAMES			:= bips box brace bubble button_arrow cornerbutton dialogbox folder 
NAMES 			+= icon_gb icon_gba icon_gbamode icon_gg icon_md icon_nes icon_settings 
NAMES 			+= icon_sms icon_snes icon_unk launch_dot moving_arrow progress 
NAMES 			+= scroll_window small_cart start_border start_text wirelessicons

TARGETS			:= $(NAMES:%=$(GRF)/%.grf)

.PHONY: clean all directories

# With matching grit-file
$(GRF)/%.grf: $(GRAPHICS)/%.png $(GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@
# With matching grit-file
$(GRF)/%.grf: $(GRAPHICS)/%.bmp $(GRAPHICS)/%.grit
	@mkdir -p $(@D)
	@$(GRIT) $< -ftr -fh! -o $@

all: $(TARGETS)
	
clean:
	@echo Clean ...
	@rm -rf $(GRFOUT)