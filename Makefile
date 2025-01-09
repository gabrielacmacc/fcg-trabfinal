./bin/Linux/main: src/main.cpp src/glad.c include/matrices.h include/utils/error_utils.h include/external/dejavufont.h
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/Linux/main src/main.cpp src/glad.c include/external/tiny_obj_loader.cpp include/external/stb_image.cpp ./libs/lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run
clean:
	rm -f bin/Linux/main

run: ./bin/Linux/main
	cd bin/Linux && ./main
