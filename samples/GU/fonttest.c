/*
 * fonttest.c
 * This file is used to display the PSP's internal font (pgf firmware files)
 * Written in 2007 by BenHur
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

#include "../../intraFont.h"

PSP_MODULE_INFO("intraFontTest", PSP_MODULE_USER, 1, 1);

static int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

int exit_callback(int arg1, int arg2, void *common)
{
	running = 0;
	return 0;
}

int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("CallbackThread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);

	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

int main()
{
	SetupCallbacks();
	
	sceGuInit();
	sceGuStart(GU_DIRECT, list);

	sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
	sceGuDepthBuffer((void*)0x110000, BUF_WIDTH);
 
	sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuDepthRange(65535, 0);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0,0);
 
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	// Colors
	enum colors {
		RED =	0xFF0000FF,
		GREEN =	0xFF00FF00,
		BLUE =	0xFFFF0000,
		WHITE =	0xFFFFFFFF,
		GRAY =  0xFF7F7F7F,
		DARKGRAY = 0xFF3F3F3F,
		BLACK = 0xFF000000
	};

	intraFontInit();

	// Load fonts
	intraFont* ltn0 = intraFontLoad("flash0:/font/ltn0.pgf"); //latin sans-serif regular
    intraFont* ltn1 = intraFontLoad("flash0:/font/ltn1.pgf"); //latin with serif regular
    intraFont* ltn2 = intraFontLoad("flash0:/font/ltn2.pgf"); //latin sans-serif italic
    intraFont* ltn3 = intraFontLoad("flash0:/font/ltn3.pgf"); //latin with serif italic
    intraFont* ltn4 = intraFontLoad("flash0:/font/ltn4.pgf"); //latin sans-serif bold
    intraFont* ltn5 = intraFontLoad("flash0:/font/ltn5.pgf"); //latin with serif bold
    intraFont* ltn6 = intraFontLoad("flash0:/font/ltn6.pgf"); //latin sans-serif italic & bold
    intraFont* ltn7 = intraFontLoad("flash0:/font/ltn7.pgf"); //latin sans-serif italic & bold
    intraFont* ltn8 = intraFontLoad("flash0:/font/ltn8.pgf"); //same as above with small font
    intraFont* ltn9 = intraFontLoad("flash0:/font/ltn9.pgf"); //...
    intraFont* ltn10 = intraFontLoad("flash0:/font/ltn10.pgf");
    intraFont* ltn11 = intraFontLoad("flash0:/font/ltn11.pgf");
    intraFont* ltn12 = intraFontLoad("flash0:/font/ltn12.pgf");
    intraFont* ltn13 = intraFontLoad("flash0:/font/ltn13.pgf");
    intraFont* ltn14 = intraFontLoad("flash0:/font/ltn14.pgf");
    intraFont* ltn15 = intraFontLoad("flash0:/font/ltn15.pgf");
    intraFont* jpn0 = intraFontLoad("flash0:/font/jpn0.pgf"); //japanese font (and parts of latin)

	// Make sure the fonts are loaded
	if(!ltn0 || !ltn1 || !ltn2 || !ltn3 || !ltn4 || !ltn5 || !ltn6 || !ltn7 || !ltn8 || 
       !ltn9 || !ltn10 || !ltn11 || !ltn12 || !ltn13 || !ltn14 || !ltn15 || !jpn0) 
		sceKernelExitGame();
		
	while(running) 	{
		sceGuStart(GU_DIRECT, list);

		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective( 75.0f, 16.0f/9.0f, 0.5f, 1000.0f);
 
        sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();

		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		
		sceGuClearColor(GRAY);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);


        // Draw various text
        int x,y=25;
        x = intraFontPrint(ltn0, 10, y, "Latin Sans-Serif: ");
		x = intraFontPrint(ltn0, x, y, "regular, ");
        x = intraFontPrint(ltn2, x, y, "italic, ");
        x = intraFontPrint(ltn4, x, y, "bold, ");
        x = intraFontPrint(ltn6, x, y, "both");
        
        y += 25;
        x = intraFontPrint(ltn0, 10, y, "Latin Sans-Serif small: ");
		x = intraFontPrint(ltn8, x, y, "regular, ");
        x = intraFontPrint(ltn10, x, y, "italic, ");
        x = intraFontPrint(ltn12, x, y, "bold, ");
        x = intraFontPrint(ltn14, x, y, "both");
        
        y += 25;
        x = intraFontPrint(ltn0, 10, y, "Latin with Serif: ");
		x = intraFontPrint(ltn1, x, y, "regular, ");
        x = intraFontPrint(ltn3, x, y, "italic, ");
        x = intraFontPrint(ltn5, x, y, "bold, ");
        x = intraFontPrint(ltn7, x, y, "both");
        
        y += 25;
        x = intraFontPrint(ltn0, 10, y, "Latin with Serif small: ");
		x = intraFontPrint(ltn9, x, y, "regular, ");
        x = intraFontPrint(ltn11, x, y, "italic, ");
        x = intraFontPrint(ltn13, x, y, "bold, ");
        x = intraFontPrint(ltn15, x, y, "both");
        
        y += 25;
        x = intraFontPrint(ltn0, 10, y, "Japanese: ");
        unsigned short ucs2_text[8]    = { 0x3053, 0x3093, 0x306b, 0x3061, 0x306f, 0x4e16, 0x754c, 0 };
        intraFontPrintUCS2(jpn0, x, y, ucs2_text);
        
        y += 25;
		x = intraFontPrint(ltn0, 10, y, "Colors: ");
        intraFontSetStyle(ltn0, 1.0f,RED,BLUE,0);
		x = intraFontPrint(ltn0, 100, y, "colorful, ");
        intraFontSetStyle(ltn0, 1.0f,0x7FFFFFFF,BLACK,0);
        x = intraFontPrint(ltn0, x, y, "semitransparent, ");
        y += 25;
        intraFontSetStyle(ltn0, 1.0f,WHITE,0,0);
        x = intraFontPrint(ltn0, 100, y, "no shadow, ");
        intraFontSetStyle(ltn0, 1.0f,0,BLACK,0);
        x = intraFontPrint(ltn0, x, y, "just shadow, ");
        intraFontSetStyle(ltn0, 1.0f,GRAY,WHITE,0);
        x = intraFontPrint(ltn0, x, y, "glowing");
        intraFontSetStyle(ltn0, 1.0f,WHITE,BLACK,0);
        
        y += 25;
        x = intraFontPrint(ltn0, 10, y, "Spacing: ");
		intraFontSetStyle(ltn0, 1.0f,WHITE,BLACK,INTRAFONT_FIXEDWIDTH);
		x = intraFontPrint(ltn0, 100, y, "fixed, ");
        intraFontSetStyle(ltn0, 1.0f,WHITE,BLACK,0);
        x = intraFontPrint(ltn0, x, y, "variable width");
        
        y += 35;
        x = intraFontPrint(ltn0, 10, y, "Scaling: ");
		intraFontSetStyle(ltn0, 0.5f,WHITE,BLACK,0);
		x = intraFontPrint(ltn0, 100, y, "tiny, ");
        intraFontSetStyle(ltn0, 0.75f,WHITE,BLACK,0);
		x = intraFontPrint(ltn0, x, y, "small, ");
        intraFontSetStyle(ltn0, 1.0f,WHITE,BLACK,0);
		x = intraFontPrint(ltn0, x, y, "regular, ");
        intraFontSetStyle(ltn0, 1.25f,WHITE,BLACK,0);
		x = intraFontPrint(ltn0, x, y, "large, ");
        intraFontSetStyle(ltn0, 1.5f,WHITE,BLACK,0);
		x = intraFontPrint(ltn0, x, y, "huge");
        intraFontSetStyle(ltn0, 1.0f,WHITE,BLACK,0);

        
        // End drawing
		sceGuFinish();
		sceGuSync(0,0);
		
		// Swap buffers (waiting for vsync)
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}
	
	// Unload our fonts
	intraFontUnload(ltn0);
    intraFontUnload(ltn1);
    intraFontUnload(ltn2);    
    intraFontUnload(ltn3);
    intraFontUnload(ltn4);
    intraFontUnload(ltn5);
    intraFontUnload(ltn6);    
    intraFontUnload(ltn7);
    intraFontUnload(ltn8);
    intraFontUnload(ltn9);
    intraFontUnload(ltn10);    
    intraFontUnload(ltn11);
    intraFontUnload(ltn12);
    intraFontUnload(ltn13);
    intraFontUnload(ltn14);    
    intraFontUnload(ltn15);
    intraFontUnload(jpn0);

	// Shutdown font library
	intraFontShutdown();

	sceKernelExitGame();

	return 0;
}
