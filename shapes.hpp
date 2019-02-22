#ifndef SHAPES_H
#define SHAPES_H
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <math.h>


//estrutura color com valores de rgb em float
typedef struct color{
    float r;
    float g;
    float b;
}clr;
//objeto desenhado no canvas, com tipo de objeto, posição inicial, raio e cor
typedef struct objectCanvas{
    int type;
    float x0;
    float y0;
    float raio;
    clr color;
}obC;
//Função para desenhar circulo
static void draw_circle(float x0, float y0,float raio,clr color);
//Função para desenhar quadrado
static void draw_square(float x0,float y0, float raio, clr color);
//Função para desenhar torus
static void draw_torus(float x0,float y0,float raio,clr color);
//Função para desenhar teapot
static void draw_teapot(float x0,float y0,float raio,clr color);
//Função para desenhar esfera
static void draw_sphere(float x0, float y0, float raio, clr color);
//Função para desenhar cubo
static void draw_cube(float x0, float y0, float raio, clr color);
//Função para desenhar cone
static void draw_cone(float x0, float y0, float raio, clr color);
//Função para desenhar triangulo
static void draw_triangle(float x0, float y0, float raio, clr color);
#endif