/*
 * fonttest.c
 * This file is used to display the PSP's internal font (pgf firmware files)
 * intraFont Version 0.21 by BenHur - http://www.psp-programming.com/benhur
 *
 * Uses parts of pgeFont by InsertWittyName - http://insomniac.0x89.org
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <time.h>     //only needed for the blinking effect
#include <pspdebug.h> //only needed for indicating loading progress

#include "graphics.h"

#include "../../intraFont.h"

PSP_MODULE_INFO("FontTest", PSP_MODULE_USER, 1, 1);

static int running = 1;

int exit_callback(int arg1, int arg2, void *common) {
	running = 0;
	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("CallbackThread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if (thid >= 0) sceKernelStartThread(thid, 0, 0);
	return thid;
}

int main() {
	pspDebugScreenInit();
	SetupCallbacks();
    
    pspDebugScreenPrintf("intraFont 0.21 - 2007 by BenHur\n\nLoading fonts: 0%%");
        
    // Init intraFont library
    intraFontInit();
    
     // Load fonts
    intraFont* ltn[16];                                         //latin fonts (large/small, with/without serif, regular/italic/bold/italic&bold)
    char file[40];
    int i;
    for (i = 0; i < 16; i++) {
        sprintf(file, "flash0:/font/ltn%d.pgf", i); 
        ltn[i] = intraFontLoad(file,0);                         //<- this is where the actual loading happens
		pspDebugScreenSetXY(15,2);
        pspDebugScreenPrintf("%d%%",(i+1)*100/19);
    }
    
    intraFont* jpn0 = intraFontLoad("flash0:/font/jpn0.pgf",0); //japanese font (and parts of latin)
    pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("%d%%",17*100/19);
        
	intraFont* kr0 = intraFontLoad("flash0:/font/kr0.pgf",0);   //Korean font (not available on all systems)
	pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("%d%%",18*100/19);
           
	intraFont* arib = intraFontLoad("flash0:/font/arib.pgf",0); //Symbols (not available on all systems)
	pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("done\n");

	// Make sure the important fonts for this application are loaded
	if(!ltn[0] || !ltn[4] || !ltn[8]) sceKernelExitGame();
	
	
	initGraphics();

	// Colors
	enum colors {
		RED =	0xFF0000FF,
		GREEN =	0xFF00FF00,
		BLUE =	0xFFFF0000,
		WHITE =	0xFFFFFFFF,
		LITEGRAY = 0xFFBFBFBF,
		GRAY =  0xFF7F7F7F,
		DARKGRAY = 0xFF3F3F3F,		
		BLACK = 0xFF000000
	};

	while(running)
	{
		clearScreen(GRAY);

		// Must be called before any of the intraFont functions
		guStart();

		// Draw various text
        float x,y = 25;
		intraFontSetStyle(ltn[4], 1.0f,BLACK,WHITE,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn[4], 240, y, "intraFont 0.21 - 2007 by BenHur");
        intraFontSetStyle(ltn[4], 1.0f,WHITE,BLACK,0);
        
		y += 30;
		intraFontPrint(ltn[8],  10, y, "Latin Sans-Serif: ");
		intraFontPrint(ltn[0], 180, y, "regular, ");
        intraFontPrint(ltn[2], 270, y, "italic, ");
        intraFontPrint(ltn[4], 330, y, "bold, ");
        intraFontPrint(ltn[6], 390, y, "both");
        
        y += 20;
        intraFontPrint(ltn[8],  10, y, "Latin Sans-Serif small: ");
		intraFontPrint(ltn[8], 180, y, "regular, ");
        intraFontPrint(ltn[10], 270, y, "italic, ");
        intraFontPrint(ltn[12], 330, y, "bold, ");
        intraFontPrint(ltn[14], 390, y, "both");
        
        y += 20;
        intraFontPrint(ltn[8],  10, y, "Latin with Serif: ");
		intraFontPrint(ltn[1], 180, y, "regular, ");
        intraFontPrint(ltn[3], 270, y, "italic, ");
        intraFontPrint(ltn[5], 330, y, "bold, ");
        intraFontPrint(ltn[7], 390, y, "both");
        
        y += 20;
        intraFontPrint(ltn[8],  10, y, "Latin with Serif small: ");
		intraFontPrint(ltn[9], 180, y, "regular, ");
        intraFontPrint(ltn[11], 270, y, "italic, ");
        intraFontPrint(ltn[13], 330, y, "bold, ");
        intraFontPrint(ltn[15], 390, y, "both");
        
        y += 25;
        intraFontPrintf(ltn[8], 10, y, "Japanese: ");
        unsigned short ucs2_jpn[8]    = { 0x3053, 0x3093, 0x306b, 0x3061, 0x306f, 0x4e16, 0x754c, 0 };
        x = intraFontPrintUCS2(jpn0, 80, y, ucs2_jpn); 
		if (x == 80) intraFontPrint(ltn[8], 80, y, "[n/a]");
        
        intraFontPrintf(ltn[8], 250, y, "Korean: ");
		unsigned short ucs2_kr[8]    = { 0xAC00, 0xAC01, 0xAC02, 0xAC03, 0xAC04, 0xAC05, 0xAC06, 0 };
        x = intraFontPrintUCS2(kr0, 320, y, ucs2_kr); 
		if (x == 320) intraFontPrint(ltn[8], 320, y, "[n/a]");
        
		y += 20;        
        intraFontPrintf(ltn[8], 10, y, "Symbols: ");
        unsigned short ucs2_arib[6]    = { 57786, 57787, 57788, 57789, 57790, 0 };
        x = intraFontPrintUCS2(arib, 80, y, ucs2_arib);
		if (x == 80) intraFontPrint(ltn[8], 80, y, "[n/a]");
        
        y += 25;
		intraFontPrint(ltn[8], 10, y, "Colors: ");
        intraFontSetStyle(ltn[8], 1.0f,RED,BLUE,0);
		x = intraFontPrint(ltn[8], 80, y, "colorful, ");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,0,0);
        x = intraFontPrint(ltn[8], x, y, "no shadow, ");
        intraFontSetStyle(ltn[8], 1.0f,0,BLACK,0);
        x = intraFontPrint(ltn[8], x, y, "no text, ");
        intraFontSetStyle(ltn[8], 1.0f,0x7FFFFFFF,BLACK,0);
        x = intraFontPrint(ltn[8], x, y, "transparent, ");		
        intraFontSetStyle(ltn[8], 1.0f,GRAY,WHITE,0);
        x = intraFontPrint(ltn[8], x, y, "glowing, ");
		float t = ((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
		int val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
		intraFontSetStyle(ltn[8], 1.0f,LITEGRAY,(0xFF<<24)+(val<<16)+(val<<8)+(val),0);
		x = intraFontPrint(ltn[8], x, y, "flashing");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);
        
        y += 20;
        intraFontPrint(ltn[8], 10, y, "Spacing: ");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_WIDTH_FIX);
		x = intraFontPrint(ltn[8], 80, y, "fixed (default), ");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_WIDTH_FIX | 12);
        x = intraFontPrint(ltn[8], x, y, "fixed (12), ");		
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);
        x = intraFontPrint(ltn[8], x, y, "variable width");
        
        y += 30;
        intraFontPrint(ltn[8], 10, y, "Scaling: ");
		intraFontSetStyle(ltn[0], 0.5f,WHITE,BLACK,0);
		x = intraFontPrint(ltn[0], 80, y, "tiny, ");
        intraFontSetStyle(ltn[0], 0.75f,WHITE,BLACK,0);
		x = intraFontPrint(ltn[0], x, y, "small, ");
        intraFontSetStyle(ltn[0], 1.0f,WHITE,BLACK,0);
		x = intraFontPrint(ltn[0], x, y, "regular, ");
        intraFontSetStyle(ltn[0], 1.25f,WHITE,BLACK,0);
		x = intraFontPrint(ltn[0], x, y, "large, ");
        intraFontSetStyle(ltn[0], 1.5f,WHITE,BLACK,0);
		x = intraFontPrint(ltn[0], x, y, "huge"); 
		intraFontSetStyle(ltn[0], 1.0f,WHITE,BLACK,0);
		
		y += 20;
        intraFontPrint(ltn[8], 10, y, "Align: ");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_LEFT);
		intraFontPrint(ltn[8], 80, y, "left");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn[8], (80+470)/2, y, "center");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_RIGHT);
        intraFontPrint(ltn[8], 470, y, "right");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);

		// End drawing
		sceGuFinish();
		sceGuSync(0,0);
		
		// Swap buffers (waiting for vsync)
		sceDisplayWaitVblankStart();
		flipScreen();
	}
    
    //create screenshot (optional)
    //saveImage("scrshot.png", getVramDisplayBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE, 0);

	// Unload our fonts
    for (i = 0; i < 16; i++) {
        intraFontUnload(ltn[i]);
    }
    intraFontUnload(jpn0);
	intraFontUnload(kr0);
    intraFontUnload(arib);
	
	// Shutdown font library
	intraFontShutdown();

	sceKernelExitGame();

	return 0;
}
