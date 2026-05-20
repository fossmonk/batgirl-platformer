CFLAGS= -Wall -D__USE_MINGW_ANSI_STDIO=1 -O2
LFLAGS= -lopengl32 -lgdi32 -lm
PACKAGE=package
APP=batg
SRCS=main.c tigr.c
RM_RF=rmdir /s /q
PKGFLAGS=-DPACKAGE=1 -mwindows
RC=windres
COMPRESS=tar -a -c -f

all:
	@$(RC) resource.rc -O coff -o rc.o
	@cc $(SRCS) rc.o $(CFLAGS) $(LFLAGS) -o $(APP) && $(APP).exe

pkg:
	@if exist $(PACKAGE).zip del $(PACKAGE).zip
	@mkdir $(PACKAGE)
	@$(RC) resource.rc -O coff -o rc.o
	@cc $(SRCS) rc.o $(CFLAGS) $(PKGFLAGS) $(LFLAGS) -o $(PACKAGE)\$(APP)
	@xcopy /s /i sprites $(PACKAGE)\sprites > nul
	@xcopy /s /i fonts $(PACKAGE)\fonts > nul
	@$(COMPRESS) $(PACKAGE).zip $(PACKAGE)
	@$(RM_RF) $(PACKAGE)


clean:
	@if exist $(APP).exe del $(APP).exe