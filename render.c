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
#include "specials.h"
#include "enemies.h"
#include "samples.h"
#include "titles.h"

#define SCROFF 140

#ifdef __android__
extern BOOL use_alt_basepath;
#endif

extern int touchx[2], touchy[2], touchscalex, touchscaley;
extern int touchjoy, touchjoyx;
int cardw, cardh, vieww, viewh, rotated;
extern Uint8 *savegamebuf;

time_t gamestarttime, gameendtime;
double extratime;

//char bollocks[128];
extern int mousx, mousy, what_are_we_doing;

extern int lastsound, bincount;
extern int dragonmode, titlestate;

extern struct enemy *enemies[], *enemiesb[];
extern int stuffloop[];
extern int endingstate;

extern int musicvol, sfxvol;

extern BOOL onthebin;
extern BOOL musicon;
extern SDL_bool savegame_available;

BOOL ignorejump = FALSE;

int clevel, llevel, lastinv;
Uint32 frame;

extern BOOL gleft,gright,gjump;
extern BOOL spacehit,enterhit,gamepause;

BOOL strsneedupdate=FALSE;
/*
#ifdef __amigaos4__
extern BOOL screenbodge;
#endif
*/
extern struct infotext infos1[], infos2[], infos3[], infos4[], infos5[];
struct infotext *infos;
int winfo;

// RGBA Texture buffers
Uint8 blocks[256*256*4];
Uint8 sprites[256*256*4], wsprites[256*256*4];
Uint8 gsprites[256*256*4], wgsprites[256*256*4];
Uint8 psprites[256*256*4], wpsprites[256*256*4];
Uint8 texts[256*256*4 + 512*512*4];
Uint8 stick[64*64*4];
Uint8 fbtext[128*128*4];
Uint8 *screentex;

Uint8 blktrans[512];
Uint8 collisionarea[16*16*3*2];
Uint8 liftstoparea[16*16*3*2];

Uint8 strig[ST_LAST], inv[INV_LAST];

#define MAXGVTX 512

GLfloat gvtx[MAXGVTX*6*2];
GLfloat gtxq[MAXGVTX*6*2];

int fadea=255, fadeadd=-6, fadetype=0;
int lrfade1=255, lrfade2=255;

int infobulbwinfo = -1, infobulbalpha = 0;
float infobulbzoom;

struct invtext ivtexts[] = { { "Coins",                "" },
                             { "Air Horn",             "..An Air Horn.."                     },
                             { "Barrel of Beer",       "..A Barrel of Beer.."                },
                             { "Party Hits CD",        "..A Hideous Party Hits CD.."         },
                             { "Hose Pipe",            "..A Hose pipe.."                     },
                             { "Catapult",             "..A Catapult.."                      },
                             { "Spade",                "..A Spade.."                         },
                             { "Tub of Lard",          "..A Tub of Lard.."                   },
                             { "Bubble Gum",           "..Some Bubble Gum.."                 },
                             { "Control Box",          "..A Control Box.."                   },
                             { "Camera (Inc.Photo)",   "..Camera with photo.."               },
                             { "Large Cog",            "..A Large Cog.."                     },
                             { "Enormous Boot",        "..An Enormous Boot.."                },
                             { "Sturdy Plank",         "..A Sturdy Plank.."                  },
                             { "Scissors",             "..A Pair of Scissors.."              },
                             { "Turps",                "..Some Turps.."                      },
                             { "Candle Stick",         "..A Candle Stick.."                  },
                             { "Diamond",              "..A Diamond.."                       },
                             { "Lighted Candle",       "..A Lighted Candle.."                },
                             { "Balloon wreckage",     "..'Weather Balloon' Wreckage.." },
                             { "Lump of Carbon",       "..A Lump of Carbon.."                },
                             { "Electrical Toolkit",   "..An Electrical Toolkit.."           },
                             { "The A-Teams Tel No",   "..The A-Team's Phone Number.."       },
                             { "TNT Detonator",        "..An Explosives Detonator.."         },
                             { "HardHat",              "..A Hard Hat.."                      },
                             { "Mirror",               "..A Mirror.."                        },
                             { "Indigestion Pills",    "..Some Indigestion Tablets.."        },
                             { "Charged Battery",      "..A Charged Battery.."               },
                             { "Flat Battery",         "..A Flat Battery.."                  },
                             { "Digital Camera",       "..A Digital Camera.."                },
                             { "Printed Photo",        "..A Photo Of An Empty Room.."        },
                             { "Whackin' Great Bomb",  "..A Whackin' Great Bomb.."           },
                             { "Teleporter Watch",     "..An Alien Teleporter Watch.." },
                             { "Scotch mist?",         "" } };

GLuint tex[TEX_LAST];
Uint8 fgmap[30240], bgmap[3840], dmap[280], tmap[30240];

struct btex bt[256];

extern struct btex sprt1[], psprt1[], sprtg[];
extern struct btex sprt2[], psprt2[];
extern struct btex sprt3[], psprt3[];
extern struct btex sprt4[], psprt4[];
extern struct btex sprt5[], psprt5[];
struct btex *psprs, *sprs, *ps;

extern struct lift lifts1[], lifts2[], lifts3[], lifts4[], lifts5[];
struct lift *lifts;

extern struct spring springs1[], springs2[], springs3[], springs4[], springs5[];
struct spring *springs;

int num_nthings, num_bnthings, num_pthings, num_bpthings, num_gthings, num_bgthings;
struct thingy *bn_things[MAX_THINGIES]; // Normal things, behind the foreground
struct thingy *n_things[MAX_THINGIES];  // Normal things
struct thingy *bp_things[MAX_THINGIES]; // Puzzle things, behind the foreground
struct thingy *p_things[MAX_THINGIES];  // Puzzle things
struct thingy *bg_things[MAX_THINGIES]; // Global things, behind the foreground
struct thingy *g_things[MAX_THINGIES];  // Global things

struct star stars[MAX_STARS]; // Incidental stars
int nextstar = 0;

int ibubblepop[] = { 42, 43, 44, 45, 46 };
struct incidental incd[MAX_INCIDENTALS], bincd[MAX_INCIDENTALS];
int nextincd = 0, nextbincd = 0;

int edgecdlist[] = { 11,12,13,13, 14,14,14,15, 15,15,15,15, 15,15,14,14,
                     14,13,13,12, 11 };

//                            FGW  FGH  BGW  BGH  XWRP  YWRP  YOFF  /  /  IMX   IMY   IGX   IGY   SPLIT SPL YSCRTL
struct mapsz mapsizes[] = { { 400,  70,  36,  40, 0x12, 0xff, 0x00, 1, 3, 0x47, 0x0d, 0x50, 0x17,   474, 12, 130 },
                            { 240, 126,  36,  28, 0x10, 0x0e, 0x00, 1, 1, 0x00, 0x70, 0x03, 0x7a, 10000,  0,   0 },
                            { 430,  70,  36,  28, 0x0c, 0x30, 0x22, 1, 3, 0x02, 0x0a, 0x0c, 0x13, 10000,  0,   0 },
                            { 600,  29, 256,  15, 0xff, 0x80, 0x80, 1, 8, 0x49, 0x0c, 0x52, 0x0e, 10000,  0,   0 },
                            { 170, 158,  36,  90, 0x10, 0xff, 0x00, 1, 1, 0x00, 0x15, 0x09, 0x1f, 10000,  0,   0 } };

struct stickyscroll
{
  int miny, maxy, scry;
};

struct stickyscroll stsc1[] = { { -1, } };
struct stickyscroll stsc2[] = { { 1840, 1984, 1784 },
                                { 1376, 1536, 1336 },
                                {  928, 1088,  888 },
                                {  480,  640,  440 },
                                {    0,  204,    0 },
                                { -1, } };
struct stickyscroll stsc3[] = { { -1, } };
struct stickyscroll stsc4[] = { { -1, } };
struct stickyscroll stsc5[] = { { -1, } };
struct stickyscroll *stsc;

struct mapsz *mapi;

int fgx, fgy, bgx, bgy;

struct what_is_giddy_doing gid;
struct what_is_everyone_else_doing stuff;

extern Sint16 sintab[];
extern Uint8 tvborders[];
extern int hoptab[];

extern struct thingy n_things1[], g_things1[], p_things1[];
extern struct thingy n_things2[], g_things2[], p_things2[];
extern struct thingy n_things3[], g_things3[], p_things3[];
extern struct thingy n_things4[], g_things4[], p_things4[];
extern struct thingy n_things5[], g_things5[], p_things5[];

int ibstate=0, ibalpha, ibwait=0, ibitem = -1;
float ibscale, ibrot;
float ibw[3] = {0.0f,0.0f,0.0f};
float ibtw[3] = {0.0f,0.0f,0.0f};
char *ibt[3] = { NULL, NULL, NULL };
int ibpos, ibadd=0, iboff, ibdest;

extern int bangframes[];
extern int trd_x, trd_y;

BOOL considertardis;

extern BOOL audioavailable;

void giddy_say( char *what )
{
  Uint8 *s, *d;
  int i, j, k, x, y, gx, gy, gw, gh;

  // First print the text
  memset( &texts[164*256*4], 0, 92*256*4 );
  d = &texts[(170*256+6)*4];

  gx = 0;
  gy = 0;
  gw = 0;
  gh = 6;

  for( i=0; what[i]; i++ )
  {
    if( what[i] == '\n' )
    {
      gx = 0;
      gy += 8;
      gh += 8;
      d = &texts[((170+gy)*256+6)*4];
      continue;
    }

    j = (what[i]-32)*6;

    x = j%192;
    y = ((j/192)*8+76)*256;

    s = &texts[(y+x)*4];
    for( y=0; y<(8*256*4); y+=(256*4) )
    {
      for( x=0; x<24; x++ )
        d[y+x] = s[y+x];
    }
    d += 24;
    gx += 6;
    if( gx > gw ) gw = gx;
  }

  // Do the top and bottom
  i = (164*256+6)*4;
  j = ((170+gh+6)*256+6)*4;
  for( x=0; x<gw; x++ )
  {
    for( y=1; y<6; y++ )
    {
      texts[i+0+1024*y] = 0xff; // R
      texts[i+1+1024*y] = 0xff; // G
      texts[i+2+1024*y] = 0xff; // B
      texts[i+3+1024*y] = 0xff; // A
      texts[j+0-1024*y] = 0xff; // R
      texts[j+1-1024*y] = 0xff; // G
      texts[j+2-1024*y] = 0xff; // B
      texts[j+3-1024*y] = 0xff; // A
    }
    texts[i++] =    0; texts[j++] =    0; // R
    texts[i++] =    0; texts[j++] =    0; // G
    texts[i++] =    0; texts[j++] =    0; // B
    texts[i++] = 0xff; texts[j++] = 0xff; // A
  }

  // Do the left and right
  i = (170*256)*4;
  j = (170*256+gw+11)*4;
  for( y=0; y<=gh; y++ )
  {
    for( x=1; x<6; x++ )
    {
      texts[i+0+4*x] = 0xff; // R
      texts[i+1+4*x] = 0xff; // G
      texts[i+2+4*x] = 0xff; // B
      texts[i+3+4*x] = 0xff; // A
      texts[j+0-4*x] = 0xff; // R
      texts[j+1-4*x] = 0xff; // G
      texts[j+2-4*x] = 0xff; // B
      texts[j+3-4*x] = 0xff; // A
    }
    texts[i+0] =    0; texts[j+0] =    0; // R
    texts[i+1] =    0; texts[j+1] =    0; // G
    texts[i+2] =    0; texts[j+2] =    0; // B
    texts[i+3] = 0xff; texts[j+3] = 0xff; // A
    i+=1024;
    j+=1024;
  }

  // Do the corners
  k = (164*256+gw+6)*4;
  j = 164*256*4;
  i = 128*256*4;
  gy = (gh+7)*256*4;
  for( y=0; y<6; y++ )
  {
    for( x=0; x<24; x++ )
    {
      texts[j+gy] = gsprites[i+(10*256*4)];     // bottom left
      texts[k+gy] = gsprites[i+((10*256+8)*4)]; // bottom right
      texts[k++]  = gsprites[i+(8*4)];          // top right
      texts[j++]  = gsprites[i++];              // top left
    }
    i += (256*4)-24;
    j += (256*4)-24;
    k += (256*4)-24;
  }

  // Do the stalk
  if( !gid.flipped )
  {
    i = (124*256+39)*4;
    if( gw > 48 )
      j = ((164+gh+12)*256+(gw/2)+18)*4;
    else
      j = ((164+gh+12)*256+(gw/2)-8)*4;
    for( y=0; y<13; y++ )
    {
      for( x=0; x<56; x++ )
        texts[j+x] = gsprites[i+x];
      i += 256*4;
      j += 256*4;
    }
  } else {
    i = (124*256+39)*4;
    if( gw > 48 )
      j = ((164+gh+12)*256+(gw/2)-5)*4;
    else
      j = ((164+gh+12)*256+(gw/2)+8)*4;
    for( y=0; y<13; y++ )
    {
      for( x=0; x<56; x+=4 )
      {
        texts[j-x+0] = gsprites[i+x+0];
        texts[j-x+1] = gsprites[i+x+1];
        texts[j-x+2] = gsprites[i+x+2];
        texts[j-x+3] = gsprites[i+x+3];
      }
      i += 256*4;
      j += 256*4;
    }
  }

  gid.speakw = gw + 12;
  gid.speakh = gh + 25;
  gid.speakfw = ((float)gw+12)/256.0f;
  gid.speakfh = ((float)gh+25)/256.0f;
  gid.speakc = 140;
  gid.speaka = 180.0f;
  gid.speaksc = 0.1f;
  gid.speakstate = 1;
  if( gid.speakw >= 80 )
  {
    gid.speakxo = 0;
  } else {
    gid.speakxo = (gid.flipped) ? -16 : 16;
  }
  
  strsneedupdate = TRUE;
}

void animate_giddyspeak( void )
{
  BOOL goon;

  switch( gid.speakstate )
  {
    case 1:
      goon = TRUE;
      if( gid.speaka > 0.0f )
      {
        gid.speaka -= 12.0f;
        if( gid.speaka < 0.0f ) gid.speaka = 0.0f;
        goon = FALSE;
      }
      if( gid.speaksc < 1.0f )
      {
        gid.speaksc += 0.06f;
        if( gid.speaksc > 1.0f ) gid.speaksc = 1.0f;
        goon = FALSE;
      }
      if( goon ) gid.speakstate++;
      break;
    
    case 2:
      if( gamepause )
      {
        gid.speakc = 0;
        break;
      }

      if( gid.speakc > 0 )
      {
        gid.speakc--;
        break;
      }

      gid.speakstate++;
      break;
    
    case 3:
      goon = TRUE;
      if( gid.speaka < 180.0f )
      {
        gid.speaka += 12.0f;
        if( gid.speaka > 180.0f ) gid.speaka = 180.0f;
        goon = FALSE;
      }
      if( gid.speaksc > 0.1f )
      {
        gid.speaksc -= 0.06f;
        if( gid.speaksc < 0.1f ) gid.speaksc = 0.1f;
        goon = FALSE;
      }
      if( goon ) gid.speakstate = 0;
      break;
  }
}

void set_quad_tristrip( GLfloat *arr, GLfloat left, GLfloat top, GLfloat right, GLfloat bottom )
{
  arr[0] = arr[4] = left;
  arr[1] = arr[3] = top;
  arr[2] = arr[6] = right;
  arr[5] = arr[7] = bottom;  
}

void set_quad_tri( GLfloat *arr, GLfloat left, GLfloat top, GLfloat right, GLfloat bottom )
{
  arr[0] = arr[4] = arr[6] = left;
  arr[1] = arr[3] = arr[9] = top;
  arr[2] = arr[8] = arr[10] = right;
  arr[5] = arr[7] = arr[11] = bottom;
}

void (*set_texc_tristrip)( GLfloat *, GLfloat, GLfloat, GLfloat, GLfloat );
void (*set_texc_tri)( GLfloat *, GLfloat, GLfloat, GLfloat, GLfloat );

void bodge_texc_tristrip( GLfloat *arr, GLfloat left, GLfloat top, GLfloat right, GLfloat bottom )
{
  GLfloat sidetrim = 0.00025f;

  if( left > right )
  {
    sidetrim = -sidetrim;
  }

  arr[0] = arr[4] = left+sidetrim;
  arr[1] = arr[3] = top+0.00025f;
  arr[2] = arr[6] = right-sidetrim;
  arr[5] = arr[7] = bottom-0.00025f;  
}

void bodge_texc_tri( GLfloat *arr, GLfloat left, GLfloat top, GLfloat right, GLfloat bottom )
{
  GLfloat sidetrim = 0.0005f;

  if( left > right )
  {
    sidetrim = -sidetrim;
  }

  arr[0] = arr[4] = arr[6] = left+sidetrim;
  arr[1] = arr[3] = arr[9] = top+0.0005f;
  arr[2] = arr[8] = arr[10] = right-sidetrim;
  arr[5] = arr[7] = arr[11] = bottom-0.0005f;
}

void render_giddyspeak( void )
{
  float tl, tt, tr, tb;
  float vl, vt, vr, vb;
  GLfloat vtx[4*2], txq[4*2];

  if( gid.speakstate == 0 ) return;

  vl = -gid.speakw/2.0f;    tl =          0.0f;
  vt = -(gid.speakh+20.0f); tt = 164.0f/256.0f;
  vr = gid.speakw/2.0f;     tr =   gid.speakfw;
  vb = -20.0f;              tb = 164.0f/256.0f + gid.speakfh;

  glBindTexture( GL_TEXTURE_2D, tex[TEXTTEX] );
  glLoadIdentity();
  glTranslatef( gid.px-fgx, gid.py-fgy, 0.0f );
  glRotatef( gid.speaka, 0.0f, 0.0f, 1.0f );
  glScalef( gid.speaksc, gid.speaksc, 1.0f );

  set_quad_tristrip( vtx, vl+gid.speakxo, vt, vr+gid.speakxo, vb );
  set_texc_tristrip( txq, tl, tt, tr, tb );
  
  glColor4ub( 255, 255, 255, 255 );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void set_ibstr( int which, char *txt )
{
  Uint8 *s, *d;
  int i, j, x, y;

  ibt[which] = txt;
  if( !txt )
  {
    ibw[which] = 0.0f;
    return;
  }

  d = &texts[((which*11)+128)*256*4];
  for( i=0; txt[i]; i++ )
  {
    j = (txt[i]-32)*8;

    x = j&255;
    y = ((j/256)*10+46)*256;

    s = &texts[(y+x)*4];
    for( y=0; y<(10*256*4); y+=(256*4) )
    {
      for( x=0; x<32; x++ )
        d[y+x] = s[y+x];
    }
    d += 32;
  } 

  ibw[which] = (float)i*8;
  ibtw[which] = ibw[which] / 256.0f;
  strsneedupdate = TRUE;
}

void update_ibstrs( void )
{
  if( !strsneedupdate ) return;
  strsneedupdate = FALSE;
  glBindTexture( GL_TEXTURE_2D, tex[TEXTTEX] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );
}

void animate_invbox( void )
{
  BOOL animdone;
  switch( ibstate )
  {
    case 0:
      gid.usemode = 0;
      ibalpha = 0;
      ibrot   = 180.0f;
      ibscale = 0.2f;
      return;
    
    case 1:
      gid.usemode = 1;
    case 4:
      animdone = TRUE;
      if( ibalpha < 255 )
      {
        ibalpha+=14;
        if( ibalpha > 255 ) ibalpha = 255;
        animdone = FALSE;
      }
      if( ibrot < 360.0f )
      {
        ibrot += 8.0f;
        if( ibrot > 360.0f )
          ibrot = 360.0f;
        animdone = FALSE;
      }
      if( ibscale < 1.0f )
      {
        ibscale += 0.035f;
        if( ibscale > 1.0f )
          ibscale = 1.0f;
        animdone = FALSE;
      }
      if( animdone ) ibstate++;
      break;

    case 2:
      gid.usemode = 1;
      break;

    case 5:
      if( ibwait > 0 )
      {
        ibwait--;
        break;
      }
      ibdest = -81-ibw[1];
      if( ibadd == 0 ) ibadd = -4;
      ibwait = 15;
      ibstate++;
      break;
    
    case 6:
      if( ibwait > 0 )
      {
        ibwait--;
        break;
      }
      ibstate++;
      break;

    case 3:
      gid.usemode = 0;
    case 7:
      animdone = TRUE;
      if( ibalpha > 0 )
      {
        ibalpha-=14;
        if( ibalpha < 0 ) ibalpha = 0;
        animdone = FALSE;
      }
      if( ibrot < 520.0f )
      {
        ibrot += 8.0f;
        if( ibrot > 520.0f )
          ibrot = 520.0f;
        animdone = FALSE;
      }
      if( ibscale > 0.2f )
      {
        ibscale -= 0.035f;
        if( ibscale < 0.2f )
          ibscale = 0.2f;
        animdone = FALSE;
      }
      if( animdone )
      {
        ibstate = 0;
        ibalpha = 0;
        ibrot   = 180.0f;
        ibscale = 0.2f;
      }
      break;
  }
}

void reset_infos( void )
{
  int i;
  for( i=0; infos1[i].x!=-1; i++ )
    infos1[i].active = infos1[i].iactive;
  for( i=0; infos2[i].x!=-1; i++ )
    infos2[i].active = infos2[i].iactive;
  for( i=0; infos3[i].x!=-1; i++ )
    infos3[i].active = infos3[i].iactive;
  for( i=0; infos4[i].x!=-1; i++ )
    infos4[i].active = infos4[i].iactive;
  for( i=0; infos5[i].x!=-1; i++ )
    infos5[i].active = infos5[i].iactive;
}

void check_infos( void )
{
  int i;
  winfo = -1;
  for( i=0; infos[i].x!=-1; i++ )
  {
    if( !infos[i].active ) continue;
    if( (gid.px>=infos[i].x) &&
        (gid.px<(infos[i].x+infos[i].w)) &&
        (gid.py>=infos[i].y) &&
        (gid.py<(infos[i].y+infos[i].h)) )
    {
      winfo = i;
      if( ( clevel == 4 ) && ( winfo == 0 ) )
        triggermrtbubble();

      if( ( clevel == 4 ) && ( winfo == 5 ) )
        triggerbuilderbubble();

      if( ( clevel == 3 ) && ( winfo == 1 ) )
        triggermuldoonandskellybubble();

      if( ( clevel == 5 ) && ( winfo == 5 ) )
        triggercyclingalienbubble();
      return;
    }
  }
}

void reset_thingy_array( struct thingy * t )
{
  int i;
  for( i=0; t[i].ix!=-1; i++ )
  {
    t[i].active = t[i].iactive;
    t[i].x      = t[i].ix;
    t[i].y      = t[i].iy;
    t[i].frame  = 0;
    t[i].framecount = 0;
  }
}

void reset_thingies( void )
{
  reset_thingy_array( n_things1 );
  reset_thingy_array( p_things1 );
  reset_thingy_array( g_things1 );
  reset_thingy_array( n_things2 );
  reset_thingy_array( p_things2 );
  reset_thingy_array( g_things2 );
  reset_thingy_array( n_things3 );
  reset_thingy_array( p_things3 );
  reset_thingy_array( g_things3 );
  reset_thingy_array( n_things4 );
  reset_thingy_array( p_things4 );
  reset_thingy_array( g_things4 );
  reset_thingy_array( n_things5 );
  reset_thingy_array( p_things5 );
  reset_thingy_array( g_things5 );
}

void init_thingies( int levn )
{
  int i;
  struct thingy *nt, *gt, *pt;

  switch( levn )
  {
    case 1: nt = &n_things1[0]; gt = &g_things1[0]; pt = &p_things1[0]; break;
    case 2: nt = &n_things2[0]; gt = &g_things2[0]; pt = &p_things2[0]; break;
    case 3: nt = &n_things3[0]; gt = &g_things3[0]; pt = &p_things3[0]; break;
    case 4: nt = &n_things4[0]; gt = &g_things4[0]; pt = &p_things4[0]; break;
    case 5: nt = &n_things5[0]; gt = &g_things5[0]; pt = &p_things5[0]; break;
    default: return;
  }

  num_bnthings = 0;
  num_bpthings = 0;
  num_bgthings = 0;
  num_nthings = 0;
  num_pthings = 0;
  num_gthings = 0;
 
  for( i=0; nt[i].ix!=-1; i++ )
    if( nt[i].flags & THF_BEHIND )
      bn_things[num_bnthings++] = &nt[i];
    else
      n_things[num_nthings++] = &nt[i];

  for( i=0; pt[i].ix!=-1; i++ )
    if( pt[i].flags & THF_BEHIND )
      bp_things[num_bpthings++] = &pt[i];
    else
      p_things[num_pthings++] = &pt[i];

  for( i=0; gt[i].ix!=-1; i++ )
    if( gt[i].flags & THF_BEHIND )
      bg_things[num_bgthings++] = &gt[i];
    else
      g_things[num_gthings++] = &gt[i];
}

void thingy_bounds_wh( struct thingy *t, struct btex *sl, int *x, int *y, int *w, int *h )
{
  struct btex *s;

  s = &sl[t->frames[t->frame]];
  if( t->flags & THF_CENTREPOS )
  {
    *x = t->x - s->hfw;
    *y = t->y - s->hfh;
    *w = s->fw;
    *h = s->fh;
    return;
  }

  *x = t->x;
  *y = t->y;
  *w = s->fw;
  *h = s->fh;
}

void thingy_bounds( struct thingy *t, struct btex *sl, int *left, int *top, int *right, int *bot )
{
  struct btex *s;

  s = &sl[t->frames[t->frame]];
  if( t->flags & THF_CENTREPOS )
  {
    *left  = t->x - s->hfw;
    *top   = t->y - s->hfh;
    *right = t->x + s->hfw;
    *bot   = t->y + s->hfh;
    return;
  }

  *left  = t->x;
  *top   = t->y;
  *right = t->x+s->fw;
  *bot   = t->y+s->fh;
}

BOOL load_packed( char *filename, Uint8 *buf, int len )
{
  int s,d,i,j;
  Uint8 *pb;
  int plen;
  FILE *f;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fseek( f, 0, SEEK_END );
  plen = ftell( f );
  fseek( f, 0, SEEK_SET );

  pb = malloc( plen );
  if( !pb )
  {
    fclose( f );
    return FALSE;
  }

  fread( pb, plen, 1, f );
  fclose( f );

  s=d=0;
  while( s < plen )
  {
    if( d >= len ) break;

    if( pb[s] == 0xff )
    {
      s++;
      i = pb[s++];
      j = pb[s++];
      while( j > 0 )
      {
        if( d >= len ) { free( pb ); return TRUE; }
        buf[d++] = i;
        j--;
      }
      continue;
    }

    buf[d++] = pb[s++];
  }

  free( pb );
  return TRUE;
};

BOOL load_blocks( char *filename )
{
  FILE *f;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( blocks, 16*16*4, 256, f );
  fclose( f );

  glBindTexture( GL_TEXTURE_2D, tex[BLOCKTEX]);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, blocks );
  
  return TRUE;
}

BOOL load_raw( char *filename, unsigned char *raw, int w, int h, int txn )
{
  FILE *f;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( raw, w*h*4, 1, f );
  fclose( f );

  glBindTexture( GL_TEXTURE_2D, tex[txn]);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
  
  return TRUE;
}

BOOL load_sprites( char *filename )
{
  FILE *f;
  int i, j;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( sprites, 256*256*4, 1, f );
  fclose( f );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = sprites[i  ]+80; wsprites[i  ] = j>255 ? 255 : j;
    j = sprites[i+1]+80; wsprites[i+1] = j>255 ? 255 : j;
    j = sprites[i+2]+80; wsprites[i+2] = j>255 ? 255 : j;
    wsprites[i+3] = sprites[i+3];
  }

  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprites );

  glBindTexture( GL_TEXTURE_2D, tex[WSPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, wsprites );

  return TRUE;
}

BOOL load_puzzlesprites( char *filename )
{
  FILE *f;
  int i, j;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( psprites, 256*256*4, 1, f );
  fclose( f );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = psprites[i  ]+80; wpsprites[i  ] = j>255 ? 255 : j;
    j = psprites[i+1]+80; wpsprites[i+1] = j>255 ? 255 : j;
    j = psprites[i+2]+80; wpsprites[i+2] = j>255 ? 255 : j;
    wpsprites[i+3] = psprites[i+3];
  }

  glBindTexture( GL_TEXTURE_2D, tex[PSPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, psprites );

  glBindTexture( GL_TEXTURE_2D, tex[WPSPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, wpsprites );

  return TRUE;
}

BOOL load_globalsprites( void )
{
  FILE *f;
  int i, j;
#ifdef __android__
	if (use_alt_basepath)
		f = fopen( BASEPATH_ALT"hats/sprites.bin", "rb" );
	else
#endif
		f = fopen( BASEPATH"hats/sprites.bin", "rb" );

  if( !f ) return FALSE;

  fread( gsprites, 256*256*4, 1, f );
  fclose( f );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = gsprites[i  ]+80; wgsprites[i  ] = j>255 ? 255 : j;
    j = gsprites[i+1]+80; wgsprites[i+1] = j>255 ? 255 : j;
    j = gsprites[i+2]+80; wgsprites[i+2] = j>255 ? 255 : j;
    wgsprites[i+3] = gsprites[i+3];
  }

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, gsprites );

  glBindTexture( GL_TEXTURE_2D, tex[WGSPRITEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, wgsprites );

  memset( &texts[128*256*4], 0, 128*256*4 );
#ifdef __android__
	if (use_alt_basepath)
		f = fopen( BASEPATH_ALT"hats/texts.bin", "rb" );
	else
#endif
		f = fopen( BASEPATH"hats/texts.bin", "rb" );

  if( !f ) return FALSE;

  fread( texts, 256*128*4, 1, f );
  fclose( f );

  glBindTexture( GL_TEXTURE_2D, tex[TEXTTEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (viewh==320) ? GL_LINEAR : GL_NEAREST ); // Bodge for Pre (and maybe pixi?)
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texts );

  return TRUE;
}

void initstars( void )
{
  int i;

  for( i=0; i<MAX_STARS; i++ )
    stars[i].frame = 23;
}

void initincidentals( void )
{
  int i;

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    incd[i].numframes = 0;
    incd[i].frame = 0;
    bincd[i].numframes = 0;
    bincd[i].frame = 0;
  }
}

BOOL startincidental( int x, int y, int xm, int ym, int wtex, int *frames, int numframes, int speed )
{
  struct btex *sl, *s;

  switch( wtex )
  {
    case SPRITEX:  sl = sprs;  break;
    case PSPRITEX: sl = psprs; break;
    case GSPRITEX: sl = sprtg; break;
    default: return FALSE;
  }

  // Is it even on the screen?
  s = &sl[frames[0]];
  if( ( (x+s->hfw) < (fgx-64) ) ||
      ( (x-s->hfw) > (fgx+384) ) ||
      ( (y+s->hfh) < (fgy-64) ) ||
      ( (y-s->hfh) > (fgy+264) ) )
    return FALSE;

  incd[nextincd].s          = sl;
  incd[nextincd].x          = x;
  incd[nextincd].y          = y;
  incd[nextincd].xm         = xm;
  incd[nextincd].ym         = ym;
  incd[nextincd].numframes  = numframes;
  incd[nextincd].frame      = 0;
  incd[nextincd].framecount = 0;
  incd[nextincd].framespeed = speed;
  incd[nextincd].frames     = frames;
  incd[nextincd].wtex       = wtex;
  nextincd = (nextincd+1)%MAX_INCIDENTALS;
  return TRUE;
}

BOOL startbgincidental( int x, int y, int xm, int ym, int wtex, int *frames, int numframes, int speed )
{
  struct btex *sl, *s;

  switch( wtex )
  {
    case SPRITEX:  sl = sprs;  break;
    case PSPRITEX: sl = psprs; break;
    case GSPRITEX: sl = sprtg; break;
    default: return FALSE;
  }

  // Is it even on the screen?
  s = &sl[frames[0]];
  if( ( (x+s->hfw) < (bgx-64) ) ||
      ( (x-s->hfw) > (bgx+384) ) ||
      ( (y+s->hfh) < (bgy-64) ) ||
      ( (y-s->hfh) > (bgy+264) ) )
    return FALSE;

  bincd[nextbincd].s          = sl;
  bincd[nextbincd].x          = x;
  bincd[nextbincd].y          = y;
  bincd[nextbincd].xm         = xm;
  bincd[nextbincd].ym         = ym;
  bincd[nextbincd].numframes  = numframes;
  bincd[nextbincd].frame      = 0;
  bincd[nextbincd].framecount = 0;
  bincd[nextbincd].framespeed = speed;
  bincd[nextbincd].frames     = frames;
  bincd[nextbincd].wtex       = wtex;
  nextbincd = (nextbincd+1)%MAX_INCIDENTALS;
  return TRUE;
}

void setliftdeltas( int ln )
{
  int fx,fy,tx,ty;

  if( lifts[ln].hdr.fromstop == -1 )
  {
    lifts[ln].hdr.cx = lifts[ln].hdr.stops[0]<<8;
    lifts[ln].hdr.cy = lifts[ln].hdr.stops[1]<<8;
    return;
  }

  fx = lifts[ln].hdr.stops[lifts[ln].hdr.fromstop*2];
  fy = lifts[ln].hdr.stops[lifts[ln].hdr.fromstop*2+1];
  tx = lifts[ln].hdr.stops[lifts[ln].hdr.tostop*2];
  ty = lifts[ln].hdr.stops[lifts[ln].hdr.tostop*2+1];
  lifts[ln].hdr.cx = fx<<8;
  lifts[ln].hdr.cy = fy<<8;
  lifts[ln].hdr.dx = ((tx-fx)<<8)/lifts[ln].hdr.speed;
  lifts[ln].hdr.dy = ((ty-fy)<<8)/lifts[ln].hdr.speed;
}

int liftloopchan=-1, liftlooplift=-1;
void initlifts( void )
{
  int i;

  liftloopchan = -1;
  liftlooplift = -1;

  for( i=0; lifts[i].hdr.numstops!=-1; i++ )
  {
    if( lifts[i].hdr.type == LT_TRIGGER )
      lifts[i].hdr.fromstop = -1;
    else
      lifts[i].hdr.fromstop = 0;
    lifts[i].hdr.tostop = 1;
    setliftdeltas( i );
    lifts[i].hdr.timeout = lifts[i].hdr.itimeout;
    lifts[i].hdr.scale = 1.0f;
    lifts[i].hdr.dip = 0;

    switch( lifts[i].hdr.wtex )
    {
      case SPRITEX:  lifts[i].s = &sprs[lifts[i].hdr.sprite];  lifts[i].src = sprites;  break;
      case PSPRITEX: lifts[i].s = &psprs[lifts[i].hdr.sprite]; lifts[i].src = psprites; break;
      case GSPRITEX: lifts[i].s = &sprtg[lifts[i].hdr.sprite]; lifts[i].src = gsprites; break;
      default:       lifts[i].s = NULL;                    lifts[i].src = NULL;     break;
    }
  }
}

void initsprings( void )
{
  int i;

  for( i=0; springs[i].x!=-1; i++ )
  {
    springs[i].frame = 0;
    springs[i].rtime = 0;
    switch( springs[i].wtex )
    {
      case SPRITEX:  springs[i].sl = sprs;  springs[i].src = sprites;  break;
      case PSPRITEX: springs[i].sl = psprs; springs[i].src = psprites; break;
      case GSPRITEX: springs[i].sl = sprtg; springs[i].src = gsprites; break;
      default:       springs[i].sl = NULL;  springs[i].src = NULL;     break;
    }
  }
}

BOOL load_level( int levn, BOOL setgiddydir )
{
  char mkstr[256];
  int specialstart, x;
  BOOL waswarp = FALSE;

  stopallchannels();

  considertardis = FALSE;

  specialstart = levn & 0x8000;
  levn &= 0x7fff;

  mapi =  &mapsizes[levn-1];
  switch( levn )
  {
    case 1:
      sprs    = &sprt1[0];
      psprs   = &psprt1[0];
      lifts   = &lifts1[0];
      infos   = &infos1[0];
      springs = &springs1[0];
      stsc    = &stsc1[0];
      break;

    case 2:
      sprs    = &sprt2[0];
      psprs   = &psprt2[0];
      lifts   = &lifts2[0];
      infos   = &infos2[0];
      springs = &springs2[0];
      stsc    = &stsc2[0];
      break;

    case 3:
      sprs    = &sprt3[0];
      psprs   = &psprt3[0];
      lifts   = &lifts3[0];
      infos   = &infos3[0];
      springs = &springs3[0];
      stsc    = &stsc3[0];
      break;

    case 4:
      sprs    = &sprt4[0];
      psprs   = &psprt4[0];
      lifts   = &lifts4[0];
      infos   = &infos4[0];
      springs = &springs4[0];
      stsc    = &stsc4[0];
      break;

    case 5:
      sprs    = &sprt5[0];
      psprs   = &psprt5[0];
      lifts   = &lifts5[0];
      infos   = &infos5[0];
      springs = &springs5[0];
      stsc    = &stsc5[0];
      break;
  }

  initstars();
  initlifts();
  initsprings();
  initincidentals();
#ifdef __android__
  if (use_alt_basepath)
	  sprintf( mkstr, BASEPATH_ALT"onion%d/blocks.bin", levn );
  else
#endif
	  sprintf( mkstr, BASEPATH"onion%d/blocks.bin", levn );

  if( !load_blocks( mkstr ) ) return FALSE;

#ifdef __android__
	if (use_alt_basepath)
		sprintf( mkstr, BASEPATH_ALT"onion%d/sprites.bin", levn );
	else
#endif
		sprintf( mkstr, BASEPATH"onion%d/sprites.bin", levn );

  if( !load_sprites( mkstr ) ) return FALSE;

#ifdef __android__
	if (use_alt_basepath)
		sprintf( mkstr, BASEPATH_ALT"onion%d/psprites.bin", levn );
	else
#endif
		sprintf( mkstr, BASEPATH"onion%d/psprites.bin", levn );

  if( !load_puzzlesprites( mkstr ) ) return FALSE;

#ifdef __android__
	if (use_alt_basepath)
		sprintf( mkstr, BASEPATH_ALT"onion%d/TRANSTAB.PAK", levn );
	else
#endif
		sprintf( mkstr, BASEPATH"onion%d/TRANSTAB.PAK", levn );

  if( !load_packed( mkstr, blktrans, 512 ) ) return FALSE;

#ifdef __android__
	if (use_alt_basepath)
		sprintf( mkstr, BASEPATH_ALT"onion%d/MAP1.PAK", levn );
	else
#endif
		sprintf( mkstr, BASEPATH"onion%d/MAP1.PAK", levn );

  if( !load_packed( mkstr, fgmap, (levn!=4) ? mapi->fgw*mapi->fgh : mapi->fgw*(mapi->fgh-1) ) ) return FALSE;

  if( levn == 4 )
  {
    for( x=mapi->fgw*(mapi->fgh-1); x<(mapi->fgw*mapi->fgh) - 308; x++ )
    {
      fgmap[x] = 249;
    }

    for( ; x<mapi->fgw*mapi->fgh; x++ )
    {
      fgmap[x] = 220 + (x&1);
    }
  }
#ifdef __android__
  if (use_alt_basepath)
  {
	  sprintf( mkstr, BASEPATH_ALT"onion%d/MAP2.PAK", levn );
	  if( !load_packed( mkstr, bgmap, mapi->bgw*mapi->bgh ) ) return FALSE;

	  sprintf( mkstr, BASEPATH_ALT"onion%d/MAP3.PAK", levn );
	  if( !load_packed( mkstr, dmap, 20*14 ) ) return FALSE;

	  if( !load_raw( BASEPATH_ALT"hats/joystick.raw", stick, 64, 64, STICKTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH_ALT"hats/joyarrowleft.raw", stick, 64, 64, STICKLTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH_ALT"hats/joyarrowright.raw", stick, 64, 64, STICKRTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH_ALT"hats/jumpbtn.raw", stick, 64, 64, JUMPTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH_ALT"hats/usebtn.raw", stick, 64, 64, USETEX ) ) return FALSE;
	  if( !load_raw( BASEPATH_ALT"hats/hintbtn.raw", stick, 64, 64, HINTTEX ) ) return FALSE;

	  //This should be done somewhere else
#ifdef __demo__
	  if( !load_raw( BASEPATH_ALT"hats/demo_end.raw", sprites, 256, 256, DEMOENDTEX ) ) return FALSE;
#endif

	  sprintf( mkstr, BASEPATH_ALT"onion%d/tune.mod", levn );
  }
  else
#endif
  {
	  sprintf( mkstr, BASEPATH"onion%d/MAP2.PAK", levn );
	  if( !load_packed( mkstr, bgmap, mapi->bgw*mapi->bgh ) ) return FALSE;

	  sprintf( mkstr, BASEPATH"onion%d/MAP3.PAK", levn );
	  if( !load_packed( mkstr, dmap, 20*14 ) ) return FALSE;

	  if( !load_raw( BASEPATH"hats/joystick.raw", stick, 64, 64, STICKTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH"hats/joyarrowleft.raw", stick, 64, 64, STICKLTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH"hats/joyarrowright.raw", stick, 64, 64, STICKRTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH"hats/jumpbtn.raw", stick, 64, 64, JUMPTEX ) ) return FALSE;
	  if( !load_raw( BASEPATH"hats/usebtn.raw", stick, 64, 64, USETEX ) ) return FALSE;
	  if( !load_raw( BASEPATH"hats/hintbtn.raw", stick, 64, 64, HINTTEX ) ) return FALSE;

	  //This should be done somewhere else
#ifdef __demo__
	  if( !load_raw( BASEPATH"hats/demo_end.raw", sprites, 256, 256, DEMOENDTEX ) ) return FALSE;
#endif

	  sprintf( mkstr, BASEPATH"onion%d/tune.mod", levn );
  }
  playtune( mkstr );

  // Make the glow rods deadly
  if( levn == 2 )
  {
    blktrans[194+256] = 0xc;
    blktrans[195+256] = 0xc;
    blktrans[196+256] = 0xc;
  }

  init_thingies( levn );
  initenemies( enemies[levn-1] );
  initenemies( enemiesb[levn-1] );

  fgx = mapi->imapx * 16;
  fgy = mapi->imapy * 16;

  gid.px        = mapi->igidx * 16;
  gid.x         = gid.px<<8;
  gid.py        = mapi->igidy * 16;
  gid.y         = gid.py<<8;
  gid.movedisp  = 0;

  if( specialstart != 0 )
  {
    switch( levn )
    {
      case 1:
        gid.x = 6272<<8;
        gid.px = 6272;
        gid.y = 404<<8;
        gid.py = 404;
        fgx = gid.px-160;
        fgy = gid.py-SCROFF;
        break;
      
      case 4:
        gid.x = 3008<<8;
        gid.px = 3008;
        gid.y = 416<<8;
        gid.py = 416;
        fgx = gid.px-160;
        fgy = gid.py-SCROFF;
        break;
    }
  }

  gid.def = 2;
  if( setgiddydir )
  {
    gid.flipped = FALSE; // in original, it was bit 15 in giddef
    if( ( mapi->igidx - mapi->imapx ) >= 10 )
      gid.flipped = TRUE;
  }

  if( gid.watchwarp )
    waswarp = TRUE;

  gid.jumpcount  = 0;
  gid.jumping    = 0;
  gid.jumpstep   = 6;
  gid.jump       = FALSE;
  gid.jumplatch  = FALSE;
  gid.dazed      = FALSE;
  gid.usemode    = FALSE;
  gid.allowjump  = TRUE;
  gid.nogmxgrav  = FALSE;
  gid.teleport   = FALSE;
  gid.stargatewarp= FALSE;
  gid.watchwarp  = FALSE;
  gid.wwob       = 0.0f;
  gid.onlift     = -1;
  gid.speakc     = 0;
  gid.speakstate = 0;
  gid.rot        = 0;

  stuff.bossmode = FALSE;

  clevel = levn;
  llevel = levn;

  switch( levn )
  {
    case 1:
      initslug();
      initeel();
      initsprinkler();
      initwhackinggreatbomb();
      if( waswarp ) incidentalsound( SND_TELEPORT_IN, sfxvol );
      break;
    
    case 2:
      initjunkchute();
      initrecyclotron();
      initsludgemonster();
      initburstpipe();
      inittoxicgas();
      initpluggrabber();
      initprinter();
      break;

    case 3:
      inittardis();
      initmuldoonandskelly();
      initboulder();
      initdragon();
      initballoon();
      initseesaw();
      initfallingblocks();
      initstargate();
      initspecialfade();
      initboulder2();
      inittimerdoor();
      initlockblockzapper();
      initwallsteppingstones();
      initgummachine();
      break;

    case 4:
      initmrt();
      inittardis();
      initphonebox();
      initfactory();
      initbuilder();
      initcrusher();
      initninja();
      initspecialbin();
      break;
    
    case 5:
      inittripledoors();
      initbigassfan();
      initlaserbeam();
      initbigscreen();
      inittpbubs();
      initbiledude();
      initcyclingalien();
      initflashybuttons();
      initeggsterminatorproductionline();
      incidentalsound( SND_TELEPORT_IN, sfxvol );
      break;
  }

  return TRUE;
};

BOOL cheaty_starty( int levn )
{
  switch( levn )
  {
    case 2:
      // Cheaty starty on level 2
      inv[INV_LARD] = 1;                 // Already got the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object

      // Super cheaty
      //inv[INV_BUBBLEGUM] = 1;
      //inv[INV_BOOT] = 1;
      break;
    
    case 4:
      // Cheaty starty on level 4
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_ELECTRICALTOOLKIT] = 1;   // Already got the electrical toolkit
      g_things2[3].active = FALSE;
      gid.fallpuffs = TRUE;
      break;

    case 3:
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      break;
    
    case 0x8004: // Level 4, second time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      inv[INV_DETONATOR] = 1;          // Already got the detonator
      g_things3[2].active = FALSE;
      break;
    
    case 0x8003: // Level 3, second time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      inv[INV_CATAPULT] = 1;           // Already got the catapult
      g_things3[4].active = FALSE;
      break;

    case 0x18004: // Level 4, third time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      inv[INV_ATEAMPHONENO] = 1;
      gid.coins = 231;
      break;

    case 0x18003: // Level 3, third time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
//      strig[ST3_TORCHES_LIT] = 1;
      break;

    case 0x5: // Level 5
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      break;

    case 0x10001: // Level 1, second time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_CD] = 1;                 // Got the CD
      g_things5[7].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      break;

    case 0x8002: // Level 2, second time around
      // Cheaty starty on level 2
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      inv[INV_LUMPOFCARBON] = 1;       // Got the carbon
      g_things1[7].active = FALSE;
      break;

    case 0x20004: // Level 4, fourth time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      inv[INV_LUMPOFCARBON] = 1;       // Got the carbon
      g_things1[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      break;

    case 0x20003: // Level 3, fourth time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_DIAMOND] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      gid.coins = 240;
      inv[INV_COINS] = 1;
      break;

    case 0x10005: // Level 5, second time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      inv[INV_HOSEPIPE] = 1;           // Got the hosepipe
      g_things3[17].active = FALSE;
      inv[INV_INDIGESTIONPILLS] = 1;   // Got the indigestion pills
      g_things3[19].active = FALSE;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      break;

    case 0x20001: // Level 1, third time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      inv[INV_HOSEPIPE] = 1;           // Got the hosepipe
      g_things3[17].active = FALSE;
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      inv[INV_CAMERAWITHPHOTO] = 1;
      break;

    case 0x10002: // Level 2, third time around
      // Cheaty starty on level 2
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      inv[INV_CAMERAWITHPHOTO] = 1;
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      inv[INV_BOOT] = 1;
      break;

    case 0x30004: // Level 4, fifth time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      inv[INV_HARDHAT] = 1;  // got the hard hat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_PRINTEDPHOTO] = 1;
      break;

    case 0x20005: // Level 5, third time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_PRINTEDPHOTO] = 1;
      inv[INV_SPADE] = 1;   // got the spade
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      break;

    case 0x30001: // Level 1, fourth time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_SPADE] = 1;   // got the spade
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      strig[ST5_CCTV_DISABLED] = 1;
      strig[ST5_BATTERY_PLACED] = 1;
      strig[ST5_RED_PRESSED] = 1;
      strig[ST5_GREEN_PRESSED] = 1;
      strig[ST5_BLUE_PRESSED] = 1;
      inv[INV_MIRROR] = 1;
      g_things5[10].active = FALSE;
      break;

    case 0x30005: // Level 5, fourth time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      strig[ST5_CCTV_DISABLED] = 1;
      strig[ST5_BATTERY_PLACED] = 1;
      strig[ST5_RED_PRESSED] = 1;
      strig[ST5_GREEN_PRESSED] = 1;
      strig[ST5_BLUE_PRESSED] = 1;
      inv[INV_MIRROR] = 1;
      g_things5[10].active = FALSE;
      strig[ST1_BOMB_DUG] = 1;
      strig[ST1_BOMB_COLLECTED] = 1;
      inv[INV_BOMB] = 1;
      break;
  }

  return load_level( levn, TRUE );
}

BOOL start_game( SDL_bool load_save_game )
{
  int i;

  gamepause = FALSE;

  gid.coins = 0;
  gid.lives = 3;
  gid.energy = 3;
  gid.dieanim = FALSE;
  gid.spoonedit = FALSE;
  gid.rot = 0.0f;
  gid.jumping = 0;
  gid.flashing = 0;
  gid.onlift = -1;
  gid.onspring = -1;
  gid.fallpuffs = FALSE;
  gid.lockx = -1;
  gid.hardhat = FALSE;
	gid.savedtheday = FALSE;

#ifdef __demo__
	gid.demofinished = FALSE;

	//LOGI("START");
	gid.jumpstep = 6;
	gid.framestep = 0;
	gid.jumping = 0;
	gid.jump = FALSE;
	gid.mainchryoffset = 0xfc00;
#endif

  lastinv = 0;

  reset_thingies();
  reset_infos();

	winfo = -1;
	infobulbwinfo = -1;

  for( i=0; i<ST_LAST; i++ )
    strig[i] = 0;

  for( i=0; i<INV_LAST; i++ )
    inv[i] = 0;

  if( !load_globalsprites() ) return FALSE;

  if( load_save_game && savegame_available )
  {
    int wlev;

    memcpy(&wlev, &savegamebuf[0], sizeof(clevel));
    if( !load_level( wlev, TRUE ) ) return FALSE;
    continue_game();

	if ((gid.px < 0) || (gid.py < 0) ||
		(gid.px >= (mapi->fgw*16)) ||
		(gid.py >= (mapi->fgh*16)))
	{
		fgx = mapi->imapx * 16;
		fgy = mapi->imapy * 16;
		gid.px        = mapi->igidx * 16;
		gid.x         = gid.px<<8;
		gid.py        = mapi->igidy * 16;
		gid.y         = gid.py<<8;
	}

	initstars();
    initsprings();
    initincidentals();
    init_thingies( wlev );
    initenemies( enemies[wlev-1] );
    initenemies( enemiesb[wlev-1] );
    switch( wlev )
    {
      case 1:
        initslug();
        initeel();
        initsprinkler();
        initwhackinggreatbomb();
        break;
      
      case 2:
        initjunkchute();
        initrecyclotron();
        initsludgemonster();
        initburstpipe();
        inittoxicgas();
        initpluggrabber();
        initprinter();
        break;

      case 3:
        inittardis();
        initmuldoonandskelly();
        initboulder();
        initdragon();
        initballoon();
        initseesaw();
        initfallingblocks();
        initstargate();
        initspecialfade();
        initboulder2();
        inittimerdoor();
        initlockblockzapper();
        initwallsteppingstones();
        initgummachine();
        break;

      case 4:
        initmrt();
        inittardis();
        initphonebox();
        initfactory();
        initbuilder();
        initcrusher();
        initninja();
        initspecialbin();
        break;
      
      case 5:
        inittripledoors();
        initbigassfan();
        initlaserbeam();
        initbigscreen();
        inittpbubs();
        initbiledude();
        initcyclingalien();
        initflashybuttons();
        initeggsterminatorproductionline();
        incidentalsound( SND_TELEPORT_IN, sfxvol );
        break;
    }
  }
  else
  {
   // if( !cheaty_starty( 0x10002 ) ) return FALSE;
    if( !load_level( 1, TRUE ) ) return FALSE;
    extratime = 0.0f;
  }

  time( &gamestarttime );

  giddy_say( "Come on, time to wibble." );
  return TRUE;
}

void addthingytoxcolis( struct thingy *t, struct btex *sl, Uint8 *src, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int ccx, ccy;
  struct btex *s;

  if( !t->active ) return;

  if( (t->flags&THF_BLOCKWALK) == 0 )
    return;

  thingy_bounds( t, sl, &tl, &tt, &tr, &tb );

  ccx = cx<<4;
  ccy = cy<<4;

  if( ( tl >= (ccx+32) ) ||
      ( tt >= (ccy+48) ) ||
      ( tr < ccx ) ||
      ( tb < ccy ) )
    return;

  s = &sl[t->frames[t->frame]];

  yp = 0;
  if( tt < ccy ) yp = ccy-tt;
  xl = 0;
  if( tl < ccx ) xl = ccx-tl;

  cpy = 0;
  if( tt > ccy ) cpy = tt-ccy;
  cpxl = 0;
  if( tl > ccx ) cpxl = tl-ccx;

  for( ; yp<s->fh; yp++, cpy++ )
  {
    if( cpy >= 48 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 32 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*32+cpx] = 0xff;
    }
  }  
}

void makecolisareax( int offset )
{
  int mapo, x, y, b, bpx, bpy, bx, by, i;

  mapo = ((gid.y-0xa00)>>12)*mapi->fgw + ((gid.x-offset)>>12);

  for( y=0; y<3; y++ )
  {
    for( x=0; x<2; x++ )
    {
      i = fgmap[y*mapi->fgw+x+mapo];
      b = blktrans[i];
      if( ( blktrans[i+256]&1 ) != 0 ) b = 0;
      
      bpy = (b/16) * 16 * 256 * 4;
      b = (b&15)*16*4+3;
      for( by=0; by<16; by++, bpy+=256*4 )
      {
        for( bx=0,bpx=b; bx<16; bx++, bpx+=4 )
          collisionarea[((y*16)+by)*32+(x*16+bx)] = (blocks[bpy+bpx]&0xff)?0xff:0x00;
      }
    }
  }

  x = (gid.x-offset)>>12;
  y = (gid.y-0xa00)>>12;

  // Add in any solid objects in the area
  for( i=0; i<num_nthings; i++ )
    addthingytoxcolis( n_things[i], sprs, sprites, x, y );
  for( i=0; i<num_bnthings; i++ )
    addthingytoxcolis( bn_things[i], sprs, sprites, x, y );
  for( i=0; i<num_pthings; i++ )
    addthingytoxcolis( p_things[i], psprs, psprites, x, y );
  for( i=0; i<num_bpthings; i++ )
    addthingytoxcolis( bp_things[i], psprs, psprites, x, y );
  for( i=0; i<num_gthings; i++ )
    addthingytoxcolis( g_things[i], sprtg, gsprites, x, y );
  for( i=0; i<num_bgthings; i++ )
    addthingytoxcolis( bg_things[i], sprtg, gsprites, x, y );
}

void addthingytoycolis( struct thingy *t, struct btex *sl, Uint8 *src, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int ccx, ccy;
  Uint8 bm;
  struct btex *s;

  if( !t->active ) return;

  if( gid.mainchryoffset > 0 )
  {
    if( (t->flags&THF_BLOCKJUMP) == 0 )
      return;
  } else {
    if( (t->flags&THF_BLOCKFALL) == 0 )
      return;
  }

  thingy_bounds( t, sl, &tl, &tt, &tr, &tb );

  if( ( (tb-tt) > 8 ) && ( (t->flags&THF_BLOCKWALK) == 0 ) )
    tb = tt+8;

  ccx = cx<<4;
  ccy = cy<<4;

  if( ( tl >= (ccx+48) ) ||
      ( tt >= (ccy+32) ) ||
      ( tr < ccx ) ||
      ( tb < ccy ) )
    return;

  s = &sl[t->frames[t->frame]];
  src += (s->fy*256+s->fx)*4;

  yp = 0;
  if( tt < ccy ) yp = ccy-tt;
  xl = 0;
  if( tl < ccx ) xl = ccx-tl;

  cpy = 0;
  if( tt > ccy ) cpy = tt-ccy;
  cpxl = 0;
  if( tl > ccx ) cpxl = tl-ccx;

  bm = 0xff;
  if( t->flags&THF_CONVEYLEFT  ) bm = 0xfd;
  if( t->flags&THF_CONVEYRIGHT ) bm = 0xfe;

  switch( clevel )
  {
    case 3:
      if( t == p_things[6] ) bm = 0xf8;
      if( t == p_things[20] ) bm = 0xf7;
      break;

    case 4:
      if( t == p_things[39] ) bm = 0xfb;
      if( t == p_things[8] ) bm = 0xf4;
      break;
    
    case 5:
      if( t == p_things[7] ) bm = 0xf5;
      if( t == p_things[5] ) bm = 0xf6;
      if( t == p_things[3] ) bm = 0xfa;
      if( t == n_things[0] ) bm = 0xf9;
      break;
  }

  for( ; yp<s->fh; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*48+cpx] = bm;
    }
  }  
}

void addlifttoycolis( int ln, int cx, int cy, int lx, int ly, int stop )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  Uint8 *src;
  struct btex *s;

  if( gid.mainchryoffset > 0 )
    return;

  if( !lifts[ln].s ) return;

  s = lifts[ln].s;

  tl = lx - s->hfw;
  tt = (ly+lifts[ln].hdr.bobo+lifts[ln].hdr.dip+lifts[ln].hdr.surface) - s->hfh;
  tr = tl + s->fw;
  tb = tt + lifts[ln].hdr.depth;

//  if( ( (tb-tt) > 8 ) && ( (t->flags&THF_BLOCKWALK) == 0 ) )
//    tb = tt+8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  src = &lifts[ln].src[((s->fy+lifts[ln].hdr.surface)*256+s->fx)*4];

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  for( ; yp<lifts[ln].hdr.depth; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
      {
        collisionarea[cpy*48+cpx] = ln+1;
        liftstoparea[cpy*48+cpx]  = stop;
      }
    }
  }  
}

void addspringtoycolis( int sn, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int so[] = { 0, 7, 14, 19 };
  int offs;
  Uint8 *src;
  struct btex *s;

  if( gid.mainchryoffset > 0 )
    return;

  s = &springs[sn].sl[springs[sn].frames[springs[sn].frame]];
  src = &springs[sn].src[(s->fy*256+s->fx)*4];

  tl = springs[sn].x;
  tt = springs[sn].y;
  tr = tl + s->fw;
  tb = tt + 8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  offs = so[springs[sn].frame];
  if( yp < so[springs[sn].frame] )
  {
    offs = so[springs[sn].frame]-yp;
    yp = so[springs[sn].frame];
  }
  cpy += offs;

  for( ; yp<(so[springs[sn].frame]+14); yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*48+cpx] = sn+0x80;
    }
  }  
}

void addtardistoycolis( int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;

  if( ( clevel != 3 ) && ( clevel != 4 ) )
    return;

  if( !considertardis )
    return;

  if( gid.mainchryoffset > 0 )
    return;

  tl = trd_x>>8;
  tt = (trd_y>>8)-8;
  tr = tl + 62;
  tb = tt + 8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  for( ; yp<8; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<62; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      collisionarea[cpy*48+cpx] = 0xfc;
    }
  }  
}

void makecolisareay( int offset, BOOL inclifts, BOOL inctardis )
{
  int mapo, i, j, x, y, b, bpx, bpy, bx, by;

  if( inclifts )
    memset( liftstoparea, 0, 16*16*3*2 );

  mapo = ((gid.y-offset)>>12)*mapi->fgw + ((gid.x-0x1000)>>12);

  for( y=0; y<2; y++ )
  {
    for( x=0; x<3; x++ )
    {
      i = fgmap[y*mapi->fgw+x+mapo];
      b = blktrans[i];
      if( ( gid.mainchryoffset > 0 ) && ( ( blktrans[i+256]&1 ) != 0 ) )
        b = 0;

      bpy = (b/16) * 16 * 256 * 4;
      b = (b&15)*16*4+3;
      for( by=0; by<16; by++, bpy+=256*4 )
      {
        for( bx=0,bpx=b; bx<16; bx++, bpx+=4 )
          collisionarea[((y*16)+by)*48+(x*16+bx)] = (blocks[bpy+bpx]&0xff)?0xff:0x00;
      }
    }
  }

  x = (gid.x-0x1000)>>12;
  y = (gid.y-offset)>>12;

  // Add in any solid objects in the area
  for( i=0; i<num_nthings; i++ )
    addthingytoycolis( n_things[i], sprs, sprites, x, y );
  for( i=0; i<num_bnthings; i++ )
    addthingytoycolis( bn_things[i], sprs, sprites, x, y );
  for( i=0; i<num_pthings; i++ )
    addthingytoycolis( p_things[i], psprs, psprites, x, y );
  for( i=0; i<num_bpthings; i++ )
    addthingytoycolis( bp_things[i], psprs, psprites, x, y );
  for( i=0; i<num_gthings; i++ )
    addthingytoycolis( g_things[i], sprtg, gsprites, x, y );
  for( i=0; i<num_bgthings; i++ )
    addthingytoycolis( bg_things[i], sprtg, gsprites, x, y );
  for( i=0; springs[i].x!=-1; i++ )
    addspringtoycolis( i, x, y );
  if( inclifts )
  {
    for( i=0; lifts[i].hdr.numstops!=-1; i++ )
    {
      if( lifts[i].hdr.fromstop == -1 )
      {
        for( j=0; j<lifts[i].hdr.numstops; j++ )
          addlifttoycolis( i, x, y, lifts[i].hdr.stops[j*2], lifts[i].hdr.stops[j*2+1], j );
      } else {
        addlifttoycolis( i, x, y, lifts[i].hdr.cx>>8, lifts[i].hdr.cy>>8, 0 );
      }
    }
  }
  if( inctardis )
    addtardistoycolis( x, y );
}

int testupcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareay( 0x1000, FALSE, FALSE );

  coff = ((gid.py&0xf)+16)*48 + (gid.px&0xf) + 6;
  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff - edgecdlist[i]*48; // Origin offset - 10y
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<10; k++ )
      {
        ov++; // at least one pixel overlap
        j+=48; // go up one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff++;
  }

  return bov;
}

int dotestdowncolis( int *lifthit, int *liftstophit, int *convey, int *springhit, int *trig )
{
  int coff, i, j, k, ov, bov, wtard;

  gid.y += 0x300;
  gid.py += 3;
  makecolisareay( 0, lifthit != NULL, TRUE );

  wtard = gid.ontardis == ON_TARDIS_STEP_INSIDE ? ON_TARDIS_STEP_INSIDE : ON_TARDIS_STEP_OUTSIDE;
  if( gid.ontardis != INSIDE_TARDIS )
    gid.ontardis = NOT_ON_TARDIS;

  *convey = 0;

  if( lifthit ) *lifthit = -1;
  if( springhit ) *springhit = -1;
  coff = (gid.py&0xf)*48 + (gid.px&0xf) + 6;
  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff + edgecdlist[i]*48; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      switch( collisionarea[j] )
      {
        case 0xf4:
        case 0xf5:
        case 0xf6:
        case 0xf7:
        case 0xf8:
        case 0xf9:
        case 0xfa:
        case 0xfb:
          *trig = collisionarea[j];
          break;
        case 0xfc: if( gid.ontardis == NOT_ON_TARDIS ) gid.ontardis = wtard; break;
        case 0xfd: *convey = 1; break;
        case 0xfe: *convey = 2; break;
        case 0xff: break;
        default:
          if( ( collisionarea[j] >= 0x80 ) && ( collisionarea[j] < 0x90 ) )
          {
            if( springhit ) *springhit = collisionarea[j]-0x80;
            break;
          }

          if( lifthit )     *lifthit     = collisionarea[j]-1;
          if( liftstophit ) *liftstophit = liftstoparea[j];
          break;
      }
      for( k=0; k<10; k++ )
      {
        ov++; // at least one pixel overlap
        j-=48; // go up one pixel
        if( collisionarea[j] == 0 ) break;
        switch( collisionarea[j] )
        {
          case 0xf4:
          case 0xf5:
          case 0xf6:
          case 0xf7:
          case 0xf8:
          case 0xf9:
          case 0xfa:
          case 0xfb:
            *trig = collisionarea[j];
            break;
          case 0xfc: if( gid.ontardis == NOT_ON_TARDIS ) gid.ontardis = wtard; break;
          case 0xfd: *convey = 1; break;
          case 0xfe: *convey = 2; break;
          case 0xff: break;
          default:
            if( ( collisionarea[j] >= 0x80 ) && ( collisionarea[j] < 0x90 ) )
            {
              if( springhit ) *springhit = collisionarea[j]-0x80;
              break;
            }
            if( lifthit )     *lifthit = collisionarea[j]-1;
            if( liftstophit ) *liftstophit = liftstoparea[j];
            break;
        }
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff++;
  }

  gid.y -= 0x300;
  gid.py -= 3;
  return bov;
}

int testdowncolis( int *convey, int *trig )
{
  int bov, glf, gls;

  *trig = 0;

  if (gid.py > ((mapi->fgh*16)-16))
  {
    return (gid.py - ((mapi->fgh*16)-16));
  }

  bov = dotestdowncolis( &glf, &gls, convey, &gid.onspring, trig );

  if( ( glf != -1 ) && ( glf != gid.onlift ) )
  {
    if( lifts[glf].hdr.fromstop == -1 )
    {
      lifts[glf].hdr.fromstop = gls;
      lifts[glf].hdr.tostop   = (gls+1)%lifts[glf].hdr.numstops;
      setliftdeltas( glf );
    }
    gid.lox = gid.x - lifts[glf].hdr.cx;
    if( lifts[glf].hdr.type == LT_BUBBLE )
    {
      gid.loy = gid.py - (lifts[glf].hdr.cy>>8)+lifts[glf].hdr.bobo+lifts[glf].hdr.dip;
//      if( gid.loy > lifts[glf].hdr.surface-30 )
//        gid.loy = lifts[glf].hdr.surface-30;
    } else {
      gid.loy = -((18+lifts[glf].s->hfh)-lifts[glf].hdr.surface);
//      gid.loy = lifts[glf].hdr.surface-30;
    }
    gid.onlift = glf;
  }

  return bov;
}

int testrightcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareax( 0 );

  coff = ((gid.py-10)&15)*32 + (gid.px&15);

  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff + edgecdlist[i]; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<edgecdlist[i]; k++ )
      {
        ov++; // at least one pixel overlap
        j--; // go left one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff+=32;
  }
  return bov;
}

BOOL testleftcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareax( 0x1000 );

  coff = ((gid.py-10)&15)*32 + (gid.px&15) + 16;

  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff - edgecdlist[i]; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<edgecdlist[i]; k++ )
      {
        ov++; // at least one pixel overlap
        j++; // go left one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff+=32;
  }
  return bov;
}

void jumprotate( void )
{
  if( gid.rot == 0 ) return;

  gid.def = 1;
  gid.rot -= 12;
  if( gid.rot < 0 ) gid.rot = 0;
}

void gidanim( BOOL flipped )
{
  Sint16 gidanimframes[] = { 1, 2, 3, 4, 5, 4, 3, 2 };

  gid.flipped = flipped;
  if( gid.rot == 0 )
  {
    gid.framedelay++;
    if( gid.framedelay >= 5 )
    {
      gid.framedelay = 0;
      gid.framestep++;
      if( gid.framestep >= 8 )
        gid.framestep = 0;
      if( gid.framestep == 0 ) lpactionsound( SND_GIDDYSTEP1, sfxvol );
      if( gid.framestep == 4 ) lpactionsound( SND_GIDDYSTEP2, sfxvol );
    }

    gid.def = gidanimframes[gid.framestep];
  }
}

void scroll_x( void )
{
  int off, liml, limr;

  if( stuff.bossmode )
  {
    liml = stuff.bossmodeleft;
    limr = stuff.bossmoderight-320;
  } else {
    liml = 0;
    limr = mapi->fgw*16 - 320;
    if( clevel == 4 )
      limr = 7200;
  }

  off = (fgx+160)-gid.px;
  if( off < -6 ) off = -6;
  if( off > 6 ) off = 6;

  fgx -= off;
  if( fgx < liml ) fgx = liml;
  if( fgx > limr ) fgx = limr;
}

void scroll_y( void )
{
  int off, limt, limb;
  int i, targy;

  limt = mapi->yscrolltoplimit;
  limb = mapi->fgh*16 - 240;

  targy = (gid.py-SCROFF)-stuff.quake;

//  if( gid.onlift == -1 )
  {
    for( i=0; stsc[i].miny!=-1; i++ )
    {
      if( ( gid.py >= stsc[i].miny ) &&
          ( gid.py <= stsc[i].maxy ) )
        targy = stsc[i].scry-stuff.quake;
    }
  }

  if( stuff.quake > 0 )
    stuff.quake--;

  off = fgy-targy;
  if( off < -6 ) off = -6;
  if( off > 6 ) off = 6;

  fgy -= off;
  if( fgy < limt ) fgy = limt;
  if( fgy > limb ) fgy = limb;
}

/*
  mov si,offset sintab
  mov bx,[rumbleindex]
  or bx,bx
  jz ridxok
  mov ax,2000h
  imul [word si+bx]
  mov ax,dx
  cwde
  sub [fmapposy],eax
  mov [rumbled],eax
  add bx,20
  cmp bx,180*2
  jb ridxok
  xor bx,bx
ridxok: mov [rumbleindex],bx
*/


void scrollhandler( void )
{
  scroll_x();
  if( gid.x < 0x1600 ) { gid.x = 0x1600; gid.px = 0x16; }
  if( gid.x > (mapi->fgw<<12) ) { gid.x = mapi->fgw<<12; gid.px = gid.x>>8; }

  scroll_y();

  bgx = fgx>>mapi->xdiv;
  bgy = (fgy+(mapi->yoff<<4))>>mapi->ydiv;
}

/*
;---------------------------------------------------------------------------

  xor ebx,ebx
  mov bx,[bmapxlimit]
  shl ebx,12
  mov eax,[fmapposx]  ;calc background parallax pos
  mov cl,[Bmapxdiv]
  shr eax,cl
xmlmod: cmp eax,ebx
  jb bmlok
  sub eax,ebx
  jmp xmlmod
bmlok:  mov [bmapposx],eax

  mov ax,[bmapyoffset]
  cwde
  shl eax,12
  add eax,[fmapposy]
  mov cl,[Bmapydiv]
  shr eax,cl
  xor ebx,ebx
  mov bx,[bmapylimit]
  shl ebx,12
ymlmod: cmp eax,ebx
  jb bmylok
  sub eax,ebx
  jmp ymlmod
bmylok: mov [bmapposy],eax

scrh_end:
  ret

ENDP
*/

void giddyhit( void )
{
  if( ( gid.flashing > 0 ) ||
      ( gid.dieanim ) ||
      ( gid.usemode ) )
    return;

  if( gid.energy == 0 )
  {
    gid.lives--;
    gid.dieanim = TRUE;
    gid.def = 6;
    gid.dy = gid.y;
    gid.mainchryoffset = (3<<8);
    actionsound( SND_LOSELIFE, sfxvol );
    actionsound( SND_GIDDYHITECHO, sfxvol );
  } else {
    gid.energy--;
    actionsound( SND_GIDDYHIT, sfxvol );
  }

  gid.flashing = 120;
  gid.redang = 0.0f;
}

int slowsink( void )
{
  return blktrans[fgmap[((gid.y+0x1000)>>12)*mapi->fgw+(gid.x>>12)]+256]&0x0c;
}

void make_giddy_do_things( void )
{
  Sint16 oldx, oldy;
  int i;

  oldy = gid.y;
  oldx = gid.x;
  //fmapposy+=rumbled;
  //rumbled=0;

  if( gid.fallpuffs )
  {
    if( ( (frame&3) == 0 ) && ( gid.mainchryoffset < 0 ) )
      startincidental( gid.px, gid.py, 0, -1, GSPRITEX, bangframes, 5, 2 );
  }


#ifdef __demo__
  if (gid.demofinished)
  {
	  //LOGI("state = %d fade = %d", gid.demofinishedstate,gid.demofinishedfade);
	  switch( gid.demofinishedstate )
	  {
	  case 0: // Fade it
		  if( gid.demofinishedfade < 255 )
		  {
			  gid.demofinishedfade += 4;
			  if( gid.demofinishedfade > 255 ) gid.demofinishedfade = 255;
			  break;
		  }

		  gid.demofinishedstate++;
		  break;

	  case 1: // Wait for key press
	  {
		  //Touch detection is giddy.c
		  //In fact stuck in this state, need to start a new game
	  }
	  }
	  return;
  }
#endif

  if( gid.spoonedit )
  {
    gid.spooneditwobble += 0.1f;
    if( gid.spooneditwobble > 6.2831853f ) gid.spooneditwobble -= 6.2831853f;
    switch( gid.spooneditstate )
    {
      case 0: // Fade it
        if( gid.spooneditwobblefactor > 0.0f )
        {
          gid.spooneditwobblefactor -= 0.08f;
          if( gid.spooneditwobblefactor < 0.0f )
            gid.spooneditwobblefactor = 0.0f;
        }

        if( gid.spooneditfade < 255 )
        {
          gid.spooneditfade += 4;
          if( gid.spooneditfade > 255 ) gid.spooneditfade = 255;
          break;
        }

        gid.spooneditstate++;
        break;

      case 1: // Show it
        if( gid.spooneditwobblefactor > 0.0f )
        {
          gid.spooneditwobblefactor -= 0.08f;
          if( gid.spooneditwobblefactor < 0.0f )
            gid.spooneditwobblefactor = 0.0f;
        }

        if( gid.spooneditwait > 0 )
        {
          gid.spooneditwait--;
          break;
        }

        gid.spooneditstate++;
        break;
      
      case 2: // Fade out
        fadetype = 0;
        if( fadea < 255 )
        {
          fadea += 4;
          if( fadea > 255 ) fadea = 255;
          break;
        }

        if( lrfade2 != 255 )
          break;

        gid.spooneditstate++;  // 3 = done!
        fadeadd = -6;
        break;
    }
    return;
  }

  if( gid.dieanim )
  {
    gid.dy -= gid.mainchryoffset;

    if( ((gid.dy>>8)-fgy) > 256 )
    {
      if( gid.lives < 0 )
      {
        gid.spoonedit = TRUE;
        gid.spooneditstate = 0;
        gid.spooneditfade = 0;
        gid.spooneditwobblefactor = 12.0f;
        gid.spooneditwait = 120;
        gid.spooneditwobble = 0.0f;
		destroy_save_game();
        return;
      }

      gid.energy = 3;
      gid.dieanim = FALSE;
      gid.mainchryoffset = -0x20;
      gid.def = 1;
    } else {
      gid.mainchryoffset -= 0x20;
      if( gid.mainchryoffset < -(6<<8) )
        gid.mainchryoffset = -(6<<8);
      gid.def = ((frame>>3)&1)+6;
      return;
    }
  }

  if( gid.flashing > 0 )
  {
    gid.flashing--;
    gid.redang += 0.15f;
    if( gid.redang > 3.14159265f ) gid.redang -= 3.14159265f;
  }

  if( gid.watchwarp )
  {
    gid.rot += 4;
    gid.wwob += 0.09f;
    gid.sgwsc += 0.03f;
    if( gid.sgwsc > 2.0f )
    {
      gid.wwa -= 3;
      if( gid.wwa < 0 ) gid.wwa = 0;
    }
    if( ( llevel != gid.wwl ) &&
        ( gid.sgwsc > 4.0f ) )
    {
      llevel = gid.wwl;
      fadea = 0;
      fadeadd = 8;
      fadetype = 0;
      gid.stopuntilfade = TRUE;
    }
    return;
  }

  if( gid.stargatewarp )
  {
    if( gid.px < 6654 ) { gid.px++; gid.x+=0x100; }
    if( gid.px > 6654 ) { gid.px--; gid.x-=0x100; }
    if( gid.py <  136 ) { gid.py++; gid.y+=0x100; }
    if( gid.py >  136 ) { gid.py--; gid.y-=0x100; }
    gid.rot+=12;
    if( gid.sgwsc > 0.0f )
      gid.sgwsc -= 0.02f;

    if( ( llevel != 5 ) &&
        ( gid.sgwsc < 0.4f ) )
    {
      llevel = 5;
      fadea = 0;
      fadeadd = 8;
      fadetype = 0;
      gid.stopuntilfade = TRUE;
    }
    return;
  }

  if( gid.dazed )
  {
    // Giddy is dazed
    return;
  }

  if( ( gjump ) && ( !gid.jumping ) && ( !gid.usemode ) )
	{
		if( ignorejump )
    {
			ignorejump = FALSE;
	  } else {
      gid.jump = TRUE;
      gjump = FALSE;
    }
	}

  if( !gid.usemode ) // Can't move if in use mode
  {
    if( ( !gid.stopuntilfade ) ||
        ( fadeadd == 0 ) )
    {
      if( ( !gid.nogmxgrav ) &&
          ( !gid.jumplatch ) &&
          ( gid.jump ) &&
          ( !gid.jumping ) &&
          ( gid.allowjump ) &&
          ( gid.ontardis != INSIDE_TARDIS ) )
      {
        gid.jumpcount++;
        gid.jumping = 1;
        gid.rot = 358;
        considertardis = TRUE;
        gid.ontardis = NOT_ON_TARDIS;
        actionsound( SND_JUMP, sfxvol );
      }

      if( gid.jumping )
      {
        gid.mainchryoffset = ((sintab[gid.jumping+90]*0xb)>>8)&0xff00;
          
        jumprotate();
        gid.jumping += gid.jumpstep;
        if( gid.jumping >= 180 )
        {
          gid.jumpstep = 6;
          gid.framestep = 0;
          gid.jumping = 0;
          gid.jump = FALSE;
          gid.mainchryoffset = 0xfc00;
        }
      }

      if( ( !gid.nogmxgrav ) &&
          ( !gid.teleport ) )
      {
        // Inertial X motion control
        if( gright )
        {
          gid.movedisp+=2;
          if( gid.movedisp > 8 ) gid.movedisp = 8;
        }

        if( gleft )
        {
          gid.movedisp-=2;
          if( gid.movedisp < -8 ) gid.movedisp = -8;
        }
      }
    }
  } else {

    if( ( gleft ) && ( ibadd == 0 ) && ( ibitem != -1 ) )
    {
      do { ibitem=(ibitem+1)%INV_LAST; } while( inv[ibitem] == 0 );
      set_ibstr( 2, ivtexts[ibitem].use );
      iboff = ibw[1] + 81;
      ibdest = -(strlen(ivtexts[ibitem].use)*4) - iboff;
      ibadd = -4;
      actionsound( SND_USE, sfxvol );
    }

    if( ( gright ) && ( ibadd == 0 ) && ( ibitem != -1 ) )
    {
      do { ibitem=(ibitem+INV_LAST-1)%INV_LAST; } while( inv[ibitem] == 0 );
      set_ibstr( 2, ibt[1] );
      set_ibstr( 1, ivtexts[ibitem].use );
      iboff = ibw[1]+81;
      ibpos -= iboff;
      ibdest = -strlen(ivtexts[ibitem].use)*4;
      ibadd = 4;
      actionsound( SND_USE, sfxvol );
    }
  }

  if( gid.ontardis == INSIDE_TARDIS )
  {
    gid.x = trd_x+(26<<8);
    gid.movedisp = 0;
    gid.mainchryoffset = 0;
    return;
  }

  if( gid.lockx != -1 )
  {
    gid.x = gid.lockx;
    gid.px = gid.x>>8;
    gid.y = gid.locky;
    gid.py = gid.y>>8;
    return;
  }

  // Dampen the motion
  if( gid.movedisp > 0 ) gid.movedisp--;
  if( gid.movedisp < 0 ) gid.movedisp++;

  if( gid.movedisp > 0 )
  {
    if( gid.def != 6 )
      gid.flipped = FALSE;
    if( ( testrightcolis() == 0 ) && ( gid.movedisp > 1 ) )
    {
      gid.x += ((gid.movedisp<<7)&0xffffff00);
      gid.px = gid.x>>8;
      if( stuff.bossmode )
      {
        if( gid.x > (stuff.bossmoderight<<8) )
        {
          gid.x = (stuff.bossmoderight<<8);
        } else {
          i = testrightcolis();
          if( i != 0 )
            gid.x -= (i<<8);
          else
            gidanim( FALSE );
        }
      } else {
        if( gid.onlift != -1 ) gid.lox += ((gid.movedisp<<7)&0xffffff00);
        i = testrightcolis();
        if( i != 0 )
          gid.x -= (i<<8);
        else
          gidanim( FALSE );
      }
      gid.px = gid.x>>8;
    }

  } else if( gid.movedisp < 0 ) {

    if( gid.def != 6 )
      gid.flipped = TRUE;
    if( ( testleftcolis() == 0 ) && ( gid.movedisp < -1 ) )
    {
      gid.x += ((gid.movedisp<<7)&0xffffff00);
      gid.px = gid.x>>8;
      if( stuff.bossmode )
      {
        if( gid.x < (stuff.bossmodeleft<<8) )
        {
          gid.x = (stuff.bossmodeleft<<8);
        } else {
          i = testleftcolis();
          if( i != 0 )
          {
            gid.x += (i<<8);
            if( ( gid.x&0xffffff00 ) < ( oldx&0xffffff00 ) )
              gidanim( TRUE );
          } else {
            gidanim( TRUE );
          }
        }
      } else {
        if( gid.onlift != -1 ) gid.lox += ((gid.movedisp<<7)&0xffffff00);
        i = testleftcolis();
        if( i != 0 )
        {
          gid.x += (i<<8);
          if( ( gid.x&0xffffff00 ) < ( oldx&0xffffff00 ) )
            gidanim( TRUE );
        } else {
          gidanim( TRUE );
        }
      }
      gid.px = gid.x>>8;
    }
  }

  gid.y -= (int)gid.mainchryoffset;  // down?
  gid.py = gid.y>>8;

  if( gid.mainchryoffset <= 0 )
  {
    int convey, triggr;

    i = testdowncolis( &convey, &triggr );
    if( i == 0 )
    {
      i = slowsink();
      if( i != 0 )
      {
        gid.y += gid.mainchryoffset;
        gid.y += 0x100;
        gid.py = gid.y>>8;
        gid.mainchryoffset = 0xff80;
        if(i&0x04) giddyhit();
      }
    } else {
      gid.mainchryoffset = 0xfe80;
      gid.y -= (i<<8);
      gid.py = gid.y>>8;
      gid.fallpuffs = FALSE;
      switch( gid.ontardis )
      {
        case ON_TARDIS_STEP_OUTSIDE:
          if( gid.x >= (trd_x+(44<<8)) )
            gid.ontardis = ON_TARDIS_STEP_INSIDE;
          break;
        
        case ON_TARDIS_STEP_INSIDE:
          if( gid.x < (trd_x+(26<<8)) )
          {
            gid.ontardis = INSIDE_TARDIS;
            incidentalsound( SND_TARDIS, sfxvol );
          }
          break;
        
        case NOT_ON_TARDIS:
          considertardis = FALSE;
          break;
      }
    }

    switch( triggr )
    {
      case 0xfb:
        triggerfactory();
        break;
      
      case 0xf4:
        specialbinstand();
        break;

      case 0xf5:
        if( p_things5[7].frames[0] == 22 )
        {
          p_things5[7].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_BLUE_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_BLUE_PRESSED] = 1;
          p_things5[8].numframes = 2;
          p_things5[23].active = TRUE;
          initbigassfan();
        }
        break;

      case 0xf6:
        if( p_things5[5].frames[0] == 22 )
        {
          p_things5[5].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_RED_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_RED_PRESSED] = 1;
          p_things5[6].numframes = 2;
          p_things5[21].active = TRUE;
          initbigassfan();
        }
        break;

      case 0xfa:
        if( p_things5[3].frames[0] == 22 )
        {
          p_things5[3].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_GREEN_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_GREEN_PRESSED] = 1;
          p_things5[4].numframes = 2;
          p_things5[22].active = TRUE;
          initbigassfan();
        }
        break;
      
      case 0xf9:
        gid.sgwsc = 1.0f;
        gid.wwa = 255;
        gid.watchwarp = TRUE;
        gid.wwl = 1;
        incidentalsound( SND_TELEPORT_OUT, sfxvol );
        break;
      
      case 0xf8:
        if( p_things3[6].y == 901 )
        {
          p_things3[6].y = 905;
          gid.y += (4<<8);
          gid.py += 4;
          triggertimerdoor();
        }
        break;
      
      case 0xf7:
        if( p_things3[20].y == 1061 )
        {
          p_things3[20].y = 1065;
          gid.y += (4<<8);
          gid.py += 4;
          triggertimerdoor();
        }
        break;

      default:
        if( p_things5[3].frames[0] == 23 )
          p_things5[3].frames[0] = 22;
        if( p_things5[5].frames[0] == 23 )
          p_things5[5].frames[0] = 22;
        if( p_things5[7].frames[0] == 23 )
          p_things5[7].frames[0] = 22;
        if( p_things3[6].y == 905 )
          p_things3[6].y = 901;
        if( p_things3[20].y == 1065 )
          p_things3[20].y = 1061;
        if( onthebin ) specialbinleave();
        break;
    }

    switch( convey )
    {
      case 1:
        gid.x -= 0x100;
        gid.px--;
        break;
      
      case 2:
        gid.x += 0x100;
        gid.px++;
        break;
    }

    if( gid.onspring != -1)
    {
      if( springs[gid.onspring].frame < 3 )
      {
        springs[gid.onspring].frame++;
        springs[gid.onspring].rtime = 5;
      } else {
        if( gid.jumping == 0 )
        {
          incidentalsound( SND_SPRING, sfxvol );
          gid.jumpcount++;
          gid.jumping = 1;
          gid.rot = 358;
          gid.y -= 0x1400;
          gid.py -= 0x14;
        }
      }
    }

    gid.allowjump = (i!=0);
    if( gid.onlift != -1 ) gid.allowjump = TRUE;
    gid.mainchryoffset -= 0x80;
    if( gid.mainchryoffset < -(6*256) )
      gid.mainchryoffset = -(6*256);
  }

  if( gid.mainchryoffset > 0 )
  {
    i = testupcolis();
    if( i != 0 )
    {
      gid.y += gid.mainchryoffset;
      gid.py = gid.y>>8;
    }

    gid.onspring = -1;

    // cheatfly
  }

/*
       and [word giddef],0cfffh
       mov al,[gidflicker]
       or al,al
       jz ngflk
       dec [gidflicker]
       cmp [gidflicker],97
       jb gflik
       or [giddef],2000h
       jmp ngflk
gflik: and al,2
       jz ngflk
       or [word giddef],01000h

ngflk: mov ax,[gidback]
       or [giddef],ax
       mov [gidback],0
*/
}

void render_tvborders( void )
{
  glLoadIdentity();
  glColor4ub( 255, 255, 255, 255 );
/*
  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  glBegin( GL_QUADS );
    glTexCoord2f(         0.0f,         0.0f ); glVertex3f(   0.0f,  -8.0f, 0.0f );
    glTexCoord2f( 56.0f/256.0f,         0.0f ); glVertex3f( 320.0f,  -8.0f, 0.0f );
    glTexCoord2f( 56.0f/256.0f, 62.0f/256.0f ); glVertex3f( 320.0f,   0.0f, 0.0f );
    glTexCoord2f(         0.0f, 62.0f/256.0f ); glVertex3f(   0.0f,   0.0f, 0.0f );

    glTexCoord2f(         0.0f,         0.0f ); glVertex3f(   0.0f, 224.0f, 0.0f );
    glTexCoord2f( 56.0f/256.0f,         0.0f ); glVertex3f( 320.0f, 224.0f, 0.0f );
    glTexCoord2f( 56.0f/256.0f, 62.0f/256.0f ); glVertex3f( 320.0f, 232.0f, 0.0f );
    glTexCoord2f(         0.0f, 62.0f/256.0f ); glVertex3f(   0.0f, 232.0f, 0.0f );
  glEnd();
*/
  glBindTexture( GL_TEXTURE_2D, tex[TVTEX] );

  set_quad_tri( &gvtx[ 0],  -0.5f,  -0.5f,  15.5f,  15.5f );
  set_texc_tri( &gtxq[ 0],   0.0f,   0.0f,   0.5f,   0.5f );
  set_quad_tri( &gvtx[12], 304.5f,  -0.5f, 320.5f,  15.5f );
  set_texc_tri( &gtxq[12],   0.5f,   0.0f,   1.0f,   0.5f );
  set_quad_tri( &gvtx[24],  -0.5f, 224.5f,  15.5f, 240.5f );
  set_texc_tri( &gtxq[24],   0.0f,   0.5f,   0.5f,   1.0f );
  set_quad_tri( &gvtx[36], 304.5f, 224.5f, 320.5f, 240.5f );
  set_texc_tri( &gtxq[36],   0.5f,   0.5f,   1.0f,   1.0f );

  glVertexPointer( 2, GL_FLOAT, 0, gvtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
  glDrawArrays(GL_TRIANGLES, 0, 24);
}

void render_background( void )
{
  int o, xo, yo, bx, by, x, y, splity, splo, vo;
  struct btex *tx;

  glBindTexture( GL_TEXTURE_2D, tex[BLOCKTEX]);
  glColor4ub(255, 255, 255, 255);

  glLoadIdentity();
  for( y=0, yo=0, o=0; y<=240; y+=16, yo++ )
  {
    for( x=0, xo=0; x<320; x+=16, xo++ )
    {
      tx = &bt[dmap[yo*20+xo]];

      set_quad_tri( &gvtx[o], x, y, x+16, y+16 );
      set_texc_tri( &gtxq[o], tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );

      o += 12;
    }
  }

  glVertexPointer( 2, GL_FLOAT, 0, gvtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
  glDrawArrays(GL_TRIANGLES, 0, o/2);

  splity = mapi->splitpos-fgy;
  if( ( clevel == 1 ) && ( gid.y < (370<<8) ) )
    splity = 100000;

  xo = -(bgx&15);
  yo = -(bgy&15);
  by = (bgy>>4)%mapi->ywrap;

  splo = 0;

  vo = 0;
  for( y=yo; y<240; y+=16 )
  {
    bx = (bgx>>4)%mapi->xwrap;

    if( ( splo == 0 ) && ( y >= splity ) )
      splo = mapi->sply;

    for( x=xo; x<336; x+=16 )
    {
      o = bgmap[(by+splo)*mapi->bgw+bx];
      if( o > 0 )
      {
        tx = &bt[o];

        set_quad_tri( &gvtx[vo], x, y, x+16, y+16 );
        set_texc_tri( &gtxq[vo], tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );

        vo += 12;
      }
      bx = (bx+1)%mapi->xwrap;
    }

    by=(by+1)%mapi->ywrap;
    if( ( clevel == 1 ) && ( splo == 0 ) && ( by > 15 ) && ( gid.y < (390<<8) ) )
      by = 15;
    if( ( clevel == 4 ) && ( by > 14 ) )
      by = 14;
  }

  glVertexPointer( 2, GL_FLOAT, 0, gvtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
  glDrawArrays(GL_TRIANGLES, 0, vo/2);
}

void render_foreground_zone( int fx, int fy, int fw, int fh )
{
  int x, y, xc, yc, b, o;
  struct btex *tx;

  x = (fx*16)-fgx;
  y = (fy*16)-fgy;
  if( ( x >= 320 ) || ( (x+(fw*16)) < 0 ) ) return;
  if( ( y >= 240 ) || ( (y+(fh*16)) < 0 ) ) return;

  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[BLOCKTEX] );
  glColor4ub(255, 255, 255, 255);
  o = 0;
  for( yc=0; yc<fh; yc++ )
  {
    x = (fx*16)-fgx;
    for( xc=0; xc<fw; xc++ )
    {
      b = fgmap[(yc+fy)*mapi->fgw+xc+fx];
      if( b )
      {
        tx = &bt[b];
        set_quad_tri( &gvtx[o], x, y, x+16, y+16 );
        set_texc_tri( &gtxq[o], tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
        o += 12;
      }
      x+=16;
    }
    y+=16;
  }

  if( o > 0 )
  {
    glVertexPointer( 2, GL_FLOAT, 0, gvtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
    glDrawArrays(GL_TRIANGLES, 0, o/2);
  }
}

void render_foreground( void )
{
  int xo, yo, bx, by, x, y, o;
  struct btex *tx;

  xo = -(fgx&15);
  yo = -(fgy&15);
  bx = fgx>>4;
  by = (fgy>>4) * mapi->fgw;

  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[BLOCKTEX] );
  glColor4ub(255, 255, 255, 255);
  o = 0;
  for( y=yo; y<240; y+=16 )
  {
    bx = fgx>>4;
    for( x=xo; x<336; x+=16 )
    {
      if( fgmap[by+bx] != 0 )
      {
        tx = &bt[fgmap[by+bx]];
        set_quad_tri( &gvtx[o], x, y, x+16, y+16 );
        set_texc_tri( &gtxq[o], tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
        o += 12;
      }
      bx++;
    }
    by+=mapi->fgw;
  }

  if (o > 0)
  {
    glVertexPointer( 2, GL_FLOAT, 0, gvtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
    glDrawArrays(GL_TRIANGLES, 0, o/2);
  }
}

void render_sprite_scaled( struct btex *tx, int x, int y, BOOL flipx, float rot, float scale )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  glScalef( scale, scale, 1.0f );
  glColor4ub(255, 255, 255, 255);
  set_quad_tristrip( vtx, -sw, -sh, sw, sh );
  if( flipx )
  {
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }

  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_scaleda( struct btex *tx, int x, int y, BOOL flipx, float rot, float scale, int alpha )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  glScalef( scale, scale, 1.0f );
  glColor4ub(255, 255, 255, alpha);
  set_quad_tristrip( vtx, -sw, -sh, sw, sh );
  if( flipx )
  {
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }

  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_giddy( void )
{
  float sw, sh, x, y, ca, cb, ga;
  struct btex *tx;
  float rot;
  GLfloat vtx[4*2], txq[4*2];

  if( gid.dieanim )
  {
    y = (gid.dy>>8)-fgy;
    rot = 0.0f;
    if( ( clevel == 4 ) && ( gid.hardhat ) )
    {
      glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
      tx = &sprs[gid.def+36];
    } else {
      glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
      tx = &sprtg[gid.def];
    }
  } else {
    if( gid.ontardis == INSIDE_TARDIS )
      return;

    y = gid.py-fgy;
    rot = gid.rot;
    tx = &sprs[gid.def];
    glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  }

  x = gid.px-fgx;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !gid.flipped ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  if( gid.stargatewarp )
    glScalef( gid.sgwsc, gid.sgwsc, 1.0f );
  if( gid.watchwarp )
  {
    glScalef( gid.sgwsc + sin( gid.wwob ) * gid.sgwsc * 0.4f,
              gid.sgwsc + sin( gid.wwob*0.8f ) * gid.sgwsc * 0.4f,
              1.0f );
  }
  ga = 255;
  if( gid.watchwarp ) ga = gid.wwa;
  if( ( gid.flashing ) && ( !gid.dieanim ) )
  {
    ca = ((int)(sin(gid.redang)*63.0f))+192;
    cb = ((int)(sin(gid.redang)*160.0f)); if( cb < 0 ) cb = 0;
  } else if( gid.watchwarp ) {
    ca = ((int)(sin(gid.wwob*1.2f)*95.0f))+160;
    cb = ((int)(sin(gid.wwob*0.9f)*95.0f))+160;
  } else {
    ca = 255;
    cb = 255;
  }

  glColor4ub( ca, cb, cb, ga );

  if( ( clevel == 4 ) && ( gid.hardhat ) && ( !gid.dieanim ) )
  {
    float hsb, hy;
 
    hsb = 24.0f - sh;
    hy = tx->y+(24.0f/256.0f);

    if( gid.flipped )
    {
      set_quad_tri( &gvtx[0], -sw, -sh, sw, hsb );
      set_texc_tri( &gtxq[0], tx->x+tx->w, 232.0f/256.0f, tx->x, 1.0f );
      set_quad_tri( &gvtx[12], -sw, hsb, sw, sh );
      set_texc_tri( &gtxq[12], tx->x+tx->w, hy, tx->x, tx->y+tx->h );
    } else {
      set_quad_tri( &gvtx[0], -sw, -sh, sw, hsb );
      set_texc_tri( &gtxq[0], tx->x, 232.0f/256.0f, tx->x+tx->w, 1.0f );
      set_quad_tri( &gvtx[12], -sw, hsb, sw, sh );
      set_texc_tri( &gtxq[12], tx->x, hy, tx->x+tx->w, tx->y+tx->h );
    }
    glVertexPointer( 2, GL_FLOAT, 0, gvtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, gtxq );
    glDrawArrays(GL_TRIANGLES, 0, 12);
    return;
  }

  if( gid.flipped )
  {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite( struct btex *tx, int x, int y, BOOL flipx, float rot )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_a( struct btex *tx, int x, int y, BOOL flipx, float rot, int alpha )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  glColor4ub(255, 255, 255, alpha);
  if( flipx )
  {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, -sw, -sh, sw, sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_tl( struct btex *tx, int x, int y, BOOL flipx )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->fw;
  sh = tx->fh;

  glLoadIdentity();
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_tl_stretch( struct btex *tx, int x, int y, float sw, float sh, BOOL flipx )
{
  GLfloat vtx[4*2], txq[4*2];
  if( tx->w == 0.0f ) return;  // Dummy sprite?

  glLoadIdentity();
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_tl_clipy( struct btex *tx, int x, int y, int clipy, BOOL flipx )
{
  float sw, sh, th;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?
  if( y >= clipy ) return;

  sw = tx->fw;
  sh = tx->fh;
  th = tx->h;

  if( ( y+sh ) >= clipy )
  {
    sh = clipy-y;
    th = sh/256.0f;
  }

  glLoadIdentity();
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+th );
  } else {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+th );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_tl_clipyb( struct btex *tx, int x, int y, int clipy, BOOL flipx )
{
  float sw, sh, tt, th;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?
  if( (y+tx->fh) <= clipy ) return;

  sw = tx->fw;
  sh = tx->fh;
  tt = tx->y;
  th = tx->h;

  if( y < clipy )
  {
    sh -= clipy-y;
    th -= (clipy-y)/256.0f;
    tt += (clipy-y)/256.0f;
    y = clipy;
  }

  glLoadIdentity();
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x+tx->w, tt, tx->x, tt+th );
  } else {
    set_quad_tristrip( vtx, x, y, x+sw, y+sh );
    set_texc_tristrip( txq, tx->x, tt, tx->x+tx->w, tt+th );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_sprite_offs( struct btex *tx, int x, int y, int xo, int yo, BOOL flipx, float rot )
{
  float sw, sh;
  GLfloat vtx[4*2], txq[4*2];

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

  glLoadIdentity();
  glTranslatef( x, y, 0.0f );
  glRotatef( rot, 0.0f, 0.0f, 1.0f );
  glColor4ub(255, 255, 255, 255);
  if( flipx )
  {
    set_quad_tristrip( vtx, xo-sw, yo-sh, xo+sw, yo+sh );
    set_texc_tristrip( txq, tx->x+tx->w, tx->y, tx->x, tx->y+tx->h );
  } else {
    set_quad_tristrip( vtx, xo-sw, yo-sh, xo+sw, yo+sh );
    set_texc_tristrip( txq, tx->x, tx->y, tx->x+tx->w, tx->y+tx->h );
  }
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_nthing( struct thingy *t )
{
  int x, y;
  struct btex *s;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &sprs[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;
 
  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    glBindTexture( GL_TEXTURE_2D, tex[WSPRITEX] );
  else
    glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );

  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
};

void render_pthing( struct thingy *t )
{
  int x, y;
  struct btex *s;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &psprs[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;
 
  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    glBindTexture( GL_TEXTURE_2D, tex[WPSPRITEX] );
  else
    glBindTexture( GL_TEXTURE_2D, tex[PSPRITEX] );

  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
};

void render_gthing( struct thingy *t )
{
  int x, y;
  struct btex *s;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &sprtg[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;

  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    glBindTexture( GL_TEXTURE_2D, tex[WGSPRITEX] );
  else
    glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
 
  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
};


void render_infobulb( void )
{
  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
	if( infobulbalpha > 0 )
    render_sprite_scaleda( &sprtg[110], 245, 17, FALSE, 0, infobulbzoom, infobulbalpha );

  if( winfo == -1 ) 
	{
		infobulbwinfo = -1;
		return;
  }

  if( infobulbwinfo != winfo )
	{
		infobulbalpha = 255;
		infobulbzoom = 1.0f;
		infobulbwinfo = winfo;
	}

  render_sprite_tl( &sprtg[(frame&0x08)?110:5], 236, 4, FALSE );
}

void ring_o_stars( int x, int y, float ang, float astep, int num, float speed )
{
  int i;

  for( i=0; i<num; i++ )
  {
    stars[nextstar].x          = (x<<8);
    stars[nextstar].y          = (y<<8);
    stars[nextstar].dx         = (int)(cos(ang)*speed);
    stars[nextstar].dy         = (int)(sin(ang)*speed);
    stars[nextstar].framecount = 0;
    stars[nextstar].framespeed = 5;
    stars[nextstar].frame      = 18;
    nextstar = (nextstar+1)%MAX_STARS;
    ang += astep;
  }
}

void movelifts( void )
{
  int i, j, oldx, oldy;
  int fx,fy,tx,ty;
  float ang, scc;
  BOOL moveon;

  for( i=0; lifts[i].hdr.numstops!=-1; i++ )
  {
    oldx = lifts[i].hdr.cx;
    oldy = lifts[i].hdr.cy;
    if( lifts[i].hdr.type == LT_FRETURN )
      lifts[i].hdr.bobo = ((int)(sin( ((float)frame)/6.0f )*lifts[i].hdr.bob));
    else
      lifts[i].hdr.bobo = ((int)(sin( ((float)frame)/16.0f )*lifts[i].hdr.bob));

    if( gid.onlift == i )
    {
      if( lifts[i].hdr.dip < lifts[i].hdr.dipmax )
        lifts[i].hdr.dip+=3;
    } else {
      if( lifts[i].hdr.dip > 0 )
        lifts[i].hdr.dip-=3;
    }

    if( lifts[i].hdr.fromstop == -1 )
      continue;

    fx = lifts[i].hdr.stops[lifts[i].hdr.fromstop*2];
    fy = lifts[i].hdr.stops[lifts[i].hdr.fromstop*2+1];
    tx = lifts[i].hdr.stops[lifts[i].hdr.tostop*2];
    ty = lifts[i].hdr.stops[lifts[i].hdr.tostop*2+1];

    if( lifts[i].hdr.timeout > 0 )
    {
      lifts[i].hdr.timeout--;
      if( gid.onlift != i )
      {
        if( lifts[i].hdr.type == LT_BTRIGGR )
        {
          if( lifts[i].hdr.fromstop == 0 )
          {
            lifts[i].hdr.timeout = lifts[i].hdr.itimeout;
            if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
              stopchannel( liftloopchan );
            continue;
          }
        }

        if( lifts[i].hdr.type == LT_TRIGGER )
        {
          lifts[i].hdr.fromstop = -1;
          lifts[i].hdr.timeout = lifts[i].hdr.itimeout;
          if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
            stopchannel( liftloopchan );
          continue;
        }
      }
    } else {
      moveon = FALSE;

      lifts[i].hdr.cx += lifts[i].hdr.dx;
      lifts[i].hdr.cy += lifts[i].hdr.dy;

      if( lifts[i].hdr.dy == 0 )
      {
        if( lifts[i].hdr.dx > 0 )
        {
          if( (lifts[i].hdr.cx>>8) >= tx )
            moveon = TRUE;
        } else {
          if( (lifts[i].hdr.cx>>8) <= tx )
            moveon = TRUE;
        }
      } else {
        if( lifts[i].hdr.dy > 0 )
        {
          if( (lifts[i].hdr.cy>>8) >= ty )
            moveon = TRUE;
        } else {
          if( (lifts[i].hdr.cy>>8) <= ty )
            moveon = TRUE;
        }
      }

      if( moveon )
      {
        if( ( (lifts[i].hdr.cx>>8) > (fgx-64) ) &&
            ( (lifts[i].hdr.cx>>8) < (fgx+384) ) &&
            ( (lifts[i].hdr.cy>>8) > (fgy-64) ) &&
            ( (lifts[i].hdr.cy>>8) < (fgy+304) ) )
        {
          if( lifts[i].hdr.sndstop != -1 )
            incidentalsound( lifts[i].hdr.sndstop, sfxvol );
        }

        if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
          stopchannel( liftloopchan );

        switch( lifts[i].hdr.type )
        {
          case LT_BUBBLE:
            {
              BOOL anydraw;
              lifts[i].hdr.cx = lifts[i].hdr.stops[0]<<8;
              lifts[i].hdr.cy = lifts[i].hdr.stops[1]<<8;
              lifts[i].hdr.timeout = 60;
              anydraw = FALSE;
              ang = 0.0f;
              for( j=0; j<8; j++, ang+=(3.14159265/4.0f) )
                anydraw |= startincidental( lifts[i].hdr.stops[2] + ((int)(sin(ang)*18.0f)), lifts[i].hdr.stops[3] + ((int)(cos(ang)*18.0f)), 0, 0, SPRITEX, ibubblepop, 5, 1 );
              if( gid.onlift == i ) gid.onlift = -1;
              if( anydraw ) incidentalsound( SND_BUBBLEPOP, sfxvol );
            }
            break;
          
          case LT_TRIGGER:
            if( gid.onlift != i )
            {
              lifts[i].hdr.fromstop = -1;
              continue;
            }

          default:
            lifts[i].hdr.timeout = lifts[i].hdr.itimeout;

            lifts[i].hdr.fromstop = lifts[i].hdr.tostop;
            lifts[i].hdr.tostop = (lifts[i].hdr.tostop+1)%lifts[i].hdr.numstops;
            setliftdeltas( i );
            break;
        }
      } else {
        if( ( lifts[i].hdr.sndloop != -1 ) && ( liftloopchan == -1 ) )
        {
          if( ( (lifts[i].hdr.cx>>8) > (fgx-64) ) &&
              ( (lifts[i].hdr.cx>>8) < (fgx+384) ) &&
              ( (lifts[i].hdr.cy>>8) > (fgy-64) ) &&
              ( (lifts[i].hdr.cy>>8) < (fgy+304) ) )
          {
            liftlooplift = i;
            ambientloop( lifts[i].hdr.sndloop, sfxvol, &liftloopchan );
          }
        }
      }
    }

    if( lifts[i].hdr.type == LT_BUBBLE )
    {
      scc = ((float)(lifts[i].hdr.stops[1]-(lifts[i].hdr.cy>>8))) / 64.0f;
      if( scc > 1.0f ) scc = 1.0f;
      lifts[i].hdr.scale = scc;
    }

    if( gid.onlift == i )
    {
      if( gid.mainchryoffset <= 0 )
      {
        gid.x = lifts[i].hdr.cx + gid.lox;
        gid.y = lifts[i].hdr.cy + ((gid.loy+lifts[i].hdr.bobo+lifts[i].hdr.dip)<<8);
        gid.px = gid.x>>8;
        gid.py = gid.y>>8;
      } else {
        gid.onlift = -1;
      }
      j = ((lifts[i].s->fw/2)+9)<<8;
      if( ( gid.lox < -j ) || ( gid.lox > j ) )
        gid.onlift = -1;
    }
  }
}

void animatesprings( void )
{
  int i;
  for( i=0; springs[i].x!=-1; i++ )
  {
    if( springs[i].rtime > 0 )
    {
      springs[i].rtime--;
      continue;
    }
    if( ( i != gid.onspring ) && ( springs[i].frame != 0 ) )
      springs[i].frame = 0;
  }
}

void movestars( void )
{
  int i;
 
  for( i=0; i<MAX_STARS; i++ )
  {
    if( stars[i].frame < 23 )
    {
      stars[i].x += stars[i].dx;
      stars[i].y += stars[i].dy;
      if( stars[i].framecount < stars[i].framespeed )
      {
        stars[i].framecount++;
        continue;
      }
      stars[i].frame++;
      stars[i].framecount = 0;
    }
  }
}

void timeincidentals( void )
{
  int i;
 
  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( incd[i].frame < incd[i].numframes )
    {
      incd[i].x += incd[i].xm;
      incd[i].y += incd[i].ym;
      if( incd[i].framecount < incd[i].framespeed )
      {
        incd[i].framecount++;
        continue;
      }
      incd[i].frame++;
      incd[i].framecount = 0;
    }
  }

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( bincd[i].frame < bincd[i].numframes )
    {
      bincd[i].x += bincd[i].xm;
      bincd[i].y += bincd[i].ym;
      if( bincd[i].framecount < bincd[i].framespeed )
      {
        bincd[i].framecount++;
        continue;
      }
      bincd[i].frame++;
      bincd[i].framecount = 0;
    }
  }
}

void drawlifts( BOOL behind )
{
  int i, j;
  for( i=0; lifts[i].hdr.numstops!=-1; i++ )
  {
    if( behind != lifts[i].hdr.behind ) continue;
    glBindTexture( GL_TEXTURE_2D, tex[lifts[i].hdr.wtex] );

    if( lifts[i].hdr.fromstop == -1 )
    {
      for( j=0; j<lifts[i].hdr.numstops; j++ )
      {
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s, lifts[i].hdr.stops[j*2]-fgx, (lifts[i].hdr.stops[j*2+1]-fgy)+lifts[i].hdr.bobo+lifts[i].hdr.dip, FALSE, 0, lifts[i].hdr.scale );
      }
      continue;
    }

    switch( lifts[i].hdr.type )
    {
      case LT_BUBBLE:
        render_sprite_scaled( &sprs[((frame>>4)&1)+40], (lifts[i].hdr.cx>>8)-fgx, ((lifts[i].hdr.cy>>8)-fgy)+lifts[i].hdr.bobo+lifts[i].hdr.dip, FALSE, 0, lifts[i].hdr.scale );
        break;
      
      case LT_FRETURN:
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s+((frame>>1)&1), (lifts[i].hdr.cx>>8)-fgx, ((lifts[i].hdr.cy>>8)-fgy)+lifts[i].hdr.bobo+lifts[i].hdr.dip, FALSE, 0, lifts[i].hdr.scale );
        break;
      
      case LT_GIRDER:
        {
          int x, y, k;
          x = (lifts[i].hdr.cx>>8)-fgx;
          y = ((lifts[i].hdr.cy>>8)-fgy)+lifts[i].hdr.bobo+lifts[i].hdr.dip;
          for( k=(y-57); k > -16; k -=16 )
            render_sprite_tl( &sprt4[32], x-3, k, FALSE );
          render_sprite_tl( &sprt4[31], x-10, y-41, FALSE );
          if( lifts[i].s ) render_sprite_scaled( lifts[i].s, x, y, FALSE, 0, lifts[i].hdr.scale );
        }
        break;

      default:
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s, (lifts[i].hdr.cx>>8)-fgx, ((lifts[i].hdr.cy>>8)-fgy)+lifts[i].hdr.bobo+lifts[i].hdr.dip, FALSE, 0, lifts[i].hdr.scale );
        break;
    }
  }
}

void drawsprings( void )
{
  int i;
  for( i=0; springs[i].x!=-1; i++ )
  {
    glBindTexture( GL_TEXTURE_2D, tex[springs[i].wtex] );
    render_sprite_tl( &springs[i].sl[springs[i].frames[springs[i].frame]], springs[i].x-fgx, springs[i].y-fgy, FALSE );
  }
}

void drawstars( void )
{
  int i;
  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  for( i=0; i<MAX_STARS; i++ )
  {
    if( stars[i].frame < 23 )
      render_sprite( &sprtg[stars[i].frame], (stars[i].x>>8)-fgx, (stars[i].y>>8)-fgy, FALSE, 0 );
  }
}

void drawincidentals( void )
{
  int i;
  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( incd[i].frame < incd[i].numframes )
    {
      glBindTexture( GL_TEXTURE_2D, tex[incd[i].wtex] );
      render_sprite( &incd[i].s[incd[i].frames[incd[i].frame]], incd[i].x-fgx, incd[i].y-fgy, FALSE, 0 );
    }
  }
}

void drawbgincidentals( void )
{
  int i;
  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( bincd[i].frame < bincd[i].numframes )
    {
      glBindTexture( GL_TEXTURE_2D, tex[bincd[i].wtex] );
      render_sprite( &bincd[i].s[bincd[i].frames[bincd[i].frame]], bincd[i].x-bgx, bincd[i].y-bgy, FALSE, 0 );
    }
  }
}

void render_coincount( void )
{
  int x;

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  render_sprite_tl( &sprtg[29], 263, 6, FALSE );
  x = 286;
  if( gid.coins >= 100 ) { render_sprite_tl( &sprtg[30+(gid.coins/100)], x, 12, FALSE ); x+=11; }
  if( gid.coins >= 10 )  { render_sprite_tl( &sprtg[30+((gid.coins/10)%10)], x, 12, FALSE ); x+=11; }
  render_sprite_tl( &sprtg[30+(gid.coins%10)], x, 12, FALSE );
}

void render_lives( void )
{
  int i, j;

  j = gid.lives;
  if( j < 0 ) j = 0;

  if( ( clevel == 4 ) && ( gid.hardhat ) )
  {
    glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
    render_sprite_tl( &sprs[27], 4,  2, FALSE );
    render_sprite_tl( &sprs[28], 4, 26, FALSE );
  } else {
    glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
    render_sprite_tl( &sprs[6], 4, 2, FALSE );
  }

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  render_sprite_tl( &sprtg[30+(j%10)], 46, 3, FALSE );

  for( i=0; i<gid.energy; i++ )
    render_sprite_tl( &sprtg[18], 50+i*20, 15, FALSE );
}

void render_invbox( void )
{
  float tl, tt, tr, tb;
  float vl, vt, vr, vb;
  int x2;
  GLfloat vtx[4*2], txq[4*2];

  glBindTexture( GL_TEXTURE_2D, tex[TEXTTEX] );
  glLoadIdentity();
  glTranslatef( 160, 26.0f, 0.0f );
  glRotatef( ibrot, 0.0f, 0.0f, 1.0f );
  glScalef( ibscale, ibscale, 1.0f );
    glColor4ub( 255, 255, 255, ibalpha );

    set_quad_tristrip( vtx, -92, -22.5f, 92, 22.5f );
    set_texc_tristrip( txq, 0.0f, 0.0f, 0.71875f, 0.17578125f );
    glVertexPointer( 2, GL_FLOAT, 0, vtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, txq );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if( ibw[0] > 0.0f )
    {
      vl = -ibw[0]/2.0f; tl = 0.0f;
      vt = -10.0f;       tt = (128.0f/256.0f);
      vr = ibw[0]/2.0f;  tr = ibtw[0];
      vb = 0.0f;         tb = tt + (10.0f/256.0f);

      set_quad_tristrip( vtx, vl, vt, vr, vb );
      set_texc_tristrip( txq, tl, tt, tr, tb );
      glVertexPointer( 2, GL_FLOAT, 0, vtx );
      glTexCoordPointer( 2, GL_FLOAT, 0, txq );
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if( ( ibw[1] > 0.0f ) &&
        ( ibpos < 81 ) &&
        ( (ibpos+ibw[1]) > -81 ) )
    {
      if( ibpos >= -81 )
      { 
        vl = ibpos; tl = 0.0f;
      } else {
        vl = -81.0f; tl = (-81.0f-(float)ibpos)/256.0f;
      }

      if( (ibpos+ibw[1]) < 81 )
      {
        vr = ibpos+ibw[1]; tr = ibtw[1];
      } else {
        vr = 81.0f; tr = (ibw[1] - (float)((ibpos+ibw[1])-81)) / 256.0f;
      }

      vt = 0.0f; tt = (139.0f/256.0f);
      vb = 10.0f; tb = tt + (10.0f/256.0f);
      set_quad_tristrip( vtx, vl, vt, vr, vb );
      set_texc_tristrip( txq, tl, tt, tr, tb );
      glVertexPointer( 2, GL_FLOAT, 0, vtx );
      glTexCoordPointer( 2, GL_FLOAT, 0, txq );
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    x2 = ibpos+iboff;
    if( ( ibw[2] > 0.0f ) &&
        ( x2 < 81 ) &&
        ( (x2+ibw[2]) > -81 ) )
    {
      if( x2 >= -81 )
      { 
        vl = x2; tl = 0.0f;
      } else {
        vl = -81.0f; tl = (-81.0f-(float)x2)/256.0f;
      }

      if( (x2+ibw[2]) < 81 )
      {
        vr = x2+ibw[2]; tr = ibtw[2];
      } else {
        vr = 81.0f; tr = (ibw[2] - (float)((x2+ibw[2])-81)) / 256.0f;
      }

      vt = 0.0f; tt = (150.0f/256.0f);
      vb = 10.0f; tb = tt + (10.0f/256.0f);
      set_quad_tristrip( vtx, vl, vt, vr, vb );
      set_texc_tristrip( txq, tl, tt, tr, tb );
      glVertexPointer( 2, GL_FLOAT, 0, vtx );
      glTexCoordPointer( 2, GL_FLOAT, 0, txq );
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void special_animations( void )
{
  switch( clevel )
  {
    case 1:
      if( !strig[ST1_TREE_1UP] )
      {
        if( ( gid.py <= 270 ) &&
            ( gid.px >= 1320 ) &&
            ( gid.px < 1400 ) )
          strig[ST1_TREE_1UP] = 1;
        break;
      }

      if( ( g_things[1]->active ) &&
          ( g_things[1]->y < 184 ) )
        g_things[1]->y+=6;
      break;
    
    case 4:
      if( !strig[ST4_POLE_1UP] )
      {
        if( ( gid.py <= 278 ) &&
            ( gid.px >= 3280 ) &&
            ( gid.px < 3440 ) )
          strig[ST4_POLE_1UP] = 1;
        break;
      }

      if( ( g_things4[9].active ) &&
          ( g_things4[9].y < 232 ) )
      {
        g_things4[9].y+=6;
        if( g_things4[9].y > 232 )
          g_things4[9].y = 232;
      }
      break;
  }
}

void animate_thingy( struct thingy *t )
{
  if( ( !t->active ) || ( t->numframes < 2 ) )
    return;
  
  if( t->framecount < t->frametime )
  {
    t->framecount++;
    return;
  }

  t->framecount = 0;
  t->frame = (t->frame+1)%t->numframes;
}

void you_now_have( char *what )
{
  set_ibstr( 0, "You now have" );
  set_ibstr( 1, what );
  set_ibstr( 2, NULL );
  ibpos = 82;

  if( (strlen(what)*8) > 194 )
  {
    ibadd  = -2;
    ibwait = 160;
    ibdest = -(strlen(what)*8+82);
  } else {
    ibadd = -4;
    ibwait = 60;
    ibdest = -strlen(what)*4;
  }

  ibstate = 4;
}

void collect_nthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, sprs, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

void collect_pthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, psprs, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    switch( clevel )
    {
      case 1:
        switch( t->frames[0] )
        {
          case 24: // Whacking great bomb
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BOMB] = 1;
            you_now_have( ivtexts[INV_BOMB].pickup );
            strig[ST1_BOMB_COLLECTED] = 1;
            actionsound( SND_ITEMGET, sfxvol );
            break;
            
          case 31: // Boot
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BOOT] = 1;
            strig[ST1_BOOT_COLLECTED] = 1;
            you_now_have( ivtexts[INV_BOOT].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;

          case 35: // Barrel
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BARREL] = 1;
            you_now_have( ivtexts[INV_BARREL].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 2:
        switch( t->frames[0] )
        {
          case 69: // Control Box
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_CONTROLBOX] = 1;
            you_now_have( ivtexts[INV_CONTROLBOX].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
          
          case 74: // Printed photo
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_PRINTEDPHOTO] = 1;
            you_now_have( ivtexts[INV_PRINTEDPHOTO].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 4:
        switch( t->frames[0] )
        {
          case 27: // Coggy the cog
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_LARGECOG] = 1;
            you_now_have( ivtexts[INV_LARGECOG].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
          
          case 36: // Battery
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_FLATBATTERY] = 1;
            you_now_have( ivtexts[INV_FLATBATTERY].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 5:
        switch( t->frames[0] )
        {
          case 57: // Charged battery
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            p_things5[19].active = FALSE;
            strig[ST5_BATTERY_COLLECTED] = 1;
            inv[INV_CHARGEDBATTERY] = 1;
            you_now_have( ivtexts[INV_CHARGEDBATTERY].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
    }

    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

void collect_gthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, sprtg, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    switch( t->frames[0] )
    {
      case 10: // Coin?
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.coins++;
        inv[INV_COINS] = 1;
        playsound( C_COIN, SND_COIN, sfxvol );
        break;
      
      case 41: // Plank
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_PLANK] = 1;
        you_now_have( ivtexts[INV_PLANK].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 64: // Hosepipe
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_HOSEPIPE] = 1;
        you_now_have( ivtexts[INV_HOSEPIPE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 65: // Air Horn
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_AIRHORN] = 1;
        you_now_have( ivtexts[INV_AIRHORN].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 66: // Candle
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CANDLESTICK] = 1;
        you_now_have( ivtexts[INV_CANDLESTICK].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 72: // Plunger
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DETONATOR] = 1;
        you_now_have( ivtexts[INV_DETONATOR].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 73: // Diamond
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DIAMOND] = 1;
        you_now_have( ivtexts[INV_DIAMOND].pickup );
        strig[ST4_DIAMOND_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 74: // Hard hat
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_HARDHAT] = 1;
        you_now_have( ivtexts[INV_HARDHAT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 75: // Mirror
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_MIRROR] = 1;
        you_now_have( ivtexts[INV_MIRROR].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 76: // Turps
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_TURPS] = 1;
        you_now_have( ivtexts[INV_TURPS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        
      
      case 77: // Electrical toolkit
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_ELECTRICALTOOLKIT] = 1;
        you_now_have( ivtexts[INV_ELECTRICALTOOLKIT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 70: // Lit candle
      case 78:
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LIGHTEDCANDLE] = 1;
        strig[ST3_CANDLE_COLLECTED] = 1;
        g_things3[12].active = FALSE;
        g_things3[13].active = FALSE;
        you_now_have( ivtexts[INV_LIGHTEDCANDLE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 80: // Lump of carbon
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LUMPOFCARBON] = 1;
        you_now_have( ivtexts[INV_LUMPOFCARBON].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 81: // Scissors
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_SCISSORS] = 1;
        you_now_have( ivtexts[INV_SCISSORS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 82: // Bubble gum
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_BUBBLEGUM] = 1;
        you_now_have( ivtexts[INV_BUBBLEGUM].pickup );
        strig[ST3_GUM_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 83: // Camera
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DIGITALCAMERA] = 1;
        you_now_have( ivtexts[INV_DIGITALCAMERA].pickup );
        strig[ST2_CAMERA_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 84: // Lard
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LARD] = 1;
        you_now_have( ivtexts[INV_LARD].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 86: // Chalkie
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_INDIGESTIONPILLS] = 1;
        you_now_have( ivtexts[INV_INDIGESTIONPILLS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 87: // Catapult
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CATAPULT] = 1;
        you_now_have( ivtexts[INV_CATAPULT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 88: // CD
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CD] = 1;
        you_now_have( ivtexts[INV_CD].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 92:
      case 93: // Spade
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_SPADE] = 1;
        g_things4[5].active = FALSE;
        g_things4[6].active = FALSE;
        you_now_have( ivtexts[INV_SPADE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 97: // Weather balloon wreckage
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_BALLOONWRECKAGE] = 1;
        you_now_have( ivtexts[INV_BALLOONWRECKAGE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 98: // Teleporter watch
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_TELEPORTERWATCH] = 1;
        you_now_have( ivtexts[INV_TELEPORTERWATCH].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 99: // 1UP
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.lives++;
        giddy_say( "Huzzah! Eggstra life!" );
        actionsound( SND_1UP, sfxvol );
        break;
      
      case 100: // Burger
      case 102: // Ice cream
      case 103: // Fairy cake
      case 104: // Radish
      case 105: // Cheese
        if( gid.energy >= 3 ) return;
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.energy++;
        giddy_say( "Chomp!" );
        actionsound( SND_CHOMP, sfxvol );
        break;
    }

    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

Uint32 timing( Uint32 interval, void *dummy )
{
  int i;
//  SDL_Event     event;
//  SDL_UserEvent userevent;

//  userevent.type  = SDL_USEREVENT;
//  userevent.code  = 0;
//  userevent.data1 = NULL;
//  userevent.data2 = NULL;
  
//  event.type = SDL_USEREVENT;
//  event.user = userevent;
  
//  SDL_PushEvent( &event );

  if( fadeadd > 0 )
  {
    fadea += fadeadd;
    if( fadea > 255 )
    {
      fadea = 255;
      fadeadd = 0;
    }
  } else if( fadeadd < 0 ) {
    fadea += fadeadd;
    if( fadea < 0 )
    {
      fadea = 0;
      fadeadd = 0;
    }
  }

  switch( what_are_we_doing )
  {
    case WAWD_TITLES:
    case WAWD_MENU:
    case WAWD_DEFINE_A_KEY:
      title_timing();
      return interval;
	
	  case WAWD_ENDING:
			ending_timing();
		  return interval;
  }

  if( gamepause )
  {
    animate_giddyspeak();
    return interval;
  }

	if( infobulbalpha > 0 )
	{
		infobulbalpha-=7;
		if( infobulbalpha < 0 )
			infobulbalpha = 0;
		infobulbzoom += 0.08f;
	}

  if( enterhit )
  {
    if( winfo != -1 )
    {
      if( gid.speakstate == 0 )
      {
        giddy_say( infos[winfo].txt );
        actionsound( SND_SPEECHBUB, sfxvol );
      } else {
        gid.speakc = 0;
      }
    }
    enterhit = FALSE;
  }

  if( ( gjump ) && ( gid.usemode ) )
  {
    gjump = FALSE;
    spacehit = TRUE;
    ignorejump = TRUE;
  }

  if( spacehit )
  {
    switch( ibstate )
    {
      case 0:
        if( ( gid.jumping ) || ( !gid.allowjump ) || ( gjump ) )
          break;
        set_ibstr( 0, "** Use **" );
        if( lastinv == -1 ) lastinv = 0;
        for( i=0; i<INV_LAST; i++ )
          if( inv[(i+lastinv)%INV_LAST] ) break;
        if( i<INV_LAST )
        {
          ibitem = (i+lastinv)%INV_LAST;
          set_ibstr( 1, ivtexts[ibitem].use );
          set_ibstr( 2, NULL );
          ibpos = 82;
          ibdest = -strlen(ivtexts[ibitem].use)*4;
          ibadd = -4;
          ibstate++;
        } else {
          ibitem = -1;
          set_ibstr( 1, "Scotch mist?" );
          set_ibstr( 2, NULL );
          ibpos = 82;
          ibdest = -12*4;
          ibadd = -4;
          ibstate++;
        }
        actionsound( SND_USE, sfxvol );
        break;

      case 2:
        if( ibadd != 0 ) break;

        lastinv = ibitem;

        ibstate++;

        if( ( ibitem >= 0 ) && ( inv[ibitem] != 0 ) )
        {
          BOOL complain;

          complain = TRUE;
          switch( ibitem )
          {
            case INV_COINS:
              if( ( clevel == 4 ) && ( winfo == 3 ) )
              {
                giddy_say( "Who ya gonna call?" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggergummachine() ) complain = FALSE;
              break;

            case INV_BARREL:
              if( ( clevel == 1 ) && ( winfo == 2 ) )
              {
                giddy_say( "I fear it would only\n"
                           "fuel his depression." );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggerslug() ) complain = FALSE;
              break;
            
            case INV_LARD:
              if( triggersludgemonster() ) complain = FALSE;
              break;
            
            case INV_BUBBLEGUM:
              if( triggerburstpipe() ) complain = FALSE;
              break;
            
            case INV_ELECTRICALTOOLKIT:
              if( triggerphonebox() ) complain = FALSE;
              break;
            
            case INV_BOOT:
              if( triggerjunkchuteswitcheroo() ) complain = FALSE;
              break;
            
            case INV_DETONATOR:
              if( placedetonator() ) complain = FALSE;
              break;
            
            case INV_CATAPULT:
              if( ( clevel == 3 ) && ( winfo == 2 ) )
              {
                giddy_say( "Thats not very nice!" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggerballoon() ) complain = FALSE;
              break;
            
            case INV_BALLOONWRECKAGE:
              if( triggermuldoonandskelly() ) complain = FALSE;
              break;
            
            case INV_ATEAMPHONENO:
              if( triggerateamvan() ) complain = FALSE;
              break;
            
            case INV_PLANK:
              if( ( clevel == 3 ) && ( winfo == 2 ) )
              {
                giddy_say( "Thats not very nice!" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( placeplank() ) complain = FALSE;
              break;
            
            case INV_SCISSORS:
              if( triggerseesaw() ) complain = FALSE;
              break;
            
            case INV_CHARGEDBATTERY:
              if( triggertripledoors() ) complain = FALSE;
              break;
            
            case INV_TELEPORTERWATCH:
              complain = FALSE;
              if( clevel == 5 )
              {
                giddy_say( "Apparently, it only\n"
                           "works from outside." );
                actionsound( SND_USEFAIL, sfxvol );
                break;
              }

              gid.sgwsc = 1.0f;
              gid.wwa = 255;
              gid.wwl = 5;
              gid.watchwarp = TRUE;
              gid.def = 1;
              incidentalsound( SND_TELEPORT_OUT, sfxvol );
              break;
            
            case INV_DIGITALCAMERA:
              if( triggerbigscreen1() ) complain = FALSE;
              break;

            case INV_PRINTEDPHOTO:
              if( triggerbigscreen2() ) complain = FALSE;
              break;

            case INV_INDIGESTIONPILLS:
              if( triggerbiledude() ) complain = FALSE;
              break;
            
            case INV_CD:
              if( triggereel() ) complain = FALSE;
              break;
            
            case INV_LARGECOG:
              if( triggerrecyclotron() ) complain = FALSE;
              break;
            
            case INV_LUMPOFCARBON:
              if( triggercrusher() ) complain = FALSE;
              break;

            case INV_AIRHORN:
              if( honkatdragon() ) complain = FALSE;
              break;
            
            case INV_TURPS:
              if( drinkfordragon() ) complain = FALSE;
              break;
            
            case INV_CANDLESTICK:
              if( triggerdragon() ) complain = FALSE;
              break;
            
            case INV_LIGHTEDCANDLE:
              if( triggertorches() ) complain = FALSE;
              break;
            
            case INV_DIAMOND:
              if( triggerlockblockzapper() ) complain = FALSE;
              break;
            
            case INV_FLATBATTERY:
              if( ( clevel == 5 ) && ( winfo == 0 ) )
              {
                giddy_say( "Err, the battery is flat." );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggercyclingalien() ) complain = FALSE;
              break;
            
            case INV_HOSEPIPE:
              if( triggersprinkler() ) complain = FALSE;
              break;
            
            case INV_CONTROLBOX:
              if( triggerpluggrabber() ) complain = FALSE;
              break;
            
            case INV_CAMERAWITHPHOTO:
              if( triggerprinter() ) complain = FALSE;
              break;
            
            case INV_HARDHAT:
              if( triggerbuilder() ) complain = FALSE;
              break;
            
            case INV_MIRROR:
              if( triggerlaserbeam() ) complain = FALSE;
              break;
            
            case INV_SPADE:
              if( triggerwhackinggreatbomb() ) complain = FALSE;
              break;
            
            case INV_BOMB:
              if( triggereggsterminatorproductionline() ) complain = FALSE;
              break;
          }

          if( complain )
          {
            if( winfo == -1 )
              giddy_say( "What? Where?" );
            else
              giddy_say( "Perhaps not." );
            actionsound( SND_USEFAIL, sfxvol );
          }
        }
        break;
    }
    spacehit = FALSE;
  }

  frame++;

  if( ibadd != 0 )
  {
    ibpos += ibadd;
    if( ibadd < 0 )
    {
      if( ibpos <= ibdest )
      {
        ibpos = ibdest;
        ibadd = 0;
        if( ibt[2] != NULL )
        {
          set_ibstr( 1, ibt[2] );
          set_ibstr( 2, NULL );
          ibpos += iboff;
        }
      }
    } else {
      if( ibpos >= ibdest )
      {
        ibpos = ibdest;
        ibadd = 0;
      }
    }
  }

  animate_giddyspeak();
  movestars();
  timeincidentals();
  special_animations();
  animatesprings();
  make_giddy_do_things();

  if( ( clevel == 1 ) &&
      ( gid.x >= (2848<<8) ) &&
      ( strig[ST1_HOSE_PLACED] == 1 ) &&
      ( strig[ST1_BOSS_BEATEN] == 0 ) &&
      ( stuff.bossmode == 0 ) )
    triggerpotatoboss();

  animateenemies( enemies[clevel-1] );
  animateenemies( enemiesb[clevel-1] );
  movelifts();
  triggerfallingblocks();
  dospecialfade();

  for( i=0; i<num_nthings; i++ )
  {
    animate_thingy( n_things[i] );
    collect_nthingy( n_things[i] );
  }
  for( i=0; i<num_bnthings; i++ )
  {
    animate_thingy( bn_things[i] );
    collect_nthingy( bn_things[i] );
  }
  for( i=0; i<num_pthings; i++ )
  {
    animate_thingy( p_things[i] );
    collect_pthingy( p_things[i] );
  }
  for( i=0; i<num_bpthings; i++ )
  {
    animate_thingy( bp_things[i] );
    collect_pthingy( bp_things[i] );
  }
  for( i=0; i<num_gthings; i++ )
  {
    animate_thingy( g_things[i] );
    collect_gthingy( g_things[i] );
  }
  for( i=0; i<num_bgthings; i++ )
  {
    animate_thingy( bg_things[i] );
    collect_gthingy( bg_things[i] );
  }

  switch( clevel )
  {
    case 1:
      animateslug();
      animateeel();
      animatesprinkler();
      break;
    
    case 2:
      animatejunkchute();
      animatesludgemonster();
      animateburstpipe();
      animatetoxicgas();
      animatepluggrabber();
      animaterecyclotron();
      animateprinter();
      break;
    
    case 3:
      animatetardis();
      animatemuldoonandskelly();
      animateboulder();
      animatedragon();
      animateballoon();
      animateseesaw();
      animatefallingblocks();
      animatestargate();
      animateboulder2();
      animatetimerdoor();
      animatelockblockzapper();
      animatewallsteppingstones();
      animategummachine();
      break;
    
    case 4:
      animatemrt();
      animatetardis();
      animatefactory();
      animatebuilder();
      animatecrusher();
      animateninja();
      animatespecialbin();
      break;
    
    case 5:
      animatetripledoors();
      animatebigassfan();
      animatelaserbeam();
      animatebigscreen();
      animatetpbubs();
      animatebiledude();
      animatecyclingalien();
      animateeggsterminatorproductionline();
      break;
  }

  scrollhandler();
  animate_invbox();
  check_infos();

  switch( clevel )
  {
    case 1:
    	//TEST demo screen
/*
#ifdef __demo__
    	if( ( gid.px > 2000 ) &&
    		( clevel == llevel ) )
    	{
    		if (!gid.demofinished) //Dont reset
    		{
    			gid.demofinished = TRUE;
    			gid.demofinishedstate = 0;
    			gid.demofinishedfade = 0;
    		}
    	}
#endif
*/
      if( ( gid.px > 6304 ) &&
          ( gid.py < 420 ) &&
          ( clevel == llevel ) )
      {
        llevel = 2;
        fadea = 0;
        fadeadd = 8;
        fadetype = 0;
        gid.stopuntilfade = TRUE;
      }
      break;
    
    case 2:
      if( ( gid.px < 32 ) &&
          ( gid.py >= 1120 ) &&
          ( clevel == llevel ) )
      {
        llevel = 0x8001;
        fadea = 0;
        fadetype = 0;
        fadeadd = 8;
        gid.stopuntilfade = TRUE;
      }

      if( ( gid.px >= 3648 ) &&
          ( gid.px < 3696 ) &&
          ( gid.py < 440 ) &&
          ( clevel == llevel ) )
      {

#ifdef __demo__
    	  if (!gid.demofinished) //Dont reset
    	  {
    		  gid.demofinished = TRUE;
    		  gid.demofinishedstate = 0;
    		  gid.demofinishedfade = 0;
    	  }
#else
        llevel = 4;
        fadea = 0;
        fadetype = 0;
        fadeadd = 8;
        gid.fallpuffs = TRUE;
#endif
      }
      break;
  }

  return interval;
}

BOOL render_init( void )
{
  SDL_VideoInfo *info = (SDL_VideoInfo *)SDL_GetVideoInfo();
  int x, y, i;
  float sc;

  SDL_ShowCursor( SDL_DISABLE );

  touchx[0] = -1;
  touchx[1] = -1;

  /* Landscape or portrait default? */
  if( info->current_w > info->current_h )
  {
    touchscalex = (320*256) / info->current_w;
    touchscaley = (240*256) / info->current_h;
    vieww = info->current_w;
    viewh = info->current_h;
    rotated = 0;
  } else {
    touchscalex = (320*256) / info->current_h;
    touchscaley = (240*256) / info->current_w;
    vieww = info->current_h;
    viewh = info->current_w;
    rotated = 1;
  }
  cardw = info->current_w;
  cardh = info->current_h;

  glMatrixMode( GL_PROJECTION );
  if( !rotated )
  {
    glOrthof( 0.0f, 320.0f, 240.0f, 0.0f, 0.0f, 1.0f );
#ifdef __android__
    set_texc_tristrip = bodge_texc_tristrip;
    set_texc_tri      = bodge_texc_tri;
#else
    set_texc_tristrip = set_quad_tristrip;
    set_texc_tri      = set_quad_tri;
#endif
  } else {
    glOrthof( -120.0f, 120.0f, 160.0f, -160.0f, 0.0f, 1.0f );
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-160.0f, -120.0f, 0.0f);
    set_texc_tristrip = bodge_texc_tristrip;
    set_texc_tri      = bodge_texc_tri;
  }
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glDisable( GL_DEPTH_TEST );

  i=0; sc = 1.0f/16.0f;
  for( y=0; y<16; y++ )
    for( x=0; x<16; x++ )
    {
      bt[i].x = ((float)x) * sc;
      bt[i].y = ((float)y) * sc;
      bt[i].w = 1.0f * sc;
      bt[i].h = 1.0f * sc;
      i++;
    }

  glGenTextures( TEX_LAST, tex );

  glBindTexture( GL_TEXTURE_2D, tex[TVTEX] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, tvborders );

  screentex = &texts[256*256*4];
  glBindTexture( GL_TEXTURE_2D, tex[SCREENTEX]);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, screentex );

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  return TRUE;
}

void render_shut( void )
{
}



void render_fade( void )
{
  GLfloat vtx[4*2], txq[4*2];
  lrfade2 = lrfade1;
  lrfade1 = fadea;
  if( fadea == 0 ) return;

  glLoadIdentity();

  if( fadetype == 1 )
  {
    glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
    glColor4ub( 255, 255, 255, fadea );

    set_quad_tristrip( vtx, 0.0f, 0.0f, 320.0f, 240.0f );
    set_texc_tristrip( txq, 232.0f/256.0f, 135.0f/256.0f, 254.0f/256.0f, 152.0f/256.0f );

    glVertexPointer( 2, GL_FLOAT, 0, vtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, txq );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    return;
  }

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  glColor4ub( 255, 255, 255, fadea );
  set_quad_tristrip( vtx, 0.0f, 0.0f, 320.0f, 240.0f );
  set_texc_tristrip( txq, 0.0f, 0.0f, 56.0f/256.0f, 62.0f/256.0f );

  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

#ifdef __demo__
void render_demoend( void )
{
	float extrax, extray;
	GLfloat vtx[4*2], txq[4*2];

	if( !gid.demofinished ) return;


	glLoadIdentity();
	glBindTexture( GL_TEXTURE_2D, tex[DEMOENDTEX] );
	//glTranslatef( 160.0f, 100.0f, 0.0f );
	glColor4ub( 255, 255, 255, gid.demofinishedfade );
	set_quad_tristrip( vtx, 0.0f, 0.0f, 320.0f, 240.0f );
	set_texc_tristrip( txq, 0.0f, 0.0f, 1.0f, 1.0f );

	glVertexPointer( 2, GL_FLOAT, 0, vtx );
	glTexCoordPointer( 2, GL_FLOAT, 0, txq );
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
#endif

void render_spoonedit( void )
{
  float extrax, extray;
  GLfloat vtx[4*2], txq[4*2];

  if( !gid.spoonedit ) return;

  extrax = sin(gid.spooneditwobble)*gid.spooneditwobblefactor*2.0f;
  extray = cos(gid.spooneditwobble+3.14159265f)*gid.spooneditwobblefactor;

  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );  
  glTranslatef( 160.0f, 100.0f, 0.0f );
  glColor4ub( 255, 255, 255, gid.spooneditfade );
  set_quad_tristrip( vtx, -130.0f-extrax, -18.0f-extray,  130.0f+extrax,  18.0f+extray );
  set_texc_tristrip( txq, 101.0f/256.0f, 0.0f, 231.0f/256.0f, 17.6f/256.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

extern struct wasp wasps1[];

extern float pk_ang;

void render_joystick( void )
{
  GLfloat vtx[4*2], txq[4*2];
  int spread = (vieww < 1024) ? 40 : 32;  // Spread the buttons out a bit on phone screens

  // Arrows
  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[STICKLTEX] );  
  glTranslatef( 14.0f, 216.0f, 0.0f );
  glColor4ub( 255, 255, 255, 100 );
  set_quad_tristrip( vtx, -10.0f, -10.0f, 10.0f, 10.0f );
  set_texc_tristrip( txq, 0.0f, 0.0f, 1.0f, 1.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[STICKRTEX] );  
  glTranslatef( 68.0f, 216.0f, 0.0f );
  glColor4ub( 255, 255, 255, 100 );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Joystick
  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[STICKTEX] );  
  glTranslatef( 40.0f+touchjoyx, 216.0f, 0.0f );
  glColor4ub( 255, 255, 255, 140 );
  set_quad_tristrip( vtx, -16.0f, -16.0f, 16.0f, 16.0f );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Jump button
  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[JUMPTEX] );  
  glTranslatef( 298.0f, 216.0f, 0.0f );
  glColor4ub( 255, 255, 255, 140 );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glLoadIdentity();
  glBindTexture( GL_TEXTURE_2D, tex[USETEX] );  
  glTranslatef( 298.0f - spread, 216.0f, 0.0f );
  glColor4ub( 255, 255, 255, 140 );
  glVertexPointer( 2, GL_FLOAT, 0, vtx );
  glTexCoordPointer( 2, GL_FLOAT, 0, txq );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  if( winfo != -1 )
  {
    glLoadIdentity();
    glBindTexture( GL_TEXTURE_2D, tex[HINTTEX] );  
    glTranslatef( 298.0f - spread*2, 216.0f, 0.0f );
    glColor4ub( 255, 255, 255, 140 );
    glVertexPointer( 2, GL_FLOAT, 0, vtx );
    glTexCoordPointer( 2, GL_FLOAT, 0, txq );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

BOOL render( void )
{
  int i;//, j, mx, my;
//  int bmx, bmy;
 
  if( gid.savedtheday )
  {
    killtune();
    what_are_we_doing = WAWD_ENDING;
		endingstate = 0;
    return FALSE;
  }

  //Stay stuck in end state, player needs to start a new game to reset.
/*
#ifdef __demo__
  if( ( gid.demofinished ) && ( gid.demofinishedstate == 2 ) )
  {
	  killtune();
	  titlestate = 0;
	  what_are_we_doing = WAWD_TITLES;
	  return FALSE;
  }
#endif
*/
  if( ( gid.spoonedit ) && ( gid.spooneditstate == 3 ) )
  {
    killtune();
    titlestate = 0;
    what_are_we_doing = WAWD_TITLES;
    return FALSE;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if( ( llevel != clevel ) && ( fadeadd == 0 ) && ( lrfade2 == 255 ) )
  {
    save_game();
    killtune();
    load_level( llevel, FALSE );
    gid.ontardis = NOT_ON_TARDIS;
    fadeadd = -8;
    gid.stopuntilfade = FALSE;
    gid.mainchryoffset = 0;
  }

  update_ibstrs();
/*
#ifdef __amigaos4__
  if( screenbodge ) updatebodge();
#endif
*/
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );

  render_background();

  switch( clevel )
  {
    case 4:
      renderfactory();
      renderateamvan();
      break;
  }

  drawbgincidentals();

  glBindTexture( GL_TEXTURE_2D, tex[PSPRITEX] );
  for( i=0; i<num_bpthings; i++ )
    render_pthing( bp_things[i] );

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  for( i=0; i<num_bgthings; i++ )
    render_gthing( bg_things[i] );

  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  for( i=0; i<num_bnthings; i++ )
    render_nthing( bn_things[i] );

  drawlifts( TRUE );

  switch( clevel )
  {
    case 1:
      renderslug();
      renderboss();
      break;

    case 2:
      renderrecyclotron();
      rendersludgemonster();
      rendertoxicgas();
      renderpluggrabberbg();
      break;
    
    case 3:
      renderseesawbg();
      break;
    
    case 4:
      rendercrusher();
      renderninjabg();
      renderspecialbin();
      break;
    
    case 5:
      rendertripledoorsbg();
      renderbigscreen();
      rendertpbubs();
      rendereggsterminatorproductionlinebg();
      break;
  }

  renderenemies( enemiesb[clevel-1] );

  render_foreground();

  glBindTexture( GL_TEXTURE_2D, tex[PSPRITEX] );
  for( i=0; i<num_pthings; i++ )
    render_pthing( p_things[i] );

  glBindTexture( GL_TEXTURE_2D, tex[GSPRITEX] );
  for( i=0; i<num_gthings; i++ )
    render_gthing( g_things[i] );

  glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
  for( i=0; i<num_nthings; i++ )
    render_nthing( n_things[i] );

  drawincidentals();
  drawlifts( FALSE );
  drawsprings();
  drawstars();

  switch( clevel )
  {
    case 1:
      rendereel();
      rendershootyfmissiles();
      rendersprinkler();
      break;
    
    case 2:
      renderjunkchute();
      rendersludge();
      renderburstpipe();
      renderpluggrabber();
      renderprinter();
      break;
    
    case 3:
      rendertardis();
      rendermuldoonandskelly();
      renderboulder();
      renderdragon();
      renderballoon();
      renderseesaw();
      renderfallingblocks();
      renderstargate();
      renderboulder2();
      renderlockblockzapper();
      rendergummachine();
      break;
    
    case 4:
      rendermrt();
      rendertardis();
      renderbuilder();
      renderninja();
      break;
    
    case 5:
      rendertripledoors();
      renderbigassfan();
      renderlaserbeam();
      renderbiledude();
      rendercyclingalien();
      rendereggsterminatorproductionline();
      break;
  }

  renderenemies( enemies[clevel-1] );

  render_giddy();
  redrawtardisdoor();

  switch( clevel )
  {
    case 1:
      render_foreground_zone( 392, 18, 8, 10 );
      glBindTexture( GL_TEXTURE_2D, tex[SPRITEX] );
      for( i=21; i<=26; i++ )
        render_nthing( n_things[i] );
      break;

    case 3:
      hidefallingblocks();
      glBindTexture( GL_TEXTURE_2D, tex[PSPRITEX] );
      for( i=2; i<=15; i++ )
        render_nthing( n_things[i] );
      break;

    case 4:
      renderfactorybits();
      break;
  }

  render_lives();
  render_coincount();
  render_infobulb();
  render_invbox();

  render_giddyspeak();

#ifdef __demo__
  render_demoend();
#endif

  render_spoonedit();

#ifdef __demo__ //Dont draw sticks if finished
  if (!gid.demofinished)
#endif

  render_joystick();

  render_tvborders();
  render_fade();
/*
#ifdef __amigaos4__
  if( !screenbodge )
    updatebigscreentex();
#else
*/
//#endif

  unlockaudio();
	if( (frame&3) == 0 ) updatebigscreentex();
  SDL_GL_SwapBuffers();
/*
#ifdef __amigaos4__
  if( screenbodge )
    updatebigscreentex();
#endif
*/
/*
  j = -1;
  mx = (mousx+fgx)/16;
  my = (mousy+fgy)/16;
  if( ( mx >= 0 ) && ( mx < mapi->fgw ) &&
      ( my >= 0 ) && ( my < mapi->fgh ) )
    j = fgmap[my*mapi->fgw+mx];

  bmx = (mousx+bgx)/16;
  bmy = (mousy+bgy)/16;

  sprintf( bollocks, "%d,%d - %d,%d - %d,%d - %d,%02x >> %d,%d - %d,%d - %d,%d - %d", 
    mousx+fgx, mousy+fgy,
    mx, my,
    mx*16, my*16,
    j,
    blktrans[j+256],
    mousx+bgx, mousy+bgy,
    bmx, bmy,
    bmx*16, bmy*16,
    gid.onlift );
  SDL_WM_SetCaption( bollocks, 0 );
*/
  return FALSE;
}
