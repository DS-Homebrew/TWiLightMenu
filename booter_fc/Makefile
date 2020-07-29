#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
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
INCLUDES	:=	include
DATA		:=	data  
GRAPHICS	:=  gfx

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2\
 		-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
		-ffast-math \
		$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9 -DNO_SDMMC
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

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
BINFILES	:=	load.bin bootstub.bin exceptionstub.bin
 
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

.PHONY: bootloader bootstub clean arm7/$(TARGET).elf arm7_cyclodsi/$(TARGET).elf arm9/$(TARGET).elf arm9_cyclodsi/$(TARGET).elf

all:	bootloader bootstub $(TARGET).nds
	
dist:	all
	@cp "booter_fc.nds" "../7zfile/Flashcard users/BOOT.NDS"
	@cp "booter_cyclodsi.nds" "../7zfile/Flashcard users/BOOT_cyclodsi.NDS"

info_scfw:
	ndstool -i SCFW_og.NDS

info_custom:
	ndstool -i $(TARGET)_scdso.nds

$(TARGET).nds:	$(TARGET).arm7 $(TARGET)_cyclodsi.arm7 $(TARGET).arm9 $(TARGET)_cyclodsi.arm9
  # simple nds srl without dsi extended header
	ndstool	-h 0x200 -c $(TARGET)_fc.nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -b icon.bmp "TWiLight Menu++;RocketRobz"
	ndstool -h 0x200 -c $(TARGET)_scdso.nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -g ENG0 00 "DSONE" -r9 0x2000000 -r7 0x37F8000 -e9 0x2000000 -e7 0x37F8000 
	ndstool	-h 0x200 -c $(TARGET)_cyclodsi.nds -7 $(TARGET)_cyclodsi.arm7.elf -9 $(TARGET)_cyclodsi.arm9.elf -b icon.bmp "TWiLight Menu++;RocketRobz"

$(TARGET).arm7: arm7/$(TARGET).elf
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

$(TARGET)_cyclodsi.arm7: arm7_cyclodsi/$(TARGET).elf
	cp arm7_cyclodsi/$(TARGET).elf $(TARGET)_cyclodsi.arm7.elf

$(TARGET).arm9: arm9/$(TARGET).elf
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

$(TARGET)_cyclodsi.arm9: arm9_cyclodsi/$(TARGET).elf
	cp arm9_cyclodsi/$(TARGET).elf $(TARGET)_cyclodsi.arm9.elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	@$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm7_cyclodsi/$(TARGET).elf:
	@$(MAKE) -C arm7_cyclodsi

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	@$(MAKE) -C arm9

#---------------------------------------------------------------------------------
arm9_cyclodsi/$(TARGET).elf:
	@$(MAKE) -C arm9_cyclodsi

#---------------------------------------------------------------------------------
#$(BUILD):
	#@[ -d $@ ] || mkdir -p $@
	#@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm7_cyclodsi.elf
	@rm -fr $(TARGET).arm9.elf
	@rm -fr $(TARGET).arm9_cyclodsi.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm9_cyclodsi clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C arm7_cyclodsi clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C bootstub

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
