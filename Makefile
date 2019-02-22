all: main

main: mainc shapesc
	g++ -o main integracao.o shapes.o -lX11 -lXi -lglut -lGL -lGLU -lm -lasound -fopenmp -lpthread

mainc: integracao.cpp
	g++ -c integracao.cpp -lX11 -lXi -lglut -lGL -lGLU -lm  -lasound -fopenmp -lpthread

shapesc: shapes.cpp
	g++ -c shapes.cpp -lX11 -lXi -lglut -lGL -lGLU -lm  