#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include "tiffloader/tiffloader.h"

void programLoop();

/* --------------------------- OpenGL/GLUT -------------------------- */

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

float x, maxWidth, maxHeight;
float screenshotDelayMS;
float lastTime;
int currPixNum;
float tearingSimulationDelayMS, startTime;
float tearingSimulationDelayS = 5.0;

int simulateTearing;

struct Vector2f {
    float x;
    float y;
};

#define Vector2f struct Vector2f

void drawLine(Vector2f A, Vector2f B) {
    glLineWidth(1.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(A.x, A.y);
    glVertex2f(B.x, B.y);
    glEnd();
}

void display() {
    // if tearingSimulationDelayMS = 0 there is no tearing simulation

    if(!simulateTearing && tearingSimulationDelayS > 0.0f) {
        float currentTime = glutGet(GLUT_ELAPSED_TIME);
        float timeDiff = currentTime - startTime;
        if(timeDiff >= tearingSimulationDelayMS) {
            simulateTearing = 1;
            startTime = glutGet(GLUT_ELAPSED_TIME);
        }
    } else if(simulateTearing && tearingSimulationDelayS > 0.0f) {
        float currentTime = glutGet(GLUT_ELAPSED_TIME);
        float timeDiff = currentTime - startTime;
        if(timeDiff >= tearingSimulationDelayMS) {
            simulateTearing = 0;
            startTime = glutGet(GLUT_ELAPSED_TIME);
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRectf(-1.0f, 1.0f, 1.0f, -1.0f);

    Vector2f A, B;
    A.x = x / (maxWidth / 2);
    A.y = 1.0;
    B.x = x / (maxWidth / 2);
    B.y = -1.0;

    if(simulateTearing) {
        B.y = 0.0;
    } else {
        B.y = -1.0;
    }

    drawLine(A, B);

    glutSwapBuffers();
}

void moveLine() {
    if(maxWidth <= currPixNum) {
        x = -maxWidth / 2 + 1;
        currPixNum = 0;
    }
    x++;
    currPixNum++;
}

int itsTimeToCheckTearing() {
    float currentTime = glutGet(GLUT_ELAPSED_TIME);

    // in case something fishy happens
    if(lastTime > currentTime) {
        lastTime = currentTime;
    }

    float timeDiff = currentTime - lastTime;
    if(timeDiff >= screenshotDelayMS) {
        lastTime = currentTime;
        return 1;
    }
    return 0;
}

void setup_OpenGl_Glut() {
    maxWidth = glutGet(GLUT_SCREEN_WIDTH);
    maxHeight = glutGet(GLUT_SCREEN_HEIGHT);
    x = -maxWidth / 2 + 1;
    tearingSimulationDelayMS = tearingSimulationDelayS * 1000.0f;
    startTime = glutGet(GLUT_ELAPSED_TIME);
    lastTime = startTime;
    simulateTearing = 0;

    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(maxWidth, maxHeight);
    glutCreateWindow("Tearing checker");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glutFullScreen();
    glutDisplayFunc(display);
    glutIdleFunc(programLoop);
}

/* --------------------------- SCREENSHOT -------------------------- */

Display * dpy;
int screenId;
int screenshotsMade;

uint64_t getNanos()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((uint64_t)1000000000ul*(uint64_t)tv.tv_sec)+((uint64_t)tv.tv_usec * (uint64_t)1000ul);
}

char program_name[] = "tearing";

/*
 * Open_Display: Routine to open a display with correct error handling.
 *               Does not require dpy or screen defined on entry.
 */
Display *Open_Display()
{
	Display *d;

	d = XOpenDisplay(NULL);
	if (d == NULL) {
	    fprintf (stderr, "%s:  unable to open the default X display\n",
		     program_name);
	    exit(1);
	}

	return(d);
}

float toms(uint64_t diffinnano)
{
	return (((float)diffinnano)/1000000.0f);
}

int checkTearing()
{
    dpy = Open_Display();
    screenId = XDefaultScreen(dpy);
	int err = 0;
	int format = ZPixmap;
	uint64_t t0 = getNanos();

	XImage * image = XGetImage(
        dpy,
        RootWindow(dpy, screenId),
        0, 0,
        maxWidth, maxHeight,
        AllPlanes,
        format
    );
	XFlush(dpy);
	uint64_t t1 = getNanos();
	if(image != NULL){

		ImageData iData;
		setWidth(&iData, maxWidth);
		setHeight(&iData, maxHeight);
		char * data = (char*) malloc(maxWidth * maxHeight * 3);
		setPixelData(&iData, data);
		int i, j;

        // the actual check of the tearing
        int saveScreenshot = 0;

        // saving data of top pixel of drawn line
        char columnColorData[3] = {
            image->data[currPixNum * 4 + 2],
            image->data[currPixNum * 4 + 1],
            image->data[currPixNum * 4]
        };
        printf("BASE: %d %d %d\n", columnColorData[0], columnColorData[1], columnColorData[2]);

        int yo = 0;
        for (i = currPixNum * 4; i < maxWidth * maxHeight * 4; i += maxWidth * 4){
 			if(columnColorData[0] != image->data[i]
            || columnColorData[1] != image->data[i + 1]
            || columnColorData[2] != image->data[i + 2] ) {
                saveScreenshot = 1;
                //printf("%d | %d %d %d\n", yo, image->data[i], image->data[i + 1], image->data[i + 2]);
                break;
            }
            yo++;
		}

		for (i = 0, j = 0; i < maxWidth * maxHeight * 4; i += 4, j += 3) {
			data[j + 2] = image->data[i];
			data[j + 1] = image->data[i + 1];
			data[j] = image->data[i + 2];
		}

        if(saveScreenshot) {
            printf("Tearing found, saving screenshot...\n");
            uint64_t t2 = getNanos();

            char fname[80];
            char str[15];
            sprintf(str, "%d", screenshotsMade);
            strcpy(fname, "screenshot_");
            strcat(fname, str);
            screenshotsMade++;
    		int tiffErr = saveImage(&iData, maxWidth * maxHeight * 3, fname);
    		if (tiffErr != 0){
    			err = 100 + tiffErr;
    		}

    		uint64_t t3 = getNanos();
    		printf(
                "Time log: steal: %f ms convert: %f ms filesave %f ms; sum: %f ms;\n",
                toms(t1 - t0), toms(t2 - t1), toms(t3 - t2), toms(t3 - t0)
            );
        }

        XDestroyImage(image);
        free(data);
        XCloseDisplay(dpy);
	} else {
		err = 1;
	}
	return err;
}

void setup_Screenshot() {
    screenshotsMade = 0;
    currPixNum = 0;
}

/* ---------------------------  Program Loop Begin -------------------------- */

void programLoop() {
    // in case it's time: check if screen image is torn
    if(itsTimeToCheckTearing()) {
        checkTearing();
    }

    // epic animation
    moveLine();

    glutPostRedisplay();
}

/* ---------------------------  Program Loop End ---------------------------- */

int main(int argc, char **argv)
{
    float screenshotDelaySec = 5.0f;
    if(argc >= 2) {
        float newDelay = atof(argv[1]);
        if(newDelay != 0) {
            screenshotDelaySec = newDelay;
        }
    }

    screenshotDelayMS = screenshotDelaySec * 1000.0f;
    printf("Screenshot delay: %1.0f\n", screenshotDelaySec);


    glutInit(&argc, argv);
    setup_OpenGl_Glut();
    setup_Screenshot();

    printf("Resolution: %1.0f X %1.0f\n", maxWidth, maxHeight);

    glutMainLoop();

	return 0;
}
