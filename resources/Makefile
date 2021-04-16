#---------------------------------------------------------------------------------
# OUTPUT_DIR is the directory where final published files will be placed
#---------------------------------------------------------------------------------
OUTPUT_DIR		:=	../7zfile/_nds/TWiLightMenu/extras

APFILES		:= apfix apfix/cht
WIDEFILES	:= widescreen

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY:

all:	$(OUTPUT_DIR)/apfix.pck $(OUTPUT_DIR)/widescreen.pck

$(OUTPUT_DIR)/apfix.pck: $(APFILES)
	python2 pack.py $^ -o $@

$(OUTPUT_DIR)/widescreen.pck: $(WIDEFILES)
	python2 pack.py $^ -o $@
