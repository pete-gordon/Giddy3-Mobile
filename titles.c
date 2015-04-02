
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

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
extern BOOL use_alt_basepath;
#endif

int touchmenucount = 0, touchmenugo = 0, last_menu_do = -1;

extern Uint8 sprites[], texts[], fbtext[];
extern struct what_is_giddy_doing gid;
extern double extratime;
extern BOOL icademode;

#define MAXTITLESTARS 200
#define STARSPEED 10

extern time_t gamestarttime, gameendtime;

extern BOOL audioavailable;
extern int musicvol, sfxvol;
extern int musicvolopt, sfxvolopt;
extern SDL_bool savegame_available;
static SDL_bool load_save_game = SDL_FALSE;

extern SDL_Surface *ssrf;

struct tstar
{
  int x, y;
  int dx, dy;
  int f;
};

struct tstar tstars[MAXTITLESTARS];

extern int what_are_we_doing;
int menunum, mitems;
BOOL startit = FALSE;

int testsamps[] = { SND_JUMP, SND_COIN, SND_SPRING, SND_VANHONK, SND_USE, SND_USEFAIL };


int titlestate = 0, nexttstar = 0;

// giddy 3
int titx[] = { 34, 80, 129, 186, 240, 295 };
int tits[] = {  0,  1,   2,   2,   3,   4 };
int tity[6];
float tita[6];

// titles font
int tfw[] = { 214*256*4+  12*4,  6,    // !
              214*256*4+  18*4,  9,    // "
              214*256*4+  27*4, 11,    // #
              214*256*4+  38*4, 11,    // �
              214*256*4+  49*4, 11,    // %
              214*256*4+  60*4, 12,    // &
              214*256*4+  72*4,  5,    // '
              214*256*4+  77*4,  7,    // (
              214*256*4+  84*4,  7,    // )
              214*256*4+  91*4, 11,    // *
              214*256*4+ 102*4,  9,    // +
              214*256*4+ 111*4,  6,    // ,
              214*256*4+ 117*4, 10,    // -
              214*256*4+ 127*4,  6,    // .
              214*256*4+ 133*4, 11,    // /
              214*256*4+ 144*4, 11,    // 0
              214*256*4+ 155*4,  8,    // 1
              214*256*4+ 163*4, 11,    // 2
              214*256*4+ 174*4, 11,    // 3
              214*256*4+ 185*4, 11,    // 4
              214*256*4+ 196*4, 11,    // 5
              214*256*4+ 207*4, 11,    // 6
              214*256*4+ 218*4, 11,    // 7
              214*256*4+ 229*4, 11,    // 8
              214*256*4+ 240*4, 11,    // 9
              214*256*4+ 251*4,  5,    // :
              228*256*4+   0*4,  6,    // ;
              228*256*4+   6*4,  7,    // <
              228*256*4+  13*4,  7,    // =
              228*256*4+  20*4,  7,    // >
              228*256*4+  27*4, 11,    // ?
              228*256*4+  38*4, 11,    // @
              228*256*4+  49*4, 11,    // A
              228*256*4+  60*4, 11,    // B
              228*256*4+  71*4, 11,    // C
              228*256*4+  82*4, 11,    // D
              228*256*4+  93*4, 11,    // E
              228*256*4+ 104*4, 11,    // F
              228*256*4+ 115*4, 11,    // G
              228*256*4+ 126*4, 11,    // H
              228*256*4+ 137*4,  6,    // I
              228*256*4+ 143*4, 11,    // J
              228*256*4+ 154*4, 11,    // K
              228*256*4+ 165*4, 11,    // L
              228*256*4+ 176*4, 11,    // M
              228*256*4+ 187*4, 11,    // N
              228*256*4+ 198*4, 11,    // O
              228*256*4+ 209*4, 11,    // P
              228*256*4+ 220*4, 11,    // Q
              228*256*4+ 231*4, 11,    // R
              228*256*4+ 242*4, 11,    // S
              242*256*4+   0*4, 11,    // T
              242*256*4+  11*4, 11,    // U
              242*256*4+  22*4, 11,    // V
              242*256*4+  33*4, 11,    // W
              242*256*4+  44*4, 11,    // X
              242*256*4+  55*4, 11,    // Y
              242*256*4+  66*4, 11,    // Z
              242*256*4+  77*4,  7,    // [
              242*256*4+  84*4, 11,    //
              242*256*4+  95*4,  7,    // ]
              242*256*4+ 102*4, 18,    // ^
              242*256*4+ 120*4, 18,    // _
              242*256*4+ 138*4, 11 };  // `

int tfg[4*12];

extern int fadea, fadeadd, fadetype, lrfade2;
extern GLuint tex[TEX_LAST];
extern struct btex sprtt[], sprtv[];

char *straplines[] = { "REASONABLY SPECIAL EDITION",
                       "THE RETRO EGGSPERIENCE",
                       "- BETTER THAN SPROUTS -",
                       "WITH GARISH TITLE SCREEN",
                       "FULLY 3D (POSSIBLE LIE)",
                       "- REPROGRAMMED BY HAND -",
                       "WHAT THE FLIPPING EGG?",
                       "- BEARD CONTENT: LOW -",
                       "- HE IS THE EGG-MAN -" };

int nextstrapline=0;

int stx=0, sty=0, sstx=0, ssty=0;
float sta=0.0f, ssta=0;
int strapx, strapw, strapsw, strapy;

int titlepage=0, titlepage_timer;
int titlepage_in_type = 0, titlepage_out_type = 0;

int tpx, tpxo, tpw=0;
int tpy, tpyo, tph=0;
int tpalph;
float tpscale;

BOOL pagerefresh = FALSE;

int menustaro=0, menustary=-1;
float menustara;

static int txwidth( char *str )
{
  int w, i;

  for( w=0, i=0; str[i]; i++ )
  {
    if( ( str[i] < '!' ) || ( str[i] > '`' ) )
    {
      w += 11;
      continue;
    }

    w += tfw[(str[i]-'!')*2+1];
  }

  return w;
}

static int txchar( int x, int y, int c, int col )
{
  int cx, cy, w;
  int cr, cg, cb;
  Uint32 cmp;
  Uint8 *s, *d;

  cb = (col & 3) * 12;
  cg = ((col>>2) & 3) * 12;
  cr = ((col>>4) & 3) * 12;

  if( ( c < '!' ) || ( c > '`' ) )
    return 11;

  c = (c-'!')*2;
  w = tfw[c+1];
  s = &sprites[tfw[c]];
  d = &texts[(y*512+x)*4];

  for( cy=0; cy<14; cy++ )
  {
    for( cx=0; cx<w; cx++ )
    {
      cmp = (s[0]<<16)|(s[1]<<8)|s[2];

      switch( cmp )
      {
        case 0x000050:
          *(d++) = tfg[cr+cy]-12;
          *(d++) = tfg[cg+cy]-12;
          *(d++) = tfg[cb+cy]-12;
          *(d++) = s[3];
          s+=4;
          break;
        
        case 0x000060:
          *(d++) = tfg[cr+cy]+15;
          *(d++) = tfg[cg+cy]+15;
          *(d++) = tfg[cb+cy]+15;
          *(d++) = s[3];
          s+=4;
          break;
        
        case 0x000058:
          *(d++) = tfg[cr+cy];
          *(d++) = tfg[cg+cy];
          *(d++) = tfg[cb+cy];
          *(d++) = s[3];
          s+=4;
          break;
        
        default:
          *(d++) = *(s++);
          *(d++) = *(s++);
          *(d++) = *(s++);
          *(d++) = *(s++);
          break;
      }
    }
    s += 1024-(w*4);
    d += 2048-(w*4);
  }

  return w;
}

static void txprint( int x, int y, char *str, int col )
{
  int i, cx;

  for( cx=x, i=0; str[i]; i++ )
    cx += txchar( cx, y, str[i], col );
}

static void txcentre( int y, char *str, int col )
{
  txprint( (320-txwidth(str))/2, y, str, col );
}

#define NUM_IN_TYPES 4
#define NUM_OUT_TYPES 4
#define NUM_PAGES 6

//char sktmp[6];
//
//char *sdl_key_to_str( int skey )
//{
//  strcpy( sktmp, "     " );
//
//  if( ( skey >= 'a' ) && ( skey <= 'z' ) )
//    skey -= 32;
//
//  if( ( ( skey >= '0' ) && ( skey <= '9' ) ) ||
//      ( ( skey >= 'A' ) && ( skey <= 'Z' ) ) )
//  {
//    sktmp[2] = skey;
//    return sktmp;
//  }
//
//  switch( skey )
//  {
//    case SDLK_SPACE:
//      return "SPACE";
//
//    case SDLK_RETURN:
//      return "ENTER";
//    
//    case SDLK_LSHIFT:
//      return "LSHFT";
//
//    case SDLK_RSHIFT:
//      return "RSHFT";
//    
//    case SDLK_LCTRL:
//      return "LCTRL";
//
//    case SDLK_RCTRL:
//      return "RCTRL";
//
//    case SDLK_LEFT:
//      return "LEFT ";
//
//    case SDLK_RIGHT:
//      return "RIGHT";
//
//    case SDLK_UP:
//      return "  UP ";
//
//    case SDLK_DOWN:
//      return "DOWN ";
//  }
//  return NULL;
//}

void render_menustars( void )
{
  int x, y;

  if( menustary == -1 ) return;

  for( x=menustaro+16; x<304; x+=8 )
    render_sprite_scaled( &sprtt[5], x, menustary-5, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( y=-5+(x-304); y<15; y+=8 )
    render_sprite_scaled( &sprtt[5], 304, menustary+y, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( x=304-(y-15); x>16; x-=8 )
    render_sprite_scaled( &sprtt[5], x, menustary+15, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( y=16-(16-x); y>-5; y-=8 )
    render_sprite_scaled( &sprtt[5], 16, menustary+y, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );
}

void setup_titlepage( void )
{
  memset( &texts[0], 0, 512*240*4 );

  if( ( what_are_we_doing == WAWD_MENU ) ||
      ( what_are_we_doing == WAWD_DEFINE_A_KEY ) )
  {
    switch( titlepage )
    {
      case 0: //// Main menu page
        if( savegame_available )
        {
          txcentre( 14, "CONTINUE GAME", menunum==0 ? 0x3d : 0x08 );
          txcentre( 40, "START GAME",    menunum==1 ? 0x3d : 0x08 );
          txcentre( 58, "OPTIONS",       menunum==2 ? 0x3d : 0x08 );
          txcentre( 76, "CREDITS",       menunum==3 ? 0x3d : 0x08 );
          txcentre(114, "QUIT",          menunum==4 ? 0x3d : 0x08 );
          menustary = menunum*18+114;
          if( menunum > 0 )  menustary+=8;
          if( menunum == 4 ) menustary+=20;
          mitems = 5;
          break;
        }

        txcentre( 14, "START GAME",    menunum==0 ? 0x3d : 0x08 );
        txcentre( 32, "OPTIONS",       menunum==1 ? 0x3d : 0x08 );
        txcentre( 50, "CREDITS",       menunum==2 ? 0x3d : 0x08 );
        txcentre( 88, "QUIT",          menunum==3 ? 0x3d : 0x08 );
        menustary = menunum*18+114;
        if( menunum == 3 ) menustary+=20;
        mitems = 4;
        break;
      
      case 1: //// Options page
        txcentre( 0, "OPTIONS", 0x1b );

        txprint(  22, 32, "SFX VOL", menunum==0 ? 0x3d : 0x08 );
        txprint( 128, 32, "[^_______`]", 0x00 );
        txprint( 135+(sfxvolopt*18), 32, "*", menunum==0 ? 0x3d : 0x08 );
        txprint(  22, 50, "MUSIC VOL", menunum==1 ? 0x3d : 0x08 );
        txprint( 128, 50, "[^_______`]", 0x00 );
        txprint( 135+(musicvolopt*18), 50, "*", menunum==1 ? 0x3d : 0x08 );
#ifndef __android__
        if( icademode )
          txcentre( 68, "ICADE: ENABLED", menunum == 2 ? 0x3d : 0x08 );
        else
          txcentre( 68, "ICADE: DISABLED", menunum == 2 ? 0x3d : 0x08 );
#endif
//        if( fullscreen )
//          txcentre( 68, "VIDEO: FULLSCREEN", menunum==2 ? 0x3d : 0x08 );
//        else
//          txcentre( 68, "VIDEO: WINDOW", menunum==2 ? 0x3d : 0x08 );

#ifndef __android__
        txcentre( 98, "BACK",          menunum==3 ? 0x3d : 0x08 );
#else
        txcentre( 98, "BACK",          menunum==2 ? 0x3d : 0x08 );
#endif
        switch( menunum )
        {
          case 0: menustary = 132; break;
          case 1: menustary = 150; break;
          case 2: menustary = 168; break;
          case 3: menustary = 198; break;
        }

        mitems = 4;
        break;
      
//      case 2: //// Key settings page
//        txcentre( 0, "KEY SETTINGS", 0x1b );
//
//        txprint(  80, 18, "LEFT",  menunum==0 ? 0x3d : 0x08 );
//        if( ( what_are_we_doing == WAWD_DEFINE_A_KEY ) && ( menunum == 0 ) )
//          txprint( 180, 18, "?????", 0x17 );
//        else
//          txprint( 180, 18, sdl_key_to_str( keyleft ),  menunum==0 ? 0x3d : 0x08 );
//        txprint(  80, 32, "RIGHT", menunum==1 ? 0x3d : 0x08 );
//        if( ( what_are_we_doing == WAWD_DEFINE_A_KEY ) && ( menunum == 1 ) )
//          txprint( 180, 32, "?????", 0x17 );
//        else
//          txprint( 180, 32, sdl_key_to_str( keyright ), menunum==1 ? 0x3d : 0x08 );
//        txprint(  80, 46, "JUMP",  menunum==2 ? 0x3d : 0x08 );
//        if( ( what_are_we_doing == WAWD_DEFINE_A_KEY ) && ( menunum == 2 ) )
//          txprint( 180, 46, "?????", 0x17 );
//        else
//          txprint( 180, 46, sdl_key_to_str( keyjump ),  menunum==2 ? 0x3d : 0x08 );
//        txprint(  80, 60, "USE",   menunum==3 ? 0x3d : 0x08 );
//        if( ( what_are_we_doing == WAWD_DEFINE_A_KEY ) && ( menunum == 3 ) )
//          txprint( 180, 60, "?????", 0x17 );
//        else
//          txprint( 180, 60, sdl_key_to_str( keyuse ),   menunum==3 ? 0x3d : 0x08 );
//        txprint(  80, 74, "HINT",  menunum==4 ? 0x3d : 0x08 );
//        if( ( what_are_we_doing == WAWD_DEFINE_A_KEY ) && ( menunum == 4 ) )
//          txprint( 180, 74, "?????", 0x17 );
//        else
//          txprint( 180, 74, sdl_key_to_str( keyhint ),  menunum==4 ? 0x3d : 0x08 );
//
//        txcentre( 92, "BACK", menunum==5 ? 0x3d : 0x08 );
//
//        if( menunum < 5 )
//        {
//          menustary = menunum*14+118;
//        } else {
//          menustary = 192;
//        }
//
//        mitems = 6;
//        break;

      case 2: //// Credits page 1
        txcentre(  28, "DESIGN, GRAPHICS & SFX:", 0x3d );
        txcentre(  42, "PHIL RUSTON", 0x1b );
        txcentre(  70, "SPECIAL EDITION PROGRAMMING:", 0x3d );
        txcentre(  84, "PETER GORDON", 0x1b );
        menustary = -1;
        mitems = 0;
        break;

      case 3: //// Credits page 2
        txcentre(   0, "MUSIC BY:", 0x3d );
        txcentre(  14, "LEE BEVAN", 0x1b );
        txcentre(  28, "DANIEL JOHANSSON", 0x1b );
        txcentre(  42, "JOGIER LILJEDAHL", 0x1b );
        txcentre(  56, "SAMI SAARNIO", 0x1b );
        txcentre(  84, "SPECIAL THANKS TO", 0x0f );
        txcentre(  98, "SPOT / UP ROUGH", 0x0f );
        menustary = -1;
        mitems = 0;
        break;

      case 4: //// Credits page 3
        txcentre(  0, "MORPHOS BUILD:", 0x3d );
        txcentre(  14, "ILKKA LEHTORANTA", 0x1b );
        txcentre(  42, "OS X BUILD:", 0x3d );
        txcentre(  56, "GABOR BUKOVICS", 0x1b );
        txcentre(  84, "ANDROID BUILD:", 0x3d );
        txcentre(  98, "EMILE BELANGER", 0x1b );
        menustary = -1;
        mitems = 0;
        break;
		}

    tpx     = 0;
    tpxo    = 0;
    tpw     = 320;
    tpy     = 0;
    tpyo    = 0;
    tph     = 140;
    tpscale = 1.0f;
    tpalph  = 255;
    pagerefresh = TRUE;
    return;
  }

  switch( titlepage )
  {
//    case 0: // Intro pages
//      txcentre(   0, "THIS GAME IS FREELY", 0x1d );
//      txcentre(  14, "DISTRIBUTABLE. HOWEVER, IF", 0x1d );
//      txcentre(  28, "YOU ENJOY PLAYING IT, ", 0x1d );
//      txcentre(  42, "CONSIDER SENDING A PAYPAL", 0x1d );
//      txcentre(  56, "DONATION TO:", 0x1d );
//      txcentre(  70, "DONATIONS@GIDDY3.CO.UK", 0x3d );
//      txcentre(  98, "THANKS!", 0x1d );
//      break;

    case 0:
      txcentre(   0, "* STORY TIME *", 0x3d );
      txcentre(  28, "AUGUST, SPACE YEAR 2016", 0x1b );
      txcentre(  42, "WE FIND THE WORLD IN THE", 0x1b );
      txcentre(  56, "GRIP OF APATHY AS AN", 0x1b );
      txcentre(  70, "ARMADA OF ALIEN", 0x1b );
      txcentre(  84, "SPACESHIPS HOVERS ABOVE", 0x1b );
      txcentre(  98, "EARTH'S CAPITAL CITIES..", 0x1b );
      break;

    case 1:
      txcentre(   0, "APATHY? YES INDEED.", 0x1d );
      txcentre(  14, "YOU SEE, SUCH INVASIONS", 0x1d );
      txcentre(  28, "HAD BECOME A REGULAR", 0x1d );
      txcentre(  42, "THING AROUND FRIDAY TEA", 0x1d );
      txcentre(  56, "TIMES, AND AS THE", 0x1d );
      txcentre(  70, "ALIENS WERE ONLY TWO", 0x1d );
      txcentre(  84, "INCHES TALL THEY DIDNT", 0x1d );
      txcentre(  98, "POSE MUCH OF A THREAT..", 0x1d );
      break;

    case 2:
      txcentre(   0, "FORTUNATELY FOR THIS", 0x17 );
      txcentre(  14,"STORY, THIS TIME THINGS", 0x17 );
      txcentre(  28, "WERE DIFFERENT. FROM", 0x17 );
      txcentre(  42, "THESE UFOS CAME DEADLY", 0x17 );
      txcentre(  56, "ROBOT STOMPERS ARMED", 0x17 );
      txcentre(  70, "WITH DARK GLASSES, A", 0x17 );
      txcentre(  84, "GENERAL BAD ATTITUDE", 0x17 );
      txcentre(  98, "AND WIBBLEWAVE RAY-GUNS.", 0x17 );
      break;
    
    case 3:
      txcentre(   0, "THE WORLD WAS CAUGHT", 0x3d );
      txcentre(  14, "ON THE HOP. ONLY ONE", 0x3d );
      txcentre(  28, "MAN COULD SAVE THE DAY,", 0x3d );
      txcentre(  42, "BUT THAT MAN WAS BUSY", 0x3d );
      txcentre(  56, "IRONING HIS PANTS.", 0x3d );
      txcentre(  70, "SO INSTEAD, GIDDY, THE", 0x3d );
      txcentre(  84, "EGG-SHAPED SUPERHERO", 0x3d );
      txcentre(  98, "DECIDED TO HAVE A GO.", 0x3d );
      break;

    case 4:
      txcentre(  0, "* REALITY TIME *", 0x1b );
      txcentre( 28, "WHAT ALL THIS OLD TOSH", 0x1d );
      txcentre( 42, "BOILS DOWN TO IS A 2D", 0x1d );
      txcentre( 56, "PLATFORM / PUZZLE GAME", 0x1d );
      txcentre( 70, "LIKE THOSE OF YORE..", 0x1d );
      txcentre( 98, "BUT WITH A DASH OF IRONY.", 0x3d );
      break;

    case 5:
      txcentre(  0, "USING ONLY THE POWER OF", 0x17 );
      txcentre( 14, "YOUR MIND AND FINGERS", 0x17 );
      txcentre( 28, "YOU MUST SOLVE A LOAD", 0x17 );
      txcentre( 42, "OF BRUTALLY CONTRIVED", 0x17 );
      txcentre( 56, "PUZZLES WHILST JUMPING", 0x17 );
      txcentre( 70, "ABOUT COLLECTING THINGS,", 0x17 );
      txcentre( 84, "AVOIDING BADDIES AND", 0x17 );
      txcentre( 98, "ERR.. BEING A GOOD EGG.", 0x17 );
      break;
  }

  switch( titlepage_in_type )
  {
    case 0:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 140;
      tpyo    = 0;
      tph     = 0;
      tpscale = 1.0f;
      tpalph  = 255;
      break;

    case 1:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 0;
      tpyo    = 0;
      tph     = 140;
      tpscale = 0.1f;
      tpalph  = 0;
      break;

    case 2:
      tpx     = 320;
      tpxo    = 0;
      tpw     = 0;
      tpy     = 0;
      tpyo    = 0;
      tph     = 140;
      tpscale = 1.0f;
      tpalph  = 255;
      break;

    case 3:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 70;
      tpyo    = 0;
      tph     = 0;
      tpscale = 1.0f;
      tpalph  = 255;
      break;
  }

  pagerefresh = TRUE;
}

//BOOL do_define_a_key( int *keystorea, int *keystoreb, int key )
//{
//  int keya, keyb;
//
//  keya = key;
//  keyb = key;
//  if( ( key >= 'a' ) && ( key <= 'z' ) ) { keya = (key-32); keyb = key; }
//  if( ( key >= 'A' ) && ( key <= 'Z' ) ) { keya = key; keyb = (key+32); };
//
//  if( sdl_key_to_str( key ) )
//  {
//    *keystorea = keya;
//    *keystoreb = keyb;
//    actionsound( SND_BUBBLEPOP, sfxvol );
//    return TRUE;
//  }
//
//  return FALSE;
//}

//void define_a_key( int key )
//{
//  BOOL didit;
//
//  didit = FALSE;
//  switch( menunum )
//  {
//    case 0: didit = do_define_a_key( &keyleft,  &keyleft2,  key ); break;
//    case 1: didit = do_define_a_key( &keyright, &keyright2, key ); break;
//    case 2: didit = do_define_a_key( &keyjump,  &keyjump2,  key ); break;
//    case 3: didit = do_define_a_key( &keyuse,   &keyuse2,   key ); break;
//    case 4: didit = do_define_a_key( &keyhint,  &keyhint2,  key ); break;
//  }
//
//  if( didit )
//  {
//    what_are_we_doing = WAWD_MENU;
//    setup_titlepage();
//  }
//}

BOOL animate_titlepage_in( void )
{
  switch( titlepage_in_type )
  {
    case 0:
      if( tph < 140 )
      {
        tph+=2;
        tpy-=2;
        break;
      }
      return TRUE;
    
    case 1:
      if( tpalph < 255 )
      {
        tpalph+=3;
        if( tpalph > 255 )
          tpalph = 255;
      }

      if( tpscale < 1.0f )
      {
        tpscale += 0.9f/80.0f;
        if( tpscale > 1.0f )
          tpscale = 1.0f;
      } else {
        return TRUE;
      }
      break;
    
    case 2:
      if( tpw < 320 )
      {
        tpw += 4;
        tpx -= 4;
        break;
      }
      return TRUE;
    
    case 3:
      if( tph < 140 )
      {
        tph+=2;
        tpy--;
        break;
      }
      return TRUE;
  }
  return FALSE;
}

BOOL animate_titlepage_out( void )
{
  switch( titlepage_out_type )
  {
    case 0:
      if( tph > 0 )
      {
        tpyo+=2;
        tph-=2;
        return FALSE;
      }
      break;
    
    case 1:
      if( tpalph > 0 )
      {
        tpalph-=3;
        if( tpalph < 0 )
          tpalph = 0;
      }

      if( tpscale > 0.1f )
      {
        tpscale -= 0.9f/80.0f;
        return FALSE;
      }
      break;

    case 2:
      if( tpw > 0 )
      {
        tpxo+=4;
        tpw-=4;
        return FALSE;
      }
      break;
    
    case 3:
      if( tph > 0 )
      {
        tpy++;
        tpyo+=2;
        tph-=2;
        return FALSE;
      }
      break;
  }

  titlepage_in_type = (titlepage_in_type+1)%NUM_IN_TYPES;
  titlepage_out_type = (titlepage_out_type+1)%NUM_OUT_TYPES;
  titlepage = (titlepage+1)%NUM_PAGES;
  return TRUE;
}

void render_titlepage( void )
{
  GLfloat vtx[4*2], txq[4*2];

  if( ( tpw == 0 ) || ( tph == 0 ) )
    return;

  glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
  glLoadIdentity();
  glTranslatef( 0.0f, 100.0f, 0.0f );
  glColor4ub(255, 255, 255, tpalph );
  glScalef( tpscale, tpscale, 1.0f );
  set_quad_tristrip( vtx, tpx, tpy, tpx+tpw, tpy+tph );
  set_texc_tristrip( txq, tpxo/512.0f, tpyo/256.0f, (tpxo+tpw)/512.0f, (tpyo+tph)/256.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void reset_titles( void )
{
  int i;
  float a;

  stopallchannels();
  menustary = -1;

  for( i=0; i<MAXTITLESTARS; i++ )
    tstars[i].x = -1;

  a = 0.0f;
  for( i=0; i<6; i++ )
  {
    tita[i] = a;
    tity[i] = -28;
    a -= 0.4f;
  }

  for( i=0; i<12; i++ )
  {
    tfg[i   ] = ((240-8)/12)*i+8;
    tfg[i+12] = ((240-70)/12)*i+70;
    tfg[i+24] = ((240-140)/12)*i+140;
    tfg[i+36] = ((240-210)/12)*i+210;
  }

  fadea = 255;
  fadeadd = -4;
  fadetype = 0;

  load_globalsprites();

  memset( &texts[0], 0, 512*256*4 );
  strapw = txwidth( straplines[nextstrapline] );
  strapx = (320-strapw)/2;
  strapy = 96+60;
  strapsw = 0;
  txprint( 0, 240, straplines[nextstrapline], 0x31 );
  nextstrapline    = (nextstrapline+1)%8;

  glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );
}

void animate_tstars( void )
{
  int i;

  for( i=0; i<MAXTITLESTARS; i++ )
  {
    if( tstars[i].x == -1 )
      continue;
    
    tstars[i].x += tstars[i].dx;
    tstars[i].y += tstars[i].dy;
    tstars[i].f++;
    if( tstars[i].f > (5*STARSPEED) )
      tstars[i].x = -1;
  }
}

void stbg_move( void )
{
  stx -= 4;
  if( stx <= -240 ) stx = 0;
  sty -= 6;
  if( sty <= -240 ) sty = 0;
  sta += 1.0f;
  if( sta >= 360.0f ) sta -= 360.0f;

  sstx -= 3;
  if( sstx <= -120 ) sstx = 0;
  ssty -= 4;
  if( ssty <= -120 ) ssty = 0;
  ssta += 0.4f;
  if( ssta >= 360.0f ) ssta -= 360.0f;
}

void title_timing( void )
{
  int i;

  if( touchmenucount > 0 )
  {
    touchmenucount--;
    if( touchmenucount == 0 )
    {
      touchmenugo = 1;
    }
  }

  stbg_move();

  menustaro = (menustaro+1)&7;
  menustara += 3;

  animate_tstars();


  switch( titlestate )
  {
    case 1:
      for( i=0; i<6; i++ )
      {
        tita[i] += 0.03f;
        if( tita[i] > 3.14159265f/2.0f )
        {
          tita[i] = 3.14159265f/2.0f;
          if( i == 5 ) titlestate++;
        }
        tity[i] = -80 + sin(tita[i])*176;
      }
      break;
    
    case 2:
      if( strapsw < strapw )
      {
        strapsw+=2;
        for( i=0; i<3; i++ )
        {
          tstars[nexttstar].x = (strapx+strapsw)<<8;
          tstars[nexttstar].y = (strapy+(rand()%14))<<8;
          tstars[nexttstar].dx = ((rand()%3)-4)<<8;
          tstars[nexttstar].dy = ((rand()%18)-9)<<7;
          tstars[nexttstar].f = 0;
          nexttstar = (nexttstar+1)%MAXTITLESTARS;
        }
        break;
      }
      titlestate++;
      break;

    case 3:
      if( ( tity[2] < 64 ) && ( strapy > 76 ) )
      {
        strapy -= 3;
        if( strapy < 76 ) strapy = 76;
      }

      for( i=0; i<6; i++ )
      {
        if( tity[i] > 40 )
        {
          tity[i]-=4;
          if( tity[i] < 40 ) tity[i] = 40;
        } else {
          if( ( i == 5 ) && ( strapy <= 76 ) ) titlestate++;
        }
        
        if( tity[i] > 64 )
          break;
      }
      break;
    
    case 4:
      setup_titlepage();
      titlestate++;
      break;
    
    case 5:
      if( animate_titlepage_in() )
      {
        titlepage_timer = 520;
        titlestate++;
      }
      break;
    
    case 6:
      if( titlepage_timer > 0 )
      {
        titlepage_timer--;
        break;
      }
      titlestate++;
      break;
    
    case 7:
      if( animate_titlepage_out() )
        titlestate = 4;
      break;
    
    case 8:
      break;
  }
}

void render_star( int x, int y, int alpha, float scale, float ang )
{
  GLfloat vtx[4*2], txq[4*2];

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( ang, 0.0f, 0.0f, 1.0f );
  glScalef( scale, scale, 1.0f );
  glColor4ub(255, 255, 255, alpha );
  set_quad_tristrip( vtx, -53.0f, -50.0f, 53.0f,  50.0f );
  set_texc_tristrip( txq, 0.0f/256.0f, 0.0f/256.0f, 106.0f/256.0f, 100.0f/256.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_stars( void )
{
  int x, y;

  for( y=ssty/2; y<265; y+=60 )
    for( x=sstx/2; x<348; x+=60 )
      render_star( x, y, 160, 0.5f, sta );

  for( y=sty/2; y<290; y+=120 )
    for( x=stx/2; x<376; x+=120 )
      render_star( x, y, 160, 1.0f, sta );
}

void render_tstars( void )
{
  int i;

  for( i=0; i<MAXTITLESTARS; i++ )
  {
    if( tstars[i].x == -1 )
      continue;

    render_sprite_a( &sprtt[(tstars[i].f/STARSPEED)+5], tstars[i].x>>8, tstars[i].y>>8, FALSE, 0.0f, 128 );
  }
}
    
void render_strapline( float w, int y )
{
  GLfloat vtx[4*2], txq[4*2];

  if( w < 1.0f ) return;
  glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
  glLoadIdentity();
  glTranslatef( 0.0f, y, 0.0f );
  glColor4ub(255, 255, 255, 255 );
  set_quad_tristrip( vtx, strapx, 0.0f, strapx+w, 14.0f );
  set_texc_tristrip( txq, 0.0f/512.0f, 240.0f/256.0f, w/512.0f, 254.0f/256.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void go_menus( void )
{
  int i;

  if( titlestate == 0 ) return;

  what_are_we_doing = WAWD_MENU;
  titlepage = 0;
  menunum = 0;

  for( i=0; i<6; i++ )
  {
    tity[i] = 40;
    tita[i] = 3.14159265f/2.0f;
  }
  strapy = 76;
  strapsw = strapw;

  setup_titlepage();
  titlestate = 8;
}

int menu_touch( int downaction, int x, int y )
{
  int newmenunum = -1;
  int do_it = 0;

  if( touchmenucount != 0 ) return 0;

  switch( titlepage )
  {
    case 0: //// Main menu page
      if( !downaction ) return 0;
      if( savegame_available )
      {
        if( ( y >= 114 ) && ( y < 132 ) )
        {
          newmenunum = 0;
          do_it = 1;
        }
        else if( ( y >= 140 ) && ( y < 140+3*18 ) )
        {
          newmenunum = ((y-140) / 18)+1;
          do_it = 1;        
        }
        else if( ( y >= 214 ) && ( y < 228 ) )
        {
          newmenunum = 4;
          do_it = 1;
        }
		break;
      }

      if( ( y >= 114 ) && ( y < 114+4*18 ) )
      {
        newmenunum = (y-114) / 18;
        do_it = 1;        
      }
      else if( ( y >= 188 ) && ( y < 206 ) )
      {
        newmenunum = 3;
        do_it = 1;
      }
      break;
    
    case 1: //// Options page
      if( ( y >= 132 ) && ( y < 150 ) )
      {
        if( menunum == 0 )
        {
          if ((x >= 138) && (x < 284))
          {
            x = (x-138) / 17;
            if( x < 0 ) x = 0;
            if( x > 8 ) x = 8;

            if( x != sfxvolopt )
            {
              sfxvolopt = x;
              sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
              if( sfxvol )
                playsound( 0, testsamps[rand()%6], sfxvol );
              else
                stopallchannels();
              setup_titlepage();
            }
          }
        } else {
          newmenunum = 0;
        }
      }
      else if( ( y >= 150 ) && ( y < 168 ) )
      {
        if( menunum == 1 )
        {
          if ((x >= 138) && (x < 284))
          {
            x = (x-138) / 17;
            if( x < 0 ) x = 0;
            if( x > 8 ) x = 8;

            if( x != musicvolopt )
            {
              musicvolopt = x;
              musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
              setmusicvol();
              setup_titlepage();
            }
          }
        } else {
          newmenunum = 1;
        }
      }
#ifndef __android__
      else if( ( y >= 168 ) && ( y < 186 ) )
      {
        newmenunum = 2;
        do_it = 1;
      }
#endif
      else if( ( y >= 198 ) && ( y < 216 ) )
      {
#ifndef __android__
		newmenunum = 3;
#else
		newmenunum = 2;
#endif
		do_it = 1;
      }

      break;
    
    default:
      if( downaction )
        menu_do();
      break;
  }

  if( !downaction ) return 0;
  if( newmenunum != -1 )
  {
    if( newmenunum != menunum )
    {
      menunum = newmenunum;
      setup_titlepage();
      if( do_it )
      {
        touchmenucount = 10;
        actionsound( SND_CLICKYCLICK, sfxvol );
      }
    }
    else
    {
      if( do_it ) touchmenucount = 10;
    }
  }
      
  return 0;
}

void menu_up( void )
{
  if( startit ) return;
  if( mitems == 0 ) return;
  if( menunum > 0 )
  {
    menunum--;
    setup_titlepage();
    actionsound( SND_CLICKYCLICK, sfxvol );
  }
}

void menu_down( void )
{
  if( startit ) return;
  if( mitems == 0 ) return;
  if( menunum < (mitems-1) )
  {
    menunum++;
    setup_titlepage();
    actionsound( SND_CLICKYCLICK, sfxvol );
  }
}

void menu_left( void )
{
  if( startit ) return;
  if( !icademode ) return;
  switch( titlepage )
  {
    case 1:
      switch( menunum )
      {
        case 0:
          if( sfxvolopt > 0 )
          {
            sfxvolopt--;
            sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
            if( sfxvol )
              playsound( 0, testsamps[rand()%6], sfxvol );
            else
              stopallchannels();
            setup_titlepage();
          }
          break;
        
        case 1:
          if( musicvolopt > 0 )
          {
            musicvolopt--;
            musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
            setmusicvol();
            setup_titlepage();
          }
          break;
      }
      break;
  }
}

void menu_right( void )
{
  if( startit ) return;
  if( !icademode ) return;
  switch( titlepage )
  {
    case 1:
      switch( menunum )
      {
        case 0:
          if( sfxvolopt < 8)
          {
            sfxvolopt++;
            sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
            if( sfxvol )
              playsound( 0, testsamps[rand()%6], sfxvol );
            else
              stopallchannels();
            setup_titlepage();
          }
          break;
        
        case 1:
          if( musicvolopt < 8 )
          {
            musicvolopt++;
            musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
            setmusicvol();
            setup_titlepage();
          }
          break;
      }
      break;
  }
}

BOOL menu_do( void )
{
  int actual_menunum = menunum;

  if( startit ) return FALSE;
  if( ( last_menu_do != -1 ) && ( (SDL_GetTicks()-last_menu_do) < 500 ) ) return FALSE;
  last_menu_do = SDL_GetTicks();
  switch( titlepage )
  {
    case 0:
      if ( savegame_available )
      {
        if ( menunum == 0 )
        {
          actual_menunum = 0;
          load_save_game = SDL_TRUE;
        }
        else
        {
          actual_menunum = menunum-1;
          load_save_game = SDL_FALSE;
        }
      }

      switch( actual_menunum )
      {
        case 0: // Start
          fadea = 0;
          fadeadd = 4;
          fadetype = 0;
          startit = TRUE;
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
        
        case 1: // Options
          menunum = 0;
          titlepage = 1;
          setup_titlepage();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
        
//        case 2: // Redefine keys
//          menunum = 0;
//          titlepage = 2;
//          setup_titlepage();
//          actionsound( SND_BUBBLEPOP, sfxvol );
//          break;

        case 2: // Credits
          titlepage = 2;
          setup_titlepage();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;

        case 3: // Quit
          return TRUE;
      }
      break;
    
    case 1:
      switch( menunum )
      {
        case 0: // sfx vol
          break;
        
        case 1: // music vol
          break;
        
        case 2:
#ifndef __android__
		  icademode = !icademode;
          setup_titlepage();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
        
//        case 2: // fullscreen/window
//          fullscreen = !fullscreen;
//          SDL_WM_ToggleFullScreen( ssrf );
//          setup_titlepage();
//          actionsound( SND_BUBBLEPOP, sfxvol );
//          break;
        
        case 3: // back
#endif
          menunum = 1;
          titlepage = 0;
          setup_titlepage();
          save_options();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
      }
      break;
    
//    case 2:
//      switch( menunum )
//      {
//        case 0: // left
//        case 1: // right
//        case 2: // jump
//        case 3: // use
//        case 4: // hint
//          what_are_we_doing = WAWD_DEFINE_A_KEY;
//          setup_titlepage();
//          actionsound( SND_BUBBLEPOP, sfxvol );
//          break;
//
//        case 5: // back
//          menunum = 2;
//          titlepage = 0;
//          setup_titlepage();
//          save_options();
//          actionsound( SND_BUBBLEPOP, sfxvol );
//          break;
//      }
//      break;
    
    case 2:
	  case 3:
      titlepage++;
      setup_titlepage();
      actionsound( SND_BUBBLEPOP, sfxvol );
      break;

    case 4:
      titlepage = 0;
      setup_titlepage();
      actionsound( SND_BUBBLEPOP, sfxvol );
      break;
  }

  return FALSE;
}

void render_fbicon( void )
{
  GLfloat vtx[4*2], txq[4*2];
  glBindTexture( GL_TEXTURE_2D, tex[FBTEX] );
  glLoadIdentity();
  glTranslatef( 18.0f, 18.0f, 0.0f );
  glRotatef( -10.0f, 0.0f, 0.0f, 1.0f );
  glColor4ub(255, 255, 255, 255);
  set_quad_tristrip( vtx, -12.0f, -12.0f, 12.0f, 12.0f );
  set_texc_tristrip( txq, 0.0f, 0.0f, 1.0f, 1.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

BOOL render_titles( void )
{
	int i;

	if( titlestate == 0 )
	{
		startit = FALSE;
#ifdef __android__
		if (use_alt_basepath)
		{
			if( !load_sprites( BASEPATH_ALT"hats/titlestuff.bin" ) ) return TRUE;
			if( !load_raw( BASEPATH_ALT"hats/fbicon.raw", fbtext, 128, 128, FBTEX ) ) return FALSE;

			playtune( BASEPATH_ALT"hats/ttune.mod" );
		}
		else
#endif
		{
			if( !load_sprites( BASEPATH"hats/titlestuff.bin" ) ) return TRUE;
			if( !load_raw( BASEPATH"hats/fbicon.raw", fbtext, 128, 128, FBTEX ) ) return FALSE;

			playtune( BASEPATH"hats/ttune.mod" );
		}
		reset_titles();
		titlestate = 1;

	}

  if( ( startit ) && ( fadea == 255 ) )
  {
    killtune();
    start_game(load_save_game);
    what_are_we_doing = WAWD_GAME;
    fadeadd = -4;
    return FALSE;
  }

  if( pagerefresh )
  {
    glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );
    pagerefresh = FALSE;
  }

  glClearColor( 6.0f/255.0f, 187.0f/255.0f, 203.0f/255.0f, 1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );

  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  render_stars();

  for( i=0; i<6; i++ )
    render_sprite_scaled( &sprtt[tits[i]], titx[i], tity[i], FALSE, cos(tita[i])*60.0f, 1.0f+cos(tita[i])*2.0f );

  render_strapline( strapsw, strapy );
  render_titlepage();
  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  render_tstars();
  render_menustars();

  render_fbicon();

  render_tvborders();
  render_fade();
  SDL_GL_SwapBuffers();

  if( touchmenugo )
  {
    touchmenugo = 0;
    touchmenucount = 0;
    return menu_do();
  }

  return FALSE;
}

/******* ENDING STUFF ************/

int endingstate, bgcupframe, cupy;
int endparttimer;
float enz, ena, enzd, enad;
unsigned int gametime;

void start_ending( void )
{
	int i;

	time( &gameendtime );

	gametime = (unsigned int)(difftime( gameendtime, gamestarttime ) + extratime);

  load_globalsprites();

  memset( &texts[0], 0, 512*256*4 );

  fadea = 255;
  fadeadd = -4;
  fadetype = 0;

	endingstate = 0;
	bgcupframe = 0;
	cupy = 287;

  for( i=0; i<12; i++ )
  {
    tfg[i   ] = ((240-8)/12)*i+8;
    tfg[i+12] = ((240-70)/12)*i+70;
    tfg[i+24] = ((240-140)/12)*i+140;
    tfg[i+36] = ((240-210)/12)*i+210;
  }

  glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );
#ifdef __android__
  if (use_alt_basepath)
	  playtune( BASEPATH_ALT"hats/ctune.mod" );
  else
#endif
	  playtune( BASEPATH"hats/ctune.mod" );
}

void render_bgcups( void )
{
  int x, y, f;

	f = (bgcupframe>>2)&7;

  for( y=ssty/2; y<265; y+=60 )
    for( x=sstx/2; x<348; x+=60 )
		  render_sprite_scaleda( &sprtv[f], x, y, FALSE, 0.0f, 0.5f, 80 );

  for( y=sty/2; y<290; y+=120 )
    for( x=stx/2; x<376; x+=120 )
		  render_sprite_scaleda( &sprtv[f], x, y, FALSE, 0.0f, 1.0f, 80 );

	render_sprite_scaleda( &sprtv[f], 160, cupy, FALSE, 0.0f, 1.0f, 255 );
}

void ending_timing( void )
{
	char tmp[64];
  stbg_move();
	bgcupframe++;

	switch( endingstate )
	{
		case 1:
      memset( &texts[0], 0, 512*256*4 );
			txcentre(  14+14, "** CONGRATULATIONS **", 0x3d );
			txcentre(  42+14, "YOU'VE SAVED THE WORLD", 0x17 );
			txcentre(  56+14, "AND IT ONLY TOOK YOU:", 0x17 );
			sprintf( tmp, "%u MINS AND %02u SECONDS", gametime/60, gametime%60 );
			txcentre(  84+14, tmp, 0x17 );
			txcentre( 112+14, "ADDITIONALLY, YOU MADE A", 0x0c );
			txcentre( 126+14, "HEALTHY PROFIT OF:", 0x0c );
			sprintf( tmp, "%d COINS", gid.coins );
			txcentre( 154+14, tmp, 0x0c );
			txcentre( 182+14, "SPEND IT WISELY NOW!", 0x0c );
			pagerefresh = 1;
			enz = 0.01f;
			ena = 360.0f;
			
			enzd = (1.0f-enz) / 60.0f;
			enad = (0.0f-ena) / 60.0f;

			endparttimer = 480;
			endingstate++;
			break;
		
		case 2:
	  case 6:
			enz += enzd;
		  ena += enad;

			if( enz >= 1.0f )
		  {
				enz = 1.0f;
        ena = 0.0f;
				endingstate++;
		  }
			break;
	  
		case 3:
		case 8:
			if( endparttimer > 0 )
		  {
			  endparttimer--;
				break;
			}

			endingstate++;
			break;

    case 4:
			enz -= enzd;
		  ena += enad;
			if( enz <= 0.01f )
		  {
				enz = 0.01f;
				ena = 0.0f;
				endingstate++;
			}
			break;

    case 5:
      memset( &texts[0], 0, 512*256*4 );
		  txcentre( 14, "YOU ARE HEREBY AWARDED THE", 0x31 );
			txcentre( 28, "GOLDEN EGG OF SPLENDIDNESS:", 0x31 );
			pagerefresh = 1;
			enz = 0.01f;
			ena = 360.0f;
			
			enzd = (1.0f-enz) / 60.0f;
			enad = (0.0f-ena) / 60.0f;

			endparttimer = 900;
			endingstate++;
      break;

    case 7:
    	if( cupy > 140 )
  		{
    		cupy--;
				break;
			}

      endingstate++;
		  break;

    case 9:
   		fadea = 0;
	    fadeadd = 4;
	    fadetype = 0;
			endingstate++;
			break;
	  
		case 10:
			enz -= enzd;
		  ena += enad;
			if( enz <= 0.01f )
		  {
				enz = 0.01f;
				ena = 0.0f;
			}

			if( lrfade2 < 255 )
				break;

		  titlestate = 0;
			what_are_we_doing = WAWD_TITLES;
			break;

		case 11:
			fadea = 0;
		  fadeadd = 4;
			fadetype = 0;
			endingstate++;
			break;

    case 12:
			if( lrfade2 < 255 )
			  break;

		  titlestate = 0;
			what_are_we_doing = WAWD_TITLES;
			break;
  }
}

BOOL render_ending( void )
{
  GLfloat vtx[4*2], txq[4*2];

  if( endingstate == 0 )
  {
#ifdef __android__
	  if (use_alt_basepath)
	  {
		  if( !load_sprites( BASEPATH_ALT"hats/victorystuff.bin" ) ) return TRUE;
	  }
	  else
#endif
		  if( !load_sprites( BASEPATH"hats/victorystuff.bin" ) ) return TRUE;

	  start_ending();
	  endingstate = 1;
	  return FALSE;
  }

 if( pagerefresh )
  {
    glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );
    pagerefresh = FALSE;
  }

  glClearColor( 6.0f/255.0f, 187.0f/255.0f, 203.0f/255.0f, 1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );

  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  render_bgcups();

  glBindTexture( GL_TEXTURE_2D, tex[TTEXTTEX] );
  glLoadIdentity();
  glTranslatef( 160.0f, 120.0f, 0.0f );
  glRotatef( ena, 0.0f, 0.0f, 1.0f );
  glScalef( enz, enz, 1.0f );
  glColor4ub(255, 255, 255, 255 );
  set_quad_tristrip( vtx, -160.0f, -120.0f, 160.0f, 120.0f );
  set_texc_tristrip( txq, 0.0f/512.0f, 0.0f/256.0f, 320.0f/512.0f, 240.0f/256.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	render_tvborders();
	render_fade();
	SDL_GL_SwapBuffers();

	return FALSE;
}
