#include "shapes.hpp"
static void draw_circle(float x0, float y0,float raio,clr color){
    glBegin(GL_POLYGON);
    for (int i=0; i < 360; i++){
        float degInRad = i*3.14159/180;
        glColor3f(color.r,color.g,color.b);
        glVertex2f(x0+cos(degInRad)*(raio),y0+sin(degInRad)*(raio));
    }
    glEnd();
}
static void draw_square(float x0,float y0, float raio, clr color){
    glColor3f(color.r,color.g,color.b);
    float vertexDistCenter = raio*sqrt(2.0)/2.0;
    glBegin(GL_POLYGON);
        glVertex2f(x0+vertexDistCenter,y0+vertexDistCenter);
        glVertex2f(x0+vertexDistCenter,y0-vertexDistCenter);
        glVertex2f(x0-vertexDistCenter,y0-vertexDistCenter);
        glVertex2f(x0-vertexDistCenter,y0+vertexDistCenter);
    glEnd();
}
static void draw_torus(float x0,float y0,float raio,clr color){
    glColor3f(color.r,color.g,color.b);
    glPushMatrix();
    glTranslatef(x0,y0,0.0);
    glutSolidTorus(raio/2.0, raio, 50, 50);
    glPopMatrix();
}
static void draw_teapot(float x0,float y0,float raio,clr color){
    glColor3f(color.r,color.g,color.b);
    glPushMatrix();
    glTranslatef(x0,y0,0.0);
    glutSolidTeapot(raio);
    glPopMatrix();
}
static void draw_sphere(float x0, float y0, float raio, clr color){
    glColor3f(color.r,color.g,color.b);
    glPushMatrix();
    glTranslatef(x0,y0,0.0);
    glutSolidSphere(raio,50,50);
    glPopMatrix();   
}
static void draw_cube(float x0, float y0, float raio, clr color){
    glColor3f(color.r,color.g,color.b);
    glPushMatrix();
    glTranslatef(x0,y0,0.0);
    glutSolidCube(raio);
    glPopMatrix();
}
static void draw_cone(float x0, float y0, float raio, clr color){
    glColor3f(color.r,color.g,color.b);
    glPushMatrix();
    glTranslatef(x0,y0,0.0);
    glutSolidCone(raio/2,raio,50,50);
    glPopMatrix();
}
static void draw_triangle(float x0, float y0, float raio, clr color){
     glColor3f(color.r,color.g,color.b);
    float vertexDistCenter = raio*sqrt(2.0)/2.0;
    glBegin(GL_POLYGON);
        glVertex2f(x0,y0+vertexDistCenter);
        glVertex2f(x0+vertexDistCenter,y0-vertexDistCenter);
        glVertex2f(x0-vertexDistCenter,y0-vertexDistCenter);
    glEnd();
}