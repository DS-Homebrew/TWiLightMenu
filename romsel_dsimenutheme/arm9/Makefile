#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

CURRENT_SCREEN_MODE := 3

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files embedded using bin2o
# GRAPHICS is a list of directories containing image files to be converted with grit
# MAXMOD_SOUNDBANK contains a directory of music and sound effect files
#---------------------------------------------------------------------------------
UNIVERSAL	:=	../../universal
TARGET		:=	romsel_dsimenutheme
BUILD		:=	build
SOURCES		:=	source source/common source/graphics source/tool $(UNIVERSAL)/source $(UNIVERSAL)/arm9/source $(UNIVERSAL)/source/common $(UNIVERSAL)/source/flashcard $(UNIVERSAL)/source/nds_loader $(UNIVERSAL)/source/tonccpy $(UNIVERSAL)/source/lodepng $(UNIVERSAL)/sdmmc/arm9/source
INCLUDES	:=	include source $(UNIVERSAL)/include $(UNIVERSAL)/arm9/include $(UNIVERSAL)/sdmmc/arm9/include
DATA		:=	../data  
GRAPHICS	:=  ../gfx
MUSIC       :=  ../music
MUSIC2      :=  ../musicShop

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2 \
 			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS		+=	$(INCLUDE) -DARM9 -DSTANDALONE -DCURRENT_SCREEN_MODE=$(CURRENT_SCREEN_MODE)
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17

ASFLAGS	:=	-g $(ARCH) $(INCLUDE)
LDFLAGS	=	-specs=../$(UNIVERSAL)/arm9/ds_arm9_twlm.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project (order is important)
#---------------------------------------------------------------------------------
LIBS	:= 	-lfat -lmm9 -lnds9
 
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(CURDIR) ../ $(LIBNDS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export AUDIOFILES	:=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.*)),$(CURDIR)/$(MUSIC)/$(dir))
export AUDIOFILES2	:=	$(foreach dir,$(notdir $(wildcard $(MUSIC2)/*.*)),$(CURDIR)/$(MUSIC2)/$(dir))

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BMPFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.bmp)))
PNGFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

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
					$(CPPFILES:.cpp=.o) \
					$(CFILES:.c=.o) \
					$(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)


export OUTPUT	:=	$(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean

all : $(BUILD)

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds *.bin 

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).elf	:	$(OFILES)

# #---------------------------------------------------------------------------------
# # rule to build soundbank from music files
# #---------------------------------------------------------------------------------
# soundbank.bin : $(AUDIOFILES)
# #---------------------------------------------------------------------------------
# @mmutil $^ -d -osoundbank.bin -hsoundbank.h

# #---------------------------------------------------------------------------------
# soundbank_shop.bin : $(AUDIOFILES2)
# #---------------------------------------------------------------------------------
# 	@mmutil $^ -d -o"../../nitrofiles/soundbank_shop.bin" -h"soundbank_shop.h"

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
