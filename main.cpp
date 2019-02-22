#include "chaosvisual.h"
int main(int argc, char **argv) {
    GLint glut_display;
    glutInit(&argc, argv);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    window = glutCreateWindow(argv[0]);
    init();
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
   // glutFullScreen(); 
    glutDisplayFunc(display);

    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    atexit(deinit);
    glutMainLoop();
    return EXIT_SUCCESS;
}