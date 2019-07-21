#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

# define SFX_WRONG	0
# define SFX_LAUNCH	1
# define SFX_STOP	2
# define SFX_SWITCH	3
# define SFX_STARTUP	4
# define SFX_SELECT	5
# define SFX_BACK	6

NAMES			:= startup.wav wrong.wav launch.wav stop.wav switch.wav select.wav back.wav

TARGETS			:= sfx.bin

 
.PHONY: clean all 

all: $(TARGETS)

sfx.bin : $(NAMES)
	@mmutil $^ -d -v -osfx.bin

	
clean:
	@echo Clean ...
	@rm sfx.bin