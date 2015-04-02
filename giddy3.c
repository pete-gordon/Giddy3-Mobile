
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <zlib.h>

#include <GLES/gl.h>
#include "SDL.h"
#ifndef __android__
#include "PDL.h"
#else
#include "android.h"
#endif

#include "giddy3.h"
#include "render.h"
#include "samples.h"
#include "titles.h"


#ifdef __android__
BOOL use_alt_basepath;
#endif

extern struct thingy n_things1[], g_things1[], p_things1[];
extern struct thingy n_things2[], g_things2[], p_things2[];
extern struct thingy n_things3[], g_things3[], p_things3[];
extern struct thingy n_things4[], g_things4[], p_things4[];
extern struct thingy n_things5[], g_things5[], p_things5[];
extern struct lift lifts1[], lifts2[], lifts3[], lifts4[], lifts5[];
extern int endingstate;

extern int rotated, vieww;
int touchx[2], touchy[2], touchscalex, touchscaley;
int touchjoy = -1, touchjoyx = 0, touchjump = -1;

extern time_t gamestarttime;
extern double extratime;
BOOL needquit = FALSE, audioavailable = FALSE;

BOOL icademode=FALSE;
BOOL spacehit=FALSE, enterhit=FALSE;
BOOL gleft=FALSE, gright=FALSE, gjump=FALSE;
BOOL gamepause;

extern int sfxvol, musicvol, sfxvolopt, musicvolopt;

SDL_Surface *ssrf;

int mousx, mousy;

extern struct what_is_giddy_doing gid;
extern int fgx, fgy, titlestate;

int what_are_we_doing = WAWD_TITLES;

BOOL musicon = TRUE;

extern Uint8 strig[ST_LAST], inv[INV_LAST];
extern struct what_is_giddy_doing gid;
extern struct what_is_everyone_else_doing stuff;
extern int clevel, fgx, fgy, bgx, bgy;

static int savegame_size = 0;
Uint8 *savegamebuf = NULL;
SDL_bool savegame_available = SDL_FALSE;

static int calc_thingy_active_space( struct thingy *t )
{
	int i, sz;

	sz = 0;
	for( i=0; t[i].ix!=-1; i++ )
		sz += sizeof(BOOL);

	return sz;
}

static int calc_lift_saveable_space( struct lift *l )
{
	int i, sz;

	sz = 0;
	for( i=0; l[i].hdr.numstops!=-1; i++ )
		sz += sizeof(struct liftsaveable);

	return sz;
}

static void copy_thingy_active(int *offset, struct thingy *t )
{
	int i;

	for( i=0; t[i].ix!=-1; i++ )
	{
		memcpy(&savegamebuf[*offset], &t[i].active, sizeof(BOOL));
		(*offset) += sizeof(BOOL);
	}
}

static void copy_lift_saveable(int *offset, struct lift *l )
{
	int i;

	for( i=0; l[i].hdr.numstops!=-1; i++ )
	{
		memcpy(&savegamebuf[*offset], &l[i].hdr, sizeof(struct liftsaveable));
		(*offset) += sizeof(struct liftsaveable);
	}
}

static void restore_thingy_active(int *offset, struct thingy *t )
{
	int i;

	for( i=0; t[i].ix!=-1; i++ )
	{
		memcpy(&t[i].active, &savegamebuf[*offset], sizeof(BOOL));
		(*offset) += sizeof(BOOL);
	}
}

static void restore_lift_saveable(int *offset, struct lift *l )
{
	int i;

	for( i=0; l[i].hdr.numstops!=-1; i++ )
	{
		memcpy(&l[i].hdr, &savegamebuf[*offset], sizeof(struct liftsaveable));
		(*offset) += sizeof(struct liftsaveable);
	}
}

void destroy_save_game( void )
{
#ifdef __android__
	if (use_alt_basepath)
		unlink(BASEPATH_ALT"hats/extra.bin");
	else
#endif
		unlink(BASEPATH"hats/extra.bin");

	savegame_available = SDL_FALSE;
}

void save_game( void )
{

	gzFile f = NULL;
	int i=0, chksum;
	time_t timenow;
	double timesofar;

	if( !savegamebuf ) return;

	if( what_are_we_doing != WAWD_GAME ) return;

	if (gid.spoonedit)
		destroy_save_game();

	time( &timenow );
	timesofar = difftime( timenow, gamestarttime ) + extratime;

	memset(savegamebuf, 0, savegame_size);

	memcpy(&savegamebuf[i], &clevel,    sizeof(clevel)); i += sizeof(clevel);
	memcpy(&savegamebuf[i], &timesofar, sizeof(timesofar)); i += sizeof(timesofar);
	memcpy(&savegamebuf[i], &gid,       sizeof(gid)   ); i += sizeof(gid);
	memcpy(&savegamebuf[i], &stuff,     sizeof(stuff) ); i += sizeof(stuff);
	memcpy(&savegamebuf[i], strig,      ST_LAST       ); i += ST_LAST;
	memcpy(&savegamebuf[i], inv,        INV_LAST      ); i += INV_LAST;
	memcpy(&savegamebuf[i], &fgx,       sizeof(fgx)   ); i += sizeof(fgx);
	memcpy(&savegamebuf[i], &fgy,       sizeof(fgy)   ); i += sizeof(fgy);
	memcpy(&savegamebuf[i], &bgx,       sizeof(bgx)   ); i += sizeof(bgx);
	memcpy(&savegamebuf[i], &bgy,       sizeof(bgy)   ); i += sizeof(bgy);

	copy_thingy_active(&i, n_things1);
	copy_thingy_active(&i, p_things1);
	copy_thingy_active(&i, g_things1);
	copy_thingy_active(&i, n_things2);
	copy_thingy_active(&i, p_things2);
	copy_thingy_active(&i, g_things2);
	copy_thingy_active(&i, n_things3);
	copy_thingy_active(&i, p_things3);
	copy_thingy_active(&i, g_things3);
	copy_thingy_active(&i, n_things4);
	copy_thingy_active(&i, p_things4);
	copy_thingy_active(&i, g_things4);
	copy_thingy_active(&i, n_things5);
	copy_thingy_active(&i, p_things5);
	copy_thingy_active(&i, g_things5);

	copy_lift_saveable(&i, lifts1);
	copy_lift_saveable(&i, lifts2);
	copy_lift_saveable(&i, lifts3);
	copy_lift_saveable(&i, lifts4);
	copy_lift_saveable(&i, lifts5);

	chksum = 0x53492873;
	for( i=0; i<savegame_size; i++ )
		chksum += savegamebuf[i];

#ifdef __android__
	if (use_alt_basepath)
		f = gzopen( BASEPATH_ALT"hats/extra.bin", "wb" );
	else
#endif
		f = gzopen( BASEPATH"hats/extra.bin", "wb" );

	if( !f ) return;

	gzwrite(f, savegamebuf, savegame_size);
	gzwrite(f, &chksum, sizeof(chksum));
	gzclose( f );
}

void load_game( void )
{
	gzFile f = NULL;
	int i=0, calcsum, chksum;

	if( !savegamebuf ) return;

	savegame_available = SDL_FALSE;
#ifdef __android__
	if (use_alt_basepath)
		f = gzopen( BASEPATH_ALT"hats/extra.bin", "rb" );
	else
#endif
		f = gzopen( BASEPATH"hats/extra.bin", "rb" );

	if( !f ) return;

	if( gzread(f, savegamebuf, savegame_size) != savegame_size )
	{
		gzclose( f );
		return;
	}

	if ( gzread(f, &chksum, sizeof(chksum)) != sizeof(chksum) )
	{
		gzclose( f );
		return;
	}

	gzclose( f );

	calcsum = 0x53492873;
	for( i=0; i<savegame_size; i++ )
		calcsum += savegamebuf[i];

	if ( calcsum != chksum ) return;

	savegame_available = SDL_TRUE;
}

void continue_game( void )
{
	int i;

	if( !savegamebuf ) return;
	if( !savegame_available ) return;

	i = 0;

	memcpy(&clevel,    &savegamebuf[i], sizeof(clevel)   ); i += sizeof(clevel);
	memcpy(&extratime, &savegamebuf[i], sizeof(extratime)); i += sizeof(extratime);
	memcpy(&gid,       &savegamebuf[i], sizeof(gid)      ); i += sizeof(gid);
	memcpy(&stuff,     &savegamebuf[i], sizeof(stuff)    ); i += sizeof(stuff);
	memcpy(strig,      &savegamebuf[i], ST_LAST          ); i += ST_LAST;
	memcpy(inv,        &savegamebuf[i], INV_LAST         ); i += INV_LAST;
	memcpy(&fgx,       &savegamebuf[i], sizeof(fgx)      ); i += sizeof(fgx);
	memcpy(&fgy,       &savegamebuf[i], sizeof(fgy)      ); i += sizeof(fgy);
	memcpy(&bgx,       &savegamebuf[i], sizeof(bgx)      ); i += sizeof(bgx);
	memcpy(&bgy,       &savegamebuf[i], sizeof(bgy)      ); i += sizeof(bgy);

	restore_thingy_active(&i, n_things1);
	restore_thingy_active(&i, p_things1);
	restore_thingy_active(&i, g_things1);
	restore_thingy_active(&i, n_things2);
	restore_thingy_active(&i, p_things2);
	restore_thingy_active(&i, g_things2);
	restore_thingy_active(&i, n_things3);
	restore_thingy_active(&i, p_things3);
	restore_thingy_active(&i, g_things3);
	restore_thingy_active(&i, n_things4);
	restore_thingy_active(&i, p_things4);
	restore_thingy_active(&i, g_things4);
	restore_thingy_active(&i, n_things5);
	restore_thingy_active(&i, p_things5);
	restore_thingy_active(&i, g_things5);

	restore_lift_saveable(&i, lifts1);
	restore_lift_saveable(&i, lifts2);
	restore_lift_saveable(&i, lifts3);
	restore_lift_saveable(&i, lifts4);
	restore_lift_saveable(&i, lifts5);

	gid.usemode = FALSE;
	gid.stopuntilfade = FALSE;
}

void save_options( void )
{
	FILE *f;
#ifdef __android__
	if (use_alt_basepath)
		f = fopen( BASEPATH_ALT"settings.txt", "w" );
	else
#endif
		f = fopen( BASEPATH"settings.txt", "w" );
	if( !f ) return;

	fprintf( f, "sfxvol %d\n", sfxvolopt );
	fprintf( f, "musicvol %d\n", musicvolopt );
	fprintf( f, "icade %d\n", icademode );
	fclose( f );
}

void load_options( void )
{
	FILE *f;
	int i;
	char ltmp[80];
#ifdef __android__
	if (use_alt_basepath)
		f = fopen( BASEPATH_ALT"settings.txt", "r" );
	else
#endif
		f = fopen( BASEPATH"settings.txt", "r" );

	if( !f ) return;

	while( !feof( f ) )
	{
		if( fgets( ltmp, 80, f ) == NULL )
			break;

		if( strncmp( ltmp, "sfxvol ", 7 ) == 0 )
		{
			i = atoi( &ltmp[7] );
			if( ( i >= 0 ) && ( i <= 8 ) )
			{
				sfxvolopt = i;
				sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
			}
			continue;
		}

		if( strncmp( ltmp, "musicvol ", 9 ) == 0 )
		{
			i = atoi( &ltmp[9] );
			if( ( i >= 0 ) && ( i <= 8 ) )
			{
				musicvolopt = i;
				musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
				setmusicvol();
			}
			continue;
		}

		if( strncmp( ltmp, "icade ", 6 ) == 0 )
		{
			icademode = atoi(&ltmp[6]) != 0;
			continue;
		}
	}

	fclose( f );
}

BOOL init( void )
{
	savegame_size = sizeof(clevel) + sizeof(extratime) + sizeof(gid) + sizeof(stuff) +
			ST_LAST + INV_LAST + sizeof(fgx) + sizeof(fgy) +
			sizeof(bgx) + sizeof(bgy);

	savegame_size += calc_thingy_active_space( n_things1 );
	savegame_size += calc_thingy_active_space( p_things1 );
	savegame_size += calc_thingy_active_space( g_things1 );
	savegame_size += calc_thingy_active_space( n_things2 );
	savegame_size += calc_thingy_active_space( p_things2 );
	savegame_size += calc_thingy_active_space( g_things2 );
	savegame_size += calc_thingy_active_space( n_things3 );
	savegame_size += calc_thingy_active_space( p_things3 );
	savegame_size += calc_thingy_active_space( g_things3 );
	savegame_size += calc_thingy_active_space( n_things4 );
	savegame_size += calc_thingy_active_space( p_things4 );
	savegame_size += calc_thingy_active_space( g_things4 );
	savegame_size += calc_thingy_active_space( n_things5 );
	savegame_size += calc_thingy_active_space( p_things5 );
	savegame_size += calc_thingy_active_space( g_things5 );

	savegame_size += calc_lift_saveable_space( lifts1 );
	savegame_size += calc_lift_saveable_space( lifts2 );
	savegame_size += calc_lift_saveable_space( lifts3 );
	savegame_size += calc_lift_saveable_space( lifts4 );
	savegame_size += calc_lift_saveable_space( lifts5 );

	savegamebuf = malloc(savegame_size);

	load_options();
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) < 0 )
	{
#ifdef __android__
		LOGI( "SDL says: %s\n", SDL_GetError() );
#else
		printf( "SDL says: %s\n", SDL_GetError() );
#endif
		return FALSE;
	}


	needquit = TRUE;

	// Tell it to use OpenGL version 1.0
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

	ssrf = SDL_SetVideoMode( 0, 0, 0, SDL_OPENGL );

	if( ssrf == 0 )
	{
#ifdef __android__
		LOGI( "SDL says: %s\n", SDL_GetError() );
#else
		printf( "SDL says: %s\n", SDL_GetError() );
#endif
		return FALSE;
	}

	initsounds();
	loadsounds();

	load_game();

	if( !render_init() ) return FALSE;

	titlestate = 0;
	what_are_we_doing = WAWD_TITLES;

	return TRUE;
}

void shut( void )
{
	killtune();
	freesounds();
	if( needquit ) SDL_Quit();
}

int main( int argc, char *argv[] )
{
#ifdef __android__
	use_alt_basepath = getAltBasePath();
#endif
	if( init() )
	{
		BOOL done, needrender, jletgoh, jletgov;
		int guipaused = 0, i;
		int pausetouch = -1, fbicontouch = -1;
		int btnspread = (vieww < 1024) ? 40 : 32;  // Spread the buttons out a bit on phone screens
		int deadzone  = (vieww < 1024) ? 12 : 8;   // Larger deadzone on smaller screens
		//    void *timer;

		gamepause = FALSE;
		jletgoh = FALSE;
		jletgov = FALSE;

		//    timer = SDL_AddTimer( FPSTIME, timing, 0 );
#ifndef __android__
		PDL_NotifyMusicPlaying(PDL_TRUE);
#endif
		done = FALSE;
		needrender = FALSE;
		while( !done )
		{
#ifdef __android__

			if (checkBackButton())
			{
				if (what_are_we_doing != WAWD_TITLES)
				{
					save_game();
					load_game();
					titlestate = 0;
					what_are_we_doing = WAWD_TITLES;
				}
			}

			if (checkNeedsPause())
			{
				save_game();
				pauseMe();
			}

			if (checkNeedsGLInit())
			{
				LOGI("checkNeedsGLInit return TRUE");
				InitEGL();
				if( !render_init() ) return FALSE;

				switch( what_are_we_doing )
				{
				case WAWD_TITLES:
				case WAWD_MENU:
				case WAWD_DEFINE_A_KEY:
					what_are_we_doing = WAWD_TITLES;
					titlestate = 0;
					break;
				case WAWD_GAME:
					load_game();
					continue_game();
					start_game(TRUE);
					break;

				case WAWD_ENDING:
					//done |= render_ending();
					break;
				}
			}
#endif

			SDL_Event event;

			if( !guipaused )
			{
				timing( FPSTIME, NULL );
				audioframe();
				unlockaudio();
				switch( what_are_we_doing )
				{
				case WAWD_TITLES:
				case WAWD_MENU:
				case WAWD_DEFINE_A_KEY:
					done |= render_titles();
					break;
				case WAWD_GAME:
					done |= render();
					break;

				case WAWD_ENDING:
					done |= render_ending();
					break;
				}

				if( !SDL_PollEvent( &event ) )
					continue;
			} else {
				SDL_WaitEvent( &event );
			}

			do {
				switch( event.type )
				{
				case SDL_MOUSEBUTTONUP:
#ifndef __android__ //Not for Android because UP is always preceeded by MOVE, and event.motion.which is not valid here
					i = event.motion.which;
#endif
					if( i > 1 ) break;
					touchx[i] = -1;

					switch( what_are_we_doing )
					{
					case WAWD_MENU:
					case WAWD_TITLES:
						if( fbicontouch != -1 )
						{
							if( i == fbicontouch )
							{
								fbicontouch = -1;
#ifndef __android__
								PDL_LaunchBrowser("http://www.facebook.com/pages/Giddy-3/50352374724");
#else
								launchBrowser(0);
#endif
							}
							break;
						}

						if( what_are_we_doing == WAWD_TITLES ) go_menus();
						break;

					case WAWD_GAME:
						if( touchjoy == i )
						{
							touchjoy = -1;
							touchjoyx = 0;
							gleft = FALSE;
							gright = FALSE;
						}
						else if( touchjump == i )
						{
							gjump = FALSE;
							touchjump = -1;
						}
						break;
					}
					break;

					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEMOTION:
#ifdef __android__
						if (event.type == SDL_MOUSEMOTION)
						{
							if( event.motion.which > 1 ) break;

							i = event.motion.which;

							touchx[i] = (event.motion.xrel * touchscalex) / 256;
							touchy[i] = (event.motion.yrel * touchscaley) / 256;
						}

#else
						i = event.motion.which;
						if( i > 1 ) break;

						if( !rotated )
						{
							touchx[i] = (event.motion.x * touchscalex) / 256;
							touchy[i] = (event.motion.y * touchscaley) / 256;
						} else {
							touchx[i] = (event.motion.y * touchscalex) / 256;
							touchy[i] = 239 - ((event.motion.x * touchscaley) / 256);
						}
#endif
						switch( what_are_we_doing )
						{
						case WAWD_TITLES:
							if( ( touchx[i] >= 4.0f ) && ( touchx[i] < 32.0f ) &&
									( touchy[i] >= 4.0f ) && ( touchy[i] < 32.0f ) )
							{
								fbicontouch = i;
								break;
							}
							break;

						case WAWD_MENU:
							if( ( touchx[i] >= 4.0f ) && ( touchx[i] < 32.0f ) &&
									( touchy[i] >= 4.0f ) && ( touchy[i] < 32.0f ) )
							{
								fbicontouch = i;
								break;
							}

							done |= menu_touch( event.type = SDL_MOUSEBUTTONDOWN, touchx[i], touchy[i] );
							break;

						case WAWD_GAME:
#ifdef __demo__
							//Check if demo end screen is showing, presses now bring up Google Play page
							if( ( gid.demofinished ) && ( gid.demofinishedstate == 1 ) )
							{
								launchBrowser(1);
								killtune();
								titlestate = 0;
								what_are_we_doing = WAWD_TITLES;
							}
#endif
							if( touchjoy == -1 )
							{
								if( (touchy[i] >= 180) && (touchy[i] < 232) &&
										(touchx[i] >= 0) && (touchx[i] < 160) )
								{
									touchjoy = i;
								}
							}

							if( i == touchjoy )
							{
								pausetouch = -1;
								touchjoyx = touchx[i] - 40;
								if (touchjoyx < -18) touchjoyx = -18;
								if (touchjoyx >  18) touchjoyx =  18;

								if (touchjoyx <= -deadzone)
								{
									gleft = TRUE;
									gright = FALSE;
								}
								else if (touchjoyx >= deadzone)
								{
									gleft = FALSE;
									gright = TRUE;
								}
								else
								{
									gleft = FALSE;
									gright = FALSE;
								}
								break;
							}

							if( event.type == SDL_MOUSEBUTTONDOWN )
							{
								if( (touchy[i] >= 180) && (touchy[i] < 232) )
								{
									if( (touchx[i] >= 282) && (touchx[i] < 314) )
									{
										gjump = TRUE;
										touchjump = i;
										pausetouch = -1;
										break;
									}

									if( (touchx[i] >= 282-btnspread) && (touchx[i] < 282) )
									{
										spacehit = TRUE;
										pausetouch = -1;
										break;
									}

									if( (touchx[i] >= 282-btnspread*2) && (touchx[i] < 282-btnspread) )
									{
										enterhit = TRUE;
										pausetouch = -1;
										break;
									}
								}

								if( (touchy[i] >= 0) && (touchy[i] < 40) &&
										(touchx[i] >= 236) && (touchx[i] < 298) )
								{
									enterhit = TRUE;
									pausetouch = -1;
									break;
								}

								if( ( touchy[i] < 40 ) || ( touchy[i] > 170 ) )
								{
									pausetouch = -1;
									break;
								}

								if( pausetouch == -1 )
								{
									pausetouch = SDL_GetTicks();
								} else {
									if( (SDL_GetTicks() - pausetouch) < 1000 )
									{
										gamepause = !gamepause;
										if( gamepause )
											giddy_say("Paused");
										pausetouch = -1;
									}
									else
									{
										pausetouch = SDL_GetTicks();
									}
								}
							}
						}
						break;

						case SDL_KEYDOWN:
							if( icademode )
							{
								switch( what_are_we_doing )
								{
								case WAWD_ENDING:
									if( endingstate < 9 )
										endingstate = 11;
									break;

								case WAWD_TITLES:
									go_menus();
									break;

								case WAWD_MENU:
									switch( event.key.keysym.sym )
									{
									case 'w':
										menu_up();
										break;

									case 'x':
										menu_down();
										break;

									case 'a':
										menu_left();
										break;

									case 'd':
										menu_right();
										break;

									case 'y':
									case 'u':
									case 'i':
									case 'o':
									case 'h':
									case 'j':
									case 'k':
									case 'l':
										done |= menu_do();
										break;

									default:
										break;
									}
									break;

									case WAWD_GAME:
										if( event.key.keysym.sym == 'a' )
										{
											gleft = TRUE;
											break;
										}

										if( event.key.keysym.sym == 'q' )
										{
											gleft = FALSE;
											break;
										}

										if( event.key.keysym.sym == 'd' )
										{
											gright = TRUE;
											break;
										}

										if( event.key.keysym.sym == 'c' )
										{
											gright = FALSE;
											break;
										}

										if( ( event.key.keysym.sym == 'h' ) ||
												( event.key.keysym.sym == 'y' ) )
										{
											gjump = TRUE;
											break;
										}

										if( ( event.key.keysym.sym == 'r' ) ||
												( event.key.keysym.sym == 't' ) )
										{
											gjump = FALSE;
											break;
										}

										if( ( event.key.keysym.sym == 'l' ) ||
												( event.key.keysym.sym == 'o' ) )
										{
											enterhit = TRUE;
											break;
										}

										if( ( event.key.keysym.sym == 'j' ) ||
												( event.key.keysym.sym == 'u' ) )
										{
											spacehit = TRUE;
											break;
										}
										break;

									default:
										break;
								}
							}
							break;

							//          case SDL_KEYUP:
							//            switch( what_are_we_doing )
							//            {
							//              case WAWD_ENDING:
							//                if( endingstate < 9 )
							//                  endingstate = 11;
							//                break;
							//
							//              case WAWD_TITLES:
							//                go_menus();
							//                break;
							//
							//              case WAWD_DEFINE_A_KEY:
							//                define_a_key( event.key.keysym.sym );
							//                break;
							//
							//              case WAWD_MENU:
							//                switch( event.key.keysym.sym )
							//                {
							//                  case SDLK_UP:
							//                    menu_up();
							//                    break;
							//
							//                  case SDLK_DOWN:
							//                    menu_down();
							//                    break;
							//
							//                  case SDLK_LEFT:
							//                    menu_left();
							//                    break;
							//
							//                  case SDLK_RIGHT:
							//                    menu_right();
							//                    break;
							//
							//                  case SDLK_RETURN:
							//                  case SDLK_SPACE:
							//                    done |= menu_do();
							//                    break;
							//
							//                  case SDLK_ESCAPE:
							//                    done = TRUE;
							//                    break;
							//
							//                  default:
							//                    if( ( event.key.keysym.sym == keyjump ) || ( event.key.keysym.sym == keyjump2 ) ||
							//                        ( event.key.keysym.sym == keyhint ) || ( event.key.keysym.sym == keyhint2 ) ||
							//                        ( event.key.keysym.sym == keyuse  ) || ( event.key.keysym.sym == keyuse2  ) )
							//                    {
							//                      done |= menu_do();
							//                      break;
							//                    }
							//                    break;
							//                }
							//                break;
							//
							//              case WAWD_GAME:
							//                if( ( event.key.keysym.sym == keyleft ) ||
							//                    ( event.key.keysym.sym == keyleft2 ) )
							//                {
							//                  pleft = FALSE;
							//                  gleft = jleft;
							//                  break;
							//                }
							//
							//                if( ( event.key.keysym.sym == keyright ) ||
							//                    ( event.key.keysym.sym == keyright2 ) )
							//                {
							//                  pright = FALSE;
							//                  gright = jright;
							//                  break;
							//                }
							//
							//                if( ( event.key.keysym.sym == keyjump ) ||
							//                    ( event.key.keysym.sym == keyjump2 ) )
							//                {
							//                  pjump = FALSE;
							//                  gjump = jjump;
							//                  break;
							//                }
							//
							//                switch( event.key.keysym.sym )
							//                {
							//                  case SDLK_ESCAPE:
							//                    titlestate = 0;
							//                    what_are_we_doing = WAWD_TITLES;
							//                    break;
							//
							//                  default:
							//                    break;
							//                }
							//                break;
							//            }
							break;

						case SDL_QUIT:
							//							LOGI("SDL_QUIT");
							done = TRUE;
							break;

						case SDL_ACTIVEEVENT:
							//							LOGI("SDL_ACTIVEEVENT type = %d, gain = %d, state = %d",event.active.type,event.active.gain,event.active.state);
#ifndef __android__
							if( event.active.state == SDL_APPACTIVE )
							{
								guipaused = !event.active.gain;
								if( audioavailable ) SDL_PauseAudio( guipaused );
								PDL_NotifyMusicPlaying(!guipaused);
							}
#endif
							break;

						case SDL_USEREVENT:
							//            needrender = TRUE;
							break;
						default:
							//							LOGI("Unknown event : %d", event.type);
							break;
				}
			} while( SDL_PollEvent( &event ) );

			//      if( needrender )
			//      {
			//        needrender = FALSE;
			//        switch( what_are_we_doing )
			//        {
			//          case WAWD_TITLES:
			//          case WAWD_MENU:
			//          case WAWD_DEFINE_A_KEY:
			//            done |= render_titles();
			//            break;
			//
			//          case WAWD_GAME:
			//            done |= render();
			//            break;
			//
			//          case WAWD_ENDING:
			//            done |= render_ending();
			//            break;
			//        }
			//      }
		}
		//		LOGI("what_are_we_doing = %d",what_are_we_doing);
		if( what_are_we_doing == WAWD_GAME )
		{
			save_game();
		}
#ifndef __android__
		PDL_NotifyMusicPlaying(PDL_FALSE);
#endif
	}
	shut();
	return 0;
}
