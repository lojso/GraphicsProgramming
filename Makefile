build:
	gcc -Wall \
	-std=c99 \
	./src/*.c \
	-I"C:\_Tools\libsdl\include" \
	-L"C:\_Tools\libsdl\lib" \
	-lmingw32 \
	-lSDL2main \
	-lSDL2 \
	-o example.exe