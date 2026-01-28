#include <GL/freeglut.h> 
#include "audio.h"

float angleDeg = 0.0f;

void drawCube()
{
    glBegin(GL_QUADS);
    // +X (red)
    glColor3f(1,0,0);
    glNormal3f(1,0,0);
    glVertex3f( 0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);

    // -X (green)
    glColor3f(0,1,0);
    glNormal3f(-1,0,0);
    glVertex3f(-0.5f,-0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glVertex3f(-0.5f,-0.5f,-0.5f);

    // +Y (blue)
    glColor3f(0,0,1);
    glNormal3f(0,1,0);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);

    // -Y (yellow)
    glColor3f(1,1,0);
    glNormal3f(0,-1,0);
    glVertex3f(-0.5f,-0.5f, 0.5f);
    glVertex3f(-0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);

    // +Z (magenta)
    glColor3f(1,0,1);
    glNormal3f(0,0,1);
    glVertex3f(-0.5f,-0.5f, 0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // -Z (cyan)
    glColor3f(0,1,1);
    glNormal3f(0,0,-1);
    glVertex3f( 0.5f,-0.5f,-0.5f);
    glVertex3f(-0.5f,-0.5f,-0.5f);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Simple translate back so we can see the cube
    glTranslatef(0.0f, 0.0f, -3.0f);

    // Rotate the cube
    glRotatef(angleDeg, 1.0f, 1.0f, 0.0f);

    drawCube();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Perspective
    gluPerspective(60.0, (double)w / (double)h, 0.1, 100.0);
}

void timer(int)
{
    angleDeg += 0.5f;            // rotation speed (deg per tick)
    if (angleDeg >= 360.f) angleDeg -= 360.f;
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void keyboard(unsigned char key, int, int)
{
    if (key == 27) glutLeaveMainLoop(); // ESC to quit (FreeGLUT)
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rotating Cube (GLUT)");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
