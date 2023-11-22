INCLUDE = -I include/ -I vendor/ -I vendor/GLAD/include/ -I vendor/glm/ -I vendor/GLFW/include
LIB = vendor/graphicsLibrary/lib/graphicsLib.lib
DLL = bin/glfw3.dll

build: src/main.cpp .o/glad.o
	g++ src/main.cpp -o bin/main.exe $(INCLUDE) $(LIB) $(DLL) .o/glad.o

run: bin/main.exe
	./bin/main.exe

.o/glad.o: vendor/GLAD/src/glad.c
	g++ vendor/GLAD/src/glad.c -c -o .o/glad.o -I vendor/GLAD/include/