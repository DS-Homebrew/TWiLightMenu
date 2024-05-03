BDFFILES = $(foreach dir,.,$(notdir $(wildcard $(dir)/*.bdf)))
PCFFILES = $(patsubst %.bdf,%.pcf,$(BDFFILES))

all: $(PCFFILES)

clean:
	rm *.pcf

%.pcf: %.bdf
	@echo $(notdir $<)
	bdftopcf -p1 -l -L $< -o $@

%.bdf: %.ttf
	@echo $(notdir $<)
	otf2bdf -p 10 -r 72 -o $@ $<
