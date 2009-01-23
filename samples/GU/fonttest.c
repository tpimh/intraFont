/*
 * fonttest.c
 * This file is used to display the PSP's internal font (pgf firmware files)
 * intraFont Version 0.26 by BenHur - http://www.psp-programming.com/benhur
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

#include "../../intraFont.h"

PSP_MODULE_INFO("intraFontTest", PSP_MODULE_USER, 1, 1);

static int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

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

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

int main() {
    pspDebugScreenInit();
	SetupCallbacks();

	// Colors
	enum colors {
		RED =	0xFF0000FF,
		GREEN =	0xFF00FF00,
		BLUE =	0xFFFF0000,
		WHITE =	0xFFFFFFFF,
		LITEGRAY = 0xFFBFBFBF,
		GRAY =  0xFF7F7F7F,
		DARKGRAY = 0xFF3F3F3F,		
		BLACK = 0xFF000000,
	};
    
    pspDebugScreenPrintf("intraFont 0.26 - 2009 by BenHur\n\nLoading fonts: 0%%");
        
    // Init intraFont library
    intraFontInit();
    
     // Load fonts
    intraFont* ltn[16];                                         //latin fonts (large/small, with/without serif, regular/italic/bold/italic&bold)
    char file[40];
    int i;
    for (i = 0; i < 16; i++) {
        sprintf(file, "flash0:/font/ltn%d.pgf", i); 
        ltn[i] = intraFontLoad(file,0);                                             //<- this is where the actual loading happens 
		pspDebugScreenSetXY(15,2);
        pspDebugScreenPrintf("%d%%",(i+1)*100/19);
    }

    intraFont* jpn0 = intraFontLoad("flash0:/font/jpn0.pgf",INTRAFONT_STRING_SJIS); //japanese font with SJIS text string encoding
	intraFontSetStyle(jpn0, 0.8f, WHITE, BLACK, 0);                                 //scale to 80%
    pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("%d%%",17*100/19);
        
	intraFont* kr0 = intraFontLoad("flash0:/font/kr0.pgf", INTRAFONT_STRING_UTF8);  //Korean font (not available on all systems) with UTF-8 encoding
	intraFontSetStyle(kr0, 0.8f, WHITE, BLACK, 0);                                  //scale to 80%
	pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("%d%%",18*100/19);
           
	intraFont* arib = intraFontLoad("flash0:/font/arib.pgf",0);                     //Symbols (not available on all systems)
	pspDebugScreenSetXY(15,2);
    pspDebugScreenPrintf("done\n");

	// Make sure the important fonts for this application are loaded
	if(!ltn[0] || !ltn[4] || !ltn[8]) sceKernelExitGame();

	
	// Init GU    
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

	float x_scroll1 = 80, x_scroll2 = 225, x_scroll3 = 370,  x_scroll4 = 390;
	
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
        float x,y = 20;
		intraFontSetStyle(ltn[4], 1.0f,BLACK,WHITE,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn[4], 240, y, "intraFont 0.26 - 2009 by BenHur");
        intraFontSetStyle(ltn[4], 1.0f,WHITE,BLACK,0);
		        
		y += 21;
		intraFontPrint(ltn[8],  10, y, "Latin Sans-Serif:");
		intraFontPrint(ltn[0], 180, y, "regular,");
        intraFontPrint(ltn[2], 270, y, "italic,");
        intraFontPrint(ltn[4], 330, y, "bold,");
        intraFontPrint(ltn[6], 390, y, "both");
        
        y += 18;
        intraFontPrint(ltn[8],  10, y, "Latin Sans-Serif small:");
		intraFontPrint(ltn[8], 180, y, "regular,");
        intraFontPrint(ltn[10], 270, y, "italic,");
        intraFontPrint(ltn[12], 330, y, "bold,");
        intraFontPrint(ltn[14], 390, y, "both");
        
        y += 18;
        intraFontPrint(ltn[8],  10, y, "Latin with Serif:");
		intraFontPrint(ltn[1], 180, y, "regular,");
        intraFontPrint(ltn[3], 270, y, "italic,");
        intraFontPrint(ltn[5], 330, y, "bold,");
        intraFontPrint(ltn[7], 390, y, "both");
        
        y += 18;
        intraFontPrint(ltn[8],  10, y, "Latin with Serif small:");
		intraFontPrint(ltn[9], 180, y, "regular,");
        intraFontPrint(ltn[11], 270, y, "italic,");
        intraFontPrint(ltn[13], 330, y, "bold,");
        intraFontPrint(ltn[15], 390, y, "both");
		
        y += 18;
		intraFontSetEncoding(ltn[8], INTRAFONT_STRING_UTF8);   //set text string encoding to UTF-8
		intraFontPrint(ltn[8], 10, y, "LTN (UTF-8):");         //(has no effect on std ascii)
		intraFontPrint(ltn[8], 110, y, "\xC3\xA5 \xC3\xA8 \xC3\xAD \xC3\xB4 \xC3\xBC \xC3\xB1"); //UTF-8 encoded chars with accents on top of them
		intraFontSetEncoding(ltn[8], INTRAFONT_STRING_CP437);  //switch to codepage 437 encoding for extended ascii
		intraFontPrint(ltn[8], 250, y, "LTN (CP437):");        //(has no effect on std ascii)
		intraFontPrint(ltn[8], 350, y, "\x86 \x8A \xA1 \x93 \x81 \xA4"); //the same chars as above using codepage 437 encoding
		intraFontSetEncoding(ltn[8], INTRAFONT_STRING_ASCII);  //set encoding back to ASCII (default)

		y += 18;
        intraFontPrint(ltn[8], 10, y, "JPN (UTF-8):");
        char utf8_jpn[] = {0xE3, 0x81, 0x93, 0xE3, 0x82, 0x93, 0xE3, 0x81, 0xAB, 0xE3, 0x81, 0xA1, 0xE3, 0x81, 0xAF, 0x20, 0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C, 0};
		intraFontSetEncoding(jpn0, INTRAFONT_STRING_UTF8);    //temporarely switch to UTF-8 (INTRAFONT_STRING_SJIS was set in intraFontLoad call)
        x = intraFontPrint(jpn0, 110, y, utf8_jpn);           //print UTF-8 encoded string
		if (x == 110) intraFontPrint(ltn[8], 110, y, "[n/a]");
		intraFontSetEncoding(jpn0, INTRAFONT_STRING_SJIS);    //switch back to S-JIS
        
		intraFontPrint(ltn[8], 250, y, "JPN (S-JIS):");
        x = intraFontPrint(jpn0, 350, y, "イントラフォント"); //S-JIS encoded text string (flag INTRAFONT_STRING_SJIS set in intraFontLoad call)
		if (x == 350) intraFontPrint(ltn[8], 350, y, "[n/a]");

		y += 18;   
		intraFontPrint(ltn[8], 10, y, "KOR (UTF-8):");
		char utf8_kr[] = {0xed, 0x99, 0x98, 0xec, 0x98, 0x81, 0x20, 0xeb, 0x8c, 0x80, 0xed, 0x95, 0x9c, 0xeb, 0xaf, 0xbc, 0xea, 0xb5, 0xad, 0};
        x = intraFontPrint(kr0, 110, y, utf8_kr);             //print UTF-8 string (flag INTRAFONT_STRING_UTF8 set in intraFontLoad call)
		if (x == 110) intraFontPrint(ltn[8], 110, y, "[n/a]");
		
        intraFontPrint(ltn[8], 250, y, "Symbols: ");
        unsigned short ucs2_arib[] = { 57786, 57787, 57788, 57789, 57790, 0 };
        x = intraFontPrintUCS2(arib, 350, y, ucs2_arib);
		if (x == 350) intraFontPrint(ltn[8], 350, y, "[n/a]");
        
        y += 18;
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
        
        y += 18;
        intraFontPrint(ltn[8], 10, y, "Spacing: ");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_WIDTH_FIX);
		x = intraFontPrint(ltn[8], 80, y, "fixed (default), ");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_WIDTH_FIX | 12);
        x = intraFontPrint(ltn[8], x, y, "fixed (12), ");		
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);
        x = intraFontPrint(ltn[8], x, y, "variable width");
        
        y += 22;
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
		
		y += 18;
        intraFontPrint(ltn[8], 10, y, "Align: ");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_LEFT);
		t = ((float)(clock() % (CLOCKS_PER_SEC*10))) / ((float)CLOCKS_PER_SEC);
		int length = (t < 5.0f) ? t*7.1f : (10.0f-t)*7.1f;
		intraFontPrintColumnEx(ltn[8],  80, y,  90, "left aligned with auto linebreaks  ", length);
		//NB: intraFontPrintColumnEx() is used to print a sub-string of a given length (last parameter)
		//    if you want to print the whole string, simply use intraFontPrintColumn() and omit the length parameter
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_CENTER);
		intraFontPrintColumnEx(ltn[8], 225, y, 110, "center aligned with auto linebreaks", length);
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_RIGHT);
        intraFontPrintColumnEx(ltn[8], 370, y,  90, "right aligned with auto linebreaks ", length);
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_ALIGN_FULL);
        intraFontPrintColumnEx(ltn[8], 390, y,  80, "full justified with auto linebreaks", length);
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);

		y += 40;
        intraFontPrint(ltn[8], 10, y, "Scrolling: ");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_SCROLL_LEFT);
		x_scroll1 = intraFontPrintColumn(ltn[8], x_scroll1, y, 80, "This text is scrolled to the left.");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_SCROLL_SEESAW);
        x_scroll2 = intraFontPrintColumn(ltn[8], x_scroll2, y, 90, "Back & forth like a seesaw.");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_SCROLL_RIGHT);
		x_scroll3 = intraFontPrintColumn(ltn[8], x_scroll3, y, 80, "Scrolling to the right...");
        intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,INTRAFONT_SCROLL_THROUGH);
        x_scroll4 = intraFontPrintColumn(ltn[8], x_scroll4, y, 80, "This text is scrolled through.");
		intraFontSetStyle(ltn[8], 1.0f,WHITE,BLACK,0);

        
        // End drawing
		sceGuFinish();
		sceGuSync(0,0);
		
		// Swap buffers (waiting for vsync)
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}
	
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
