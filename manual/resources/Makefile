ifeq ($(OS),Windows_NT)
CHROMIUM	:= chromium.exe
SED			:= sed.exe -i
else
	ifeq ($(shell uname -s),Darwin)
# I use Edge, change this if using another Chromium browser -Pk11
CHROMIUM	:= /Applications/Microsoft\ Edge.app/Contents/MacOS/Microsoft\ Edge
# macOS sed is dumb apparently
SED			:= sed -i ''
	else
CHROMIUM	:= chromium
SED			:= sed -i
	endif
endif

PAGES		:=	*
EXCLUDE		:= Acholi assets
LANGS		:= $(filter-out $(EXCLUDE),$(subst /,,$(wildcard $(PAGES)/)))
NITROFILES	:= ../nitrofiles

INPUTS		:= $(foreach dir,$(LANGS),$(wildcard $(dir)/*.html))
TARGETS		:= $(foreach file,$(INPUTS:.html=.gif),$(addprefix $(NITROFILES)/pages/, $(file)))

INIFILES	:= $(wildcard $(NITROFILES)/pages/english/*.ini)
INITARGETS	:= $(foreach file,$(INIFILES),$(foreach lang,$(filter-out english,$(LANGS)),$(subst english,$(lang),$(file))))

.PHONY: all clean

all:	$(TARGETS)

$(NITROFILES)/pages/%.gif	:	%.html
	@echo $$(basename $(@D))/$$(basename $<)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@# remove 2>&1 to see errors
	$(CHROMIUM) --no-sandbox --hide-scrollbars --headless --disable-gpu --window-size=256,3000 --screenshot "$<" 1>/dev/null 2>&1
	@mogrify -trim screenshot.png
	@ffmpeg -i screenshot.png -vf "palettegen=max_colors=246" palette.png -y -loglevel error
	@ffmpeg -i screenshot.png -i palette.png -filter_complex paletteuse "$@" -y -loglevel error
	@rm -f screenshot.png palette.png

inifiles	:	$(INITARGETS)

$(foreach lang,$(filter-out english,$(LANGS)),$(NITROFILES)/pages/$(lang)/%.ini)	:	$(NITROFILES)/pages/english/%.ini
	@echo $*.ini
	@$(foreach lang,$(filter-out english,$(LANGS)),[ -d "$(NITROFILES)/pages/$(lang)" ] || mkdir -p "$(NITROFILES)/pages/$(lang)";)
	@$(foreach lang,$(filter-out english,$(LANGS)),cp $< $(NITROFILES)/pages/$(lang)/$*.ini;)
	@$(foreach lang,$(LANGS),$(SED) "s/TITLE = .*/TITLE = $(shell xmllint --html $(lang)/$*.html --xpath "//head/title/text()")/" $(NITROFILES)/pages/$(lang)/$*.ini;)

clean:
	@echo clean ...
	@rm -rf $(TARGETS)
	@rm -rf $(INITARGETS)
