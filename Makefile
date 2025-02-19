includepath = -Iinclude/ -Iexternal/include/
libpath = -Lexternal/lib
libs = -lraylib -lgdi32 -lwinmm
includeall = $(includepath) $(libpath) $(libs)

all:
	@echo "building game"
	g++ src/**.cpp -o collisions $(includeall)
	@./collisions.exe

run: collisions.exe
	@./collisions.exe

collisions.exe: allobj
	@echo "linking object files"
	g++ obj/**.o -o collisions.exe $(includeall)

allobj: obj/main.o obj/gui.o obj/platformer.o obj/levelEditor.o
	@echo "all files compiled"

obj/gui.o: src/gui.cpp include/gui.hpp include/data.hpp
	@echo "compiling gui"
	g++ -c src/gui.cpp -o obj/gui.o $(includeall)

obj/main.o: src/main.cpp include/**.hpp
	@echo "compiling main"
	g++ -c src/main.cpp -o obj/main.o $(includeall)

obj/platformer.o: src/platformer.cpp include/platformer.hpp include/data.hpp
	@echo "compiling platformer engine"
	g++ -c src/platformer.cpp -o obj/platformer.o $(includeall)

obj/levelEditor.o: src/levelEditor.cpp include/levelEditor.hpp include/data.hpp
	@echo "compiling level editor"
	g++ -c src/levelEditor.cpp -o obj/levelEditor.o $(includeall)