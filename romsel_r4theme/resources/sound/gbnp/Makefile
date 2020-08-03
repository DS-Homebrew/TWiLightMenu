#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

# define SFX_SELECT	0

NAMES			:= select.wav

TARGETS			:= sfx.bin

 
.PHONY: clean all 

all: $(TARGETS)

sfx.bin : $(NAMES)
	@mmutil $^ -d -v -osfx.bin

	
clean:
	@echo Clean ...
