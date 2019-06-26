CC = cl
L = link
CFLAGS = /nologo /W3 /DLL_IMPORTS

all: build

build: libscheduler.lib

libscheduler.lib: libscheduler.dll

libscheduler.dll: so_scheduler.obj priority_table.obj t_list.obj
	$(L) /nologo /dll /out:$@ /implib:libscheduler.lib $**

so_scheduler.obj: so_scheduler.c
	$(CC) $(CFLAGS) /c $**
priority_table.obj: priority_table.c
	$(CC) $(CFLAGS) /c $**
t_list.obj: t_list.c
	$(CC) $(CFLAGS) /c $**

clean:
	del /Q /F *.obj *.lib *.dll *.exe  *.exp