TEX = pdflatex
TEXFLAGS = -interaction=nonstopmode -halt-on-error
MAIN = informe

.PHONY: pdf clean view

pdf: $(MAIN).pdf

$(MAIN).pdf: $(MAIN).tex
	$(TEX) $(TEXFLAGS) $(MAIN).tex
	$(TEX) $(TEXFLAGS) $(MAIN).tex

view: pdf
	open $(MAIN).pdf

clean:
	rm -f $(MAIN).aux $(MAIN).log $(MAIN).out $(MAIN).toc $(MAIN).pdf
