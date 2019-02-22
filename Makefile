all: main

main: mainc chaosvisualc
	g++ -o main main.o chaosvisual.o -lX11 -lXi -lglut -lGL -lGLU -lm 

mainc: main.cpp
	g++ -c main.cpp -lX11 -lXi -lglut -lGL -lGLU -lm 

chaosvisualc: chaosvisual.cpp
	g++ -c chaosvisual.cpp -lX11 -lXi -lglut -lGL -lGLU -lm  