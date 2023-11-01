INCLDUE = -I ./include -I ./vendor/include

build: src/main.cpp
	g++ src/main.cpp -o bin/main.exe $(INCLUDE)

run: bin/main.exe
	./bin/main.exe