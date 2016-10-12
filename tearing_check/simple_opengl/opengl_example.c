#include <stdio.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

float x, y, maxWidth, maxHeight;
float lastTime;
int frames;
struct Vector2f {
    float x;
    float y;
};

#define Vector2f struct Vector2f

void setup() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawLine(Vector2f A, Vector2f B) {
    glLineWidth(1.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(A.x, A.y);
    glVertex2f(B.x, B.y);
    glEnd();
}

void display() {
    /*float currentTime = glutGet(GLUT_ELAPSED_TIME);
    frames++;
    float num = currentTime - lastTime;
    printf("\n");
    if(num > 1000.0f) {
        printf("%d FPS", frames);
        lastTime = currentTime;
        frames = 0;
    }*/
    Vector2f A, B;
    A.x = x/maxWidth;
    A.y = 1.0;
    B.x = x/maxWidth;
    B.y = -1.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRectf(-1.0f, 1.0f, 1.0f, -1.0f);

    drawLine(A, B);
    glutSwapBuffers();
}

void updateScene() {
    if(maxWidth <= x) {
        x = -maxWidth;
    }
    x++;

    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    x = -1920;
    lastTime = 0.0f;
    frames = 0;

    glutInit(&argc, argv);

    maxWidth = glutGet(GLUT_SCREEN_WIDTH);
    maxHeight = glutGet(GLUT_SCREEN_HEIGHT);
    printf("Resolution: %1.0f X %1.0f\n", maxWidth, maxHeight);

    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(maxWidth, maxHeight);
    glutCreateWindow("913");

    setup();
    glutFullScreen();
    glutDisplayFunc(display);
    glutIdleFunc(updateScene);

    glutMainLoop();

    return 0;
}
