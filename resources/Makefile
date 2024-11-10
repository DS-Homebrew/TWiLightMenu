ifneq (,$(shell which python3))
PYTHON	:= python3
else ifneq (,$(shell which python2))
PYTHON	:= python2
else ifneq (,$(shell which python))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

#---------------------------------------------------------------------------------
# OUTPUT_DIR is the directory where final published files will be placed
#---------------------------------------------------------------------------------
OUTPUT_DIR		:=	../7zfile/_nds/TWiLightMenu/extras

WIDEFILES	:= widescreen

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY:

all:	$(OUTPUT_DIR)/widescreen.pck

$(OUTPUT_DIR)/widescreen.pck: $(WIDEFILES)
	$(PYTHON) pack.py $^ -o $@
