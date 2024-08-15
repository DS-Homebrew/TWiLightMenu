export ARM7_MAJOR	:= 0
export ARM7_MINOR	:= 6
export ARM7_PATCH	:= 0

VERSTRING	:=	$(ARM7_MAJOR).$(ARM7_MINOR).$(ARM7_PATCH)
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
UNIVERSAL	:=	../../universal
TARGET		:=	romsel_dsimenutheme
BUILD		:=	build
SOURCES		:=	source $(UNIVERSAL)/arm7/source $(UNIVERSAL)/sdmmc/arm7/source
INCLUDES	:=	include build $(UNIVERSAL)/include $(UNIVERSAL)/arm7/include $(UNIVERSAL)/sdmmc/arm7/include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv4t -mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2 \
 		-mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer\
		-ffast-math \
		-flto \
		$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM7

ASFLAGS	:=	-g $(ARCH) -flto
LDFLAGS	=	-specs=ds_arm7.specs -g $(ARCH) -Wl,--nmagic -Wl,-Map,$(notdir $*).map


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lmm7 -lnds7


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

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export OUTPUT	:=	$(CURDIR)/$(TARGET)

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

.PHONY: all $(BUILD) clean

all : $(BUILD)

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile


#---------------------------------------------------------------------------------
dist: all
#---------------------------------------------------------------------------------
	@tar --exclude=*CVS* --exclude=.svn -cvjf default_arm7-src-$(VERSTRING).tar.bz2 source Makefile
	@tar -cvjf default_arm7-$(VERSTRING).tar.bz2 default.elf

#---------------------------------------------------------------------------------
install: all
#---------------------------------------------------------------------------------
	cp $(TARGET).elf $(LIBNDS)

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf  $(TARGET).arm7


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

$(OUTPUT).elf	:	$(OFILES) $(LIBNDS)/lib/libnds7.a

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
