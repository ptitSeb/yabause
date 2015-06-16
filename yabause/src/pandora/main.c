#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <SDL.h>
#ifdef HAVE_LIBGL
#include <SDL_opengl.h>
#endif

#include "../yabause.h"
#include "../yui.h"
#include "../peripheral.h"
#include "../sh2core.h"
#include "../sh2int.h"
#include "../vidogl.h"
#include "../vidsoft.h"
#include "../cs0.h"
#include "../cs2.h"
#include "../cdbase.h"
#include "../scsp.h"
#include "../sndsdl.h"
#include "../sndal.h"
#include "../persdljoy.h"
#ifdef ARCH_IS_LINUX
#include "../perlinuxjoy.h"
#endif
#include "../debug.h"
#include "../m68kcore.h"
#include "../m68kc68k.h"


#include "main.h"

#define DEFAUL_BIOS "sega_101.bin"

static char biospath[256] = "\0";
static char cdpath[256] = "\0";
static char buppath[256] = "\0";
static char mpegpath[256] = "\0";
static char cartpath[256] = "\0";

M68K_struct * M68KCoreList[] = {
&M68KDummy,
#ifdef HAVE_C68K
&M68KC68K,
#endif
#ifdef HAVE_Q68
&M68KQ68,
#endif
NULL
};

SH2Interface_struct *SH2CoreList[] = {
&SH2Interpreter,
&SH2DebugInterpreter,
#ifdef SH2_DYNAREC
&SH2Dynarec,
#endif
NULL
};

PerInterface_struct *PERCoreList[] = {
&PERDummy,
#ifdef HAVE_LIBSDL
&PERSDLJoy,
#endif
#ifdef ARCH_IS_LINUX
&PERLinuxJoy,
#endif
NULL
};

CDInterface *CDCoreList[] = {
&DummyCD,
&ISOCD,
#ifndef UNKNOWN_ARCH
&ArchCD,
#endif
NULL
};

SoundInterface_struct *SNDCoreList[] = {
&SNDDummy,
#ifdef HAVE_LIBSDL
&SNDSDL,
#endif
#ifdef HAVE_LIBAL
&SNDAL,
#endif
NULL
};

VideoInterface_struct *VIDCoreList[] = {
&VIDDummy,
#ifdef HAVE_LIBGL
&VIDOGL,
#endif
&VIDSoft,
NULL
};

#ifdef YAB_PORT_OSD
OSD_struct *OSDCoreList[] = {
&OSDDummy,
#ifdef HAVE_LIBGLUT
&OSDGlut,
#endif
&OSDSoft,
NULL
};
#endif

int YuiInit(int sh2core)   {
    static yabauseinit_struct yinit;
    static void * padbits;

    yinit.percoretype = PERCORE_DUMMY;
    yinit.sh2coretype = sh2core;
    #ifdef HAVE_LIBGL
    yinit.vidcoretype = VIDCORE_OGL;
    #else
    yinit.vidcoretype = VIDCORE_SOFT;
    #endif
    yinit.m68kcoretype = M68KCORE_C68K;
    yinit.sndcoretype = 1;
    yinit.cdcoretype = CDCORE_ISO;
    yinit.carttype = CART_NONE;
    yinit.regionid = REGION_AUTODETECT;
    yinit.biospath = biospath[0] ? biospath : NULL;
    yinit.cdpath = cdpath[0]? cdpath : NULL;
    yinit.buppath = buppath[0]? buppath : NULL;
    yinit.mpegpath = mpegpath[0]? mpegpath : NULL;
    yinit.cartpath = cartpath[0]? cartpath : NULL;
    yinit.frameskip = 1;
    yinit.videoformattype = VIDEOFORMATTYPE_NTSC;
    yinit.clocksync = 1;
    yinit.basetime = 0;

#ifdef HAVE_LIBGL
    printf("Setup GL Context\n");
    SDL_SetVideoMode(800, 480, 32, SDL_OPENGL | SDL_FULLSCREEN);
#endif

    if(YabauseInit(&yinit) != 0)
      return -1;

    PerPortReset();
    padbits = PerPadAdd(&PORTDATA1);
    PerSetKey(SDLK_UP, PERPAD_UP, padbits);
    PerSetKey(SDLK_RIGHT, PERPAD_RIGHT, padbits);
    PerSetKey(SDLK_DOWN, PERPAD_DOWN, padbits);
    PerSetKey(SDLK_LEFT, PERPAD_LEFT, padbits);
    PerSetKey(SDLK_LALT, PERPAD_START, padbits);
    PerSetKey(SDLK_PAGEDOWN, PERPAD_A, padbits);	//(X)
    PerSetKey(SDLK_END, PERPAD_B, padbits);			//(B)
    PerSetKey(SDLK_BACKSPACE, PERPAD_C, padbits);	// <-
    PerSetKey(SDLK_HOME, PERPAD_X, padbits);		//(A)
    PerSetKey(SDLK_PAGEUP, PERPAD_Y, padbits);		//(Y)
    PerSetKey(SDLK_0, PERPAD_Z, padbits);			// 0
    PerSetKey(SDLK_RSHIFT, PERPAD_RIGHT_TRIGGER, padbits);	//(R)
    PerSetKey(SDLK_RCTRL, PERPAD_LEFT_TRIGGER, padbits);	//(L)

#ifdef HAVE_LIBGL
    glViewport(0, 0, 800, 480);
    VIDCore->Resize(800, 480, 0 );
#endif

    int quit = 0;
    SDL_Event event;

    while(!quit) {
	    while(SDL_PollEvent(&event)){
	    	switch(event.type){
	    		case SDL_KEYDOWN:
	    			PerKeyDown(event.key.keysym.sym);
	    			break;
	    		case SDL_KEYUP:
	    			if (event.key.keysym.sym == SDLK_ESCAPE)
	    				quit = 1;
	    			else
	    				PerKeyUp(event.key.keysym.sym);
	    			break;
	    		case SDL_QUIT:
	    			quit = 1;
	    			break;
	    	}
	    }
        PERCore->HandleEvents();
    }

    return 0;
}

void YuiErrorMsg(const char *error_text)    {
    fprintf(stderr, "Error: %s\n", error_text);
    exit(-1);
}

void YuiSwapBuffers(void)   {
    // Show the screen !
    static int old_width = 0;
    static int old_height = 0;
    int buf_width, buf_height;

    VIDCore->GetGlSize(&buf_width, &buf_height);
#ifdef HAVE_LIBGL
// OpenGL part
    SDL_GL_SwapBuffers();
#else	//HAVE_LIBGL
// Soft part
    static SDL_Surface *screen = NULL;
	static SDL_Surface *pixdata = NULL; 

    if ((old_width!=buf_width) || (old_height!=buf_height)) {
    	// set new mode
    	screen = SDL_SetVideoMode(buf_width, buf_height, 0, SDL_HWSURFACE);	// create a window
    	printf("switch screen from %dx%d to %dx%d\n", old_width, old_height, buf_width, buf_height);
    	old_width = buf_width;
    	old_height = buf_height;

	    #ifdef USE_RGB_565
		const SDL_PixelFormat pixformat = {
				.palette = NULL,
				.BitsPerPixel = 16, .BytesPerPixel = 2,
				.Rmask = 0x1f, .Gmask = 0x7e0, .Bmask = 0xf800, .Amask = 0,
				.Rloss = 3, .Gloss = 2, .Bloss = 3, .Aloss = 0,
				.Rshift = 11, .Gshift = 5, .Bshift = 0, .Ashift = 0,
				.colorkey = 0, .alpha = 0
		};
		#else
		const SDL_PixelFormat pixformat = {
				.palette = NULL,
				.BitsPerPixel = 32, .BytesPerPixel = 4,
				.Rmask = 0x0000ff00, .Gmask = 0x00ff0000, .Bmask = 0xff000000, .Amask = 0x000000ff,
				.Rloss = 0, .Gloss = 0, .Bloss = 0, .Aloss = 0,
				.Rshift = 24, .Gshift = 16, .Bshift = 8, .Ashift = 0,
				.colorkey = 0, .alpha = 0
		};
		#endif
		if (pixdata) SDL_FreeSurface(pixdata);

		pixdata = SDL_CreateRGBSurfaceFrom(dispbuffer, buf_width, buf_height, 
			pixformat.BitsPerPixel, buf_width*pixformat.BytesPerPixel,
			pixformat.Rmask, pixformat.Gmask, pixformat.Bmask, pixformat.Amask
		);
    }
    if (!screen)
    	return;

	if (!dispbuffer) {
		printf("Warning, NULL dispbuffer\n");
		return;
	}

	SDL_BlitSurface(pixdata, NULL, screen, NULL);
	// update the screen buffer
	SDL_Flip(screen);


#endif //HAVE_LIBGL
}

// utility fonction
int file_exists(const char * filename)
{
    if (access(filename, F_OK)==0)
    {
        return 1;
    }
    return 0;
}

// Setup
void set_biospath()
{
	if (biospath[0])
		return;
	if (file_exists(DEFAUL_BIOS)) {
		printf("bios file found\n");
		strcpy(biospath, DEFAUL_BIOS);
	} else {
		printf("bios not found (" DEFAUL_BIOS "), using bios emulation\n");
	}
}

void print_help()
{

	printf("command line arguments\n-------------\n\n");

	printf("-b|--bios filename\t: load filename as bios (" DEFAUL_BIOS " tried by default)\n");
	printf("-i|--image filename\t: load filename as CD image\n");
	printf("-h|--help\t\t: print this help\n");
	printf("\n");

	exit(1);
}

int main(int argc, char *argv[])    {
    int core;
    int parse_status;
#ifdef SH2_DYNAREC
    core = 2;
#else
	core = SH2CORE_DEFAULT;
#endif
	printf("Yabause for Pandora\n===================\n\n");

    // parse command lines arguments
    parse_status = 0;	
    // hack to allow just the image...
    if ((argc==2) && (argv[1][0]!='-'))
    	parse_status = 2;

    for (int i = 1; i < argc; i++) {
   		switch (parse_status) {
   			case 0: printf("Error on command line (\"%s\")\n", argv[i]);
		    	if (!strcmp(argv[i], "-h")) 
		    		print_help();
		    	else if(!strcmp(argv[i], "--help"))
		    		print_help();
		    	else if (!strcmp(argv[i], "-i"))
		    		parse_status = 2;
		    	else if(!strcmp(argv[i], "--image"))
		    		parse_status = 2;
		    	else if (!strcmp(argv[i], "-b"))
		    		parse_status = 1;
		    	else if(!strcmp(argv[i], "--bios"))
		    		parse_status = 1;
				break;
			case 1: printf("Setting Bios to \"%s\"\n", argv[i]);
				if (file_exists(argv[i]))
					strcpy(biospath, argv[i]);
				else
					printf("Bios File not found");
				parse_status = 0;
				break;
			case 2: printf("Setting Disc Image to \"%s\"\n", argv[i]);
				strcpy(cdpath, argv[i]);
				parse_status = 0;
				break;
    	}
    }
    // incase no bios has been set using command line
    set_biospath();


    // Init SDL
    if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO)) {
    	printf("Error initializing SDL...\n");
    	exit(-1);
    }

    printf("Starting emulation...\n");

    YuiInit(core);

    printf("\n*****************\nFinished\n");
    SDL_Quit();

    return 0;
}
