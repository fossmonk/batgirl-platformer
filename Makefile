CFLAGS= -Wall -D__USE_MINGW_ANSI_STDIO=1 -O2
LFLAGS= -lopengl32 -lgdi32 -lm
PACKAGE=package
APP=batg
SRCS=main.c winmagic.c game.c font.c anim.c tigr/tigr.c
RM_RF=rmdir /s /q
PKGFLAGS=-DPACKAGE=1 -mwindows
RC=windres
COMPRESS=tar -a -c -f

all:
	@$(RC) res/resource.rc -O coff -o rc.o
	@cc $(SRCS) rc.o $(CFLAGS) $(LFLAGS) -o $(APP) && $(APP).exe
	@del rc.o

pkg:
	@if exist $(PACKAGE).zip del $(PACKAGE).zip
	@mkdir $(PACKAGE)
	@$(RC) res/resource.rc -O coff -o rc.o
	@cc $(SRCS) rc.o $(CFLAGS) $(PKGFLAGS) $(LFLAGS) -o $(PACKAGE)\$(APP)
	@xcopy /s /i res $(PACKAGE)\res > nul
	@$(COMPRESS) $(PACKAGE).zip $(PACKAGE)
	@$(RM_RF) $(PACKAGE)
	@del rc.o


clean:
	@if exist $(APP).exe del $(APP).exe