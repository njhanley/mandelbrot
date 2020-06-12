BINDIR = bin
BINNAME= out

linux:
	mkdir -p $(BINDIR)/$@
	gcc -o $(BINDIR)/$@/$(BINNAME) main.c $(shell sdl2-config --cflags --libs)

win64:
	mkdir -p $(BINDIR)/$@
	x86_64-w64-mingw32-gcc -o $(BINDIR)/$@/$(BINNAME).exe main.c $(shell x86_64-w64-mingw32-sdl2-config --cflags --libs)
	cp $(shell x86_64-w64-mingw32-sdl2-config --prefix)/bin/SDL2.dll $(BINDIR)/$@

.PHONY: clean

clean:
	rm -rf $(BINDIR)
