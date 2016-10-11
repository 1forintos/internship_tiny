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

uint64_t getNanos()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return ((uint64_t)1000000000ul*(uint64_t)tv.tv_sec)+((uint64_t)tv.tv_usec * (uint64_t)1000ul);
}

char program_name[]="tearing";

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

int Window_DumpTiff(Display * dpy, int screenId, int x, int y, int width, int height, char * fname)
{
	int err=0;
	int format=ZPixmap;
	uint64_t t0=getNanos();
	XImage * image = XGetImage (dpy, RootWindow(dpy, screenId), x, y,
                    width, height, AllPlanes, format);
	XFlush(dpy);
	uint64_t t1=getNanos();
	if(image!=NULL){

		ImageData iData;
		setWidth(&iData,width);
		setHeight(&iData,height);
		char * data = (char*)malloc(width*height*3);
		setPixelData(&iData,data);
		int i,j;
		for (i = 0, j = 0; i< width * height*4;i+=4,j+=3){
			data[j+2] = image->data[i];
			data[j+1] = image->data[i+1];
			data[j] = image->data[i+2];
		}
		uint64_t t2=getNanos();
		int tiffErr = saveImage(&iData, width * height * 3, fname);
		if (tiffErr != 0){
			err = 100+tiffErr;
		}
		uint64_t t3=getNanos();
		XDestroyImage(image);
		free(data);
		printf("Time log: steal: %f ms convert: %f ms filesave %f ms; sum: %f ms;\n", toms(t1-t0), toms(t2-t1), toms(t3-t2), toms(t3-t0));
	} else {
		err=1;
	}
	return err;
}

Display * dpy;
int screenId;

int main(int argc, char **argv)
{
    int x = 150;
    int y = 150;
    int width = 500;
    int heigth = 500;
    char * fname = "file.tiff";
	dpy=Open_Display();
	screenId = XDefaultScreen(dpy);
    Window_DumpTiff(dpy, screenId, x, y, width, heigth, fname);
	XCloseDisplay(dpy);
	return 0;
}
