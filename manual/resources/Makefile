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
EXCLUDE		:= Acholi assets Makefile webserver.pid
LANGS		:= $(filter-out $(EXCLUDE),$(wildcard $(PAGES)))
NITROFILES	:= ../nitrofiles

BMPFILES	:= $(foreach dir,$(LANGS),$(wildcard $(NITROFILES)/pages/$(dir)/*.bmp))

INPUTS		:= $(foreach dir,$(LANGS),$(wildcard $(dir)/*.html))
TARGETS		:= $(filter-out $(subst bmp,png,$(BMPFILES)),$(foreach file,$(INPUTS:.html=.png),$(addprefix $(NITROFILES)/pages/, $(file))))

INIFILES	:= $(wildcard $(NITROFILES)/pages/english/*.ini)
INITARGETS	:= $(foreach file,$(INIFILES),$(foreach lang,$(LANGS),$(subst english,$(lang),$(file))))

.PHONY: all webserver kill-webserver clean

all:	kill-webserver webserver $(TARGETS) kill-webserver

webserver:
	@# remove 2>&1 to see errors
	@python -m SimpleHTTPServer 1>/dev/null 2>&1 & echo $$! > webserver.pid

kill-webserver:
	@if [ -f webserver.pid ]; then kill $$(cat webserver.pid); fi
	@rm -f webserver.pid

$(NITROFILES)/pages/%.png	:	%.html
	@echo $$(basename $(@D))/$$(basename $<)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@# remove 2>&1 to see errors
	$(CHROMIUM) --no-sandbox --hide-scrollbars --headless --disable-gpu --window-size=256,3000 --screenshot "http://localhost:8000/$<" 1>/dev/null 2>&1
	@mv screenshot.png $@
	@mogrify -trim "$@"
	@if [ $$(identify -format "%h" "$@") -gt 1000 ]; then ffmpeg -i "$@" -pix_fmt rgb555le "$(subst png,bmp,$@)" -y -loglevel error; rm "$@"; fi

inifiles	:	$(INITARGETS)

$(foreach lang,$(filter-out english,$(LANGS)),$(NITROFILES)/pages/$(lang)/%.ini)	:	$(NITROFILES)/pages/english/%.ini
	@echo $*.ini
	@$(foreach lang,$(filter-out english,$(LANGS)),[ -d "$(NITROFILES)/pages/$(lang)" ] || mkdir -p "$(NITROFILES)/pages/$(lang)";)
	@$(foreach lang,$(filter-out english,$(LANGS)),cp $< $(NITROFILES)/pages/$(lang)/$*.ini;)
	@$(foreach lang,$(LANGS),$(SED) "s/TITLE = .*/TITLE = $(shell xmllint --html $(lang)/$*.html --xpath "//head/title/text()")/" $(NITROFILES)/pages/$(lang)/$*.ini;)

clean:
	@echo clean ...
	@rm -rf $(NITROFILES)/pages/*/*.png
	@rm -rf $(NITROFILES)/pages/*/*.bmp
