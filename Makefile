includepath = -Iinclude/ -Iexternal/include/
libpath = -Lexternal/lib
libs = -lraylib -lgdi32 -lwinmm

all:
	@echo "building game"
	g++ src/**.cpp -o collisions $(includepath) $(libpath) $(libs)
	@./collisions.exe