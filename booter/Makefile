#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(PROJECT)),)
export PROJECT	:=	$(CURDIR)/../
endif

ifneq (,$(shell which python3))
PYTHON	:= python3
else ifneq (,$(shell which python2))
PYTHON	:= python2
else ifneq (,$(shell which python))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

include $(DEVKITARM)/ds_rules

export VERSION_MAJOR	:= 7
export VERSION_MINOR	:= 4
export VERSION_PATCH	:= 0


VERSION	:=	$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files embedded using bin2o
# GRAPHICS is a list of directories containing image files to be converted with grit
#---------------------------------------------------------------------------------
TARGET		:=	booter
BUILD		:=	build
SOURCES		:=	source source/graphics
INCLUDES	:=	include source source/graphics
DATA		:=	data  
GRAPHICS	:=  gfx

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2 \
		-ffunction-sections -fdata-sections \
 		-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
		-ffast-math \
		$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=ds_arm9.specs -g -Wl,--gc-sections $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project (order is important)
#---------------------------------------------------------------------------------
LIBS	:= 	-lfat -lnds9
 
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BMPFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.bmp)))
PNGFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	load.bin bootstub.bin
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(BMPFILES:.bmp=.o) \
					$(PNGFILES:.png=.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

icons := $(wildcard *.bmp)

ifneq (,$(findstring $(TARGET).bmp,$(icons)))
	export GAME_ICON := $(CURDIR)/$(TARGET).bmp
else
	ifneq (,$(findstring icon.bmp,$(icons)))
		export GAME_ICON := $(CURDIR)/icon.bmp
	endif
endif
 
export GAME_TITLE := $(TARGET)

.PHONY: bootloader bootstub arm7/$(TARGET).elf arm9/$(TARGET).elf $(TARGET).nds clean

all:	bootloader bootstub arm7/$(TARGET).elf arm9/$(TARGET).elf $(TARGET).nds

dist:	all
	@mkdir -p ../7zfile/debug
	@mkdir -p "../7zfile/DSi&3DS - SD card users"
	@cp "$(TARGET).nds" "../7zfile/DSi&3DS - SD card users/BOOT.NDS"
	@mkdir -p "../7zfile/DSi - CFW users/SDNAND root/title/00030004/53524c41/content"
	#@mkdir -p "../7zfile/DSi - CFW users/SDNAND root/title/00030015/534c524e/content"
	@cp $(TARGET).nds "../7zfile/DSi - CFW users/SDNAND root/title/00030004/53524c41/content/00000000.app"
	#@cp $(TARGET)_rungame.nds "../7zfile/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

$(TARGET).nds:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $@ -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf \
			-g SRLA 01 "TWLMENUPP" -z 80040000 -u 00030004 -a 00000138 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	$(PYTHON) animatedbannerpatch.py $@ twl_banner.bin

#$(TARGET)_rungame.nds:	$(TARGET).arm7 $(TARGET).arm9
#	ndstool	-c $@ -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf \
#			-g SLRN 01 "TWLMENUPP-LR" -z 80040000 -u 00030015 -a 00000138 -b icon.bmp "Last-run ROM;TWiLight Menu++;Rocket Robz"

ifneq ($(strip $(TWLNOPATCHSRLHEADER)), 1)
		$(PYTHON) ../patch_ndsheader_dsiware.py $(TARGET).nds --twlTouch --structParam
#		$(PYTHON) ../patch_ndsheader_dsiware.py $(TARGET)_rungame.nds --twlTouch --structParam
endif

$(TARGET).arm7: arm7/$(TARGET).elf
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

$(TARGET).arm9: arm9/$(TARGET).elf
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	@$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	@$(MAKE) -C arm9

#---------------------------------------------------------------------------------
#$(BUILD):
	#@[ -d $@ ] || mkdir -p $@
	#@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -rf data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds.orig.nds $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C $(PROJECT)/universal/bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C $(PROJECT)/universal/bootstub BOOTSTUB=$(CURDIR)/data/bootstub.bin

#---------------------------------------------------------------------------------
else
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
#$(OUTPUT).nds	: 	$(OUTPUT).elf
#$(OUTPUT).elf	:	$(OFILES)
 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule creates assembly source files using grit
# grit takes an image file and a .grit describing how the file is to be processed
# add additional rules like this for each image extension
# you use in the graphics folders 
#---------------------------------------------------------------------------------
%.s %.h	: %.bmp %.grit
#---------------------------------------------------------------------------------
	grit $< -fts -o$*


#---------------------------------------------------------------------------------
%.s %.h   : %.png %.grit
#---------------------------------------------------------------------------------
	grit $< -fts -o$*

-include $(DEPSDIR)/*.d
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
