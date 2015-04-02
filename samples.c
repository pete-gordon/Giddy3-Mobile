/*
** Sample data
*/

#include <GLES/gl.h>
#include "SDL.h"
#ifndef __android__
#include "PDL.h"
#else
#include "android.h"
#endif
#include "ptplay.h"

#include "giddy3.h"
#include "samples.h"

#ifdef __android__
extern BOOL use_alt_basepath;
#endif

extern BOOL audioavailable;
//extern BOOL musicenabled;
extern int what_are_we_doing;

#define AUDIO_BUFFERS 3
#define FREQ 44100
#define SNDBUFFERLEN ((FREQ/50)*2*2*2)

struct sample
{
  Sint32 len;
  Sint16 *data;
};

struct sndchan
{
  Sint16 *sample;
  Sint32 len, pos, delta, volume;
  Sint32 loops;
};


#ifdef __android__
Sint32 chanmix[(SNDBUFFERLEN/4)*10]; //Has to be bigger, not sure how big..
#else
Sint32 chanmix[SNDBUFFERLEN/4];
#endif

int /*mixitplease[AUDIO_BUFFERS], nextmixit,*/ lastplayed=-1;

//static volatile Uint32 curr_audio = 0;
//static Uint8 audioBuf[AUDIO_BUFFERS][SNDBUFFERLEN];
BOOL tunePlaying = FALSE;
pt_mod_s *tune = NULL;

struct sndchan sfxchan[C_LAST];
struct sample smp[SND_LAST];

int musicvol = (MIX_MAX_VOLUME*6)/8, musicvolopt = 6;
int sfxvol   = MIX_MAX_VOLUME, sfxvolopt = 8;


void audio_callback( void *dummy, Sint8 *stream, int length )
{
  int i, j;
  Sint16 *out;
  Sint32 v;

  out = (Sint16 *)stream;

	if( ( tune ) && ( tunePlaying ) )
    PtRender( tune, (s8 *)&stream[0], (s8 *)&stream[2], 4, length/4, 2, 16, 2 );
  else
    memset( stream, 0, length );

  memset( chanmix, 0, SNDBUFFERLEN );
  for( i=0; i<C_LAST; i++ )
  {
    if( ( sfxchan[i].sample ) && ( sfxchan[i].delta ) )
    {
      for( j=0; j<length/4; j++ )
      {
        if( (sfxchan[i].pos>>8) >= sfxchan[i].len )
        {
          if( sfxchan[i].loops != 0 )
          {
            sfxchan[i].pos = 0;
            if( sfxchan[i].loops > 0 )
              sfxchan[i].loops--;
          } else {
            sfxchan[i].sample = NULL;
            sfxchan[i].delta = 0;
            break;
          }
        }
           
        v = sfxchan[i].sample[sfxchan[i].pos>>8] * sfxchan[i].volume;
        chanmix[j] += v;

        sfxchan[i].pos += sfxchan[i].delta;
      }
    }
  }

  for( i=0, j=0; i<length/4; i++ )
  {
    chanmix[i] >>= 8;
    v = (out[j] + chanmix[i])>>1;
    if( v > 32767 ) v = 32767;
    if( v < -32768 ) v = -32768;
    out[j++] = v;
    v = (out[j] + chanmix[i])>>1;
    if( v > 32767 ) v = 32767;
    if( v < -32768 ) v = -32768;
    out[j++] = v;
  }
}

void audioframe( void )
{
  if( !tune ) return;

/*
  if( ( musicenabled ) || ( what_are_we_doing != WAWD_GAME ) )
  {
    if( tune->mastervolume < musicvol )
    {
      tune->mastervolume += 3;
      if( tune->mastervolume > musicvol )
        tune->mastervolume = musicvol;
    }
  } else {
    if( tune->mastervolume > 0 )
    {
      if( tune->mastervolume < 3 )
        tune->mastervolume = 0;
      else
        tune->mastervolume -= 3;
    }
  }
*/
}

void initsounds( void )
{
  SDL_AudioSpec wanted;
  int i;

  for( i=0; i<C_LAST; i++ )
  {
    sfxchan[i].sample = NULL;
    sfxchan[i].delta = 0;
  }

  for( i=0; i<SND_LAST; i++ )
  {
    smp[i].data = NULL;
    smp[i].len = 0;
  }

  // Set up SDL audio
  wanted.freq     = FREQ; 
  wanted.format   = AUDIO_S16SYS; 
  wanted.channels = 2; /* 1 = mono, 2 = stereo */
  wanted.samples  = SNDBUFFERLEN/4;

  wanted.callback = (void*)audio_callback;
  wanted.userdata = NULL;

  audioavailable = FALSE;
  if( SDL_OpenAudio( &wanted, NULL ) >= 0 )
  {
    audioavailable = SDL_TRUE;
  }

	tunePlaying = FALSE;
  SDL_PauseAudio( 0 );
}

void startmusic( void )
{
	if( tunePlaying ) return;

  lastplayed = -1;
	tunePlaying = TRUE;
}

void stopmusic( void )
{
	if(!tunePlaying) return;
	tunePlaying = FALSE;
}

void killtune( void )
{
	if( tunePlaying ) stopmusic();
	if( tune )
	{
    free( tune );
		tune = NULL;
	}
}

void playtune( char *fname )
{
  u8 *buf;
  int buflen;
  FILE *f;

  if( !audioavailable ) return;

  killtune();

  f = fopen( fname, "rb" );
  if( !f ) return;

  fseek( f, 0, SEEK_END);
  buflen = ftell(f);
  fseek( f, 0, SEEK_SET );

  buf = malloc( buflen );
  if( !buf )
  {
    fclose( f );
    return;
  }

  fread( buf, buflen, 1, f );
  fclose( f );

  tune = PtInit( buf, buflen, 44100 );
  if( !tune ) return;

  tune->mastervolume = musicvol;

	startmusic();
}

void setmusicvol( void )
{
  if( !tune ) return;
  tune->mastervolume = musicvol;
}

BOOL loadsounds( void )
{
  int i, j;
  FILE *f;
  char mkstr[256];
  Sint32 len;
  unsigned char *dat;
  unsigned char t;

  if( !audioavailable ) return TRUE;

  for( i=0; i<SND_LAST; i++ )
  {
#ifdef __android__
	if (use_alt_basepath)
		 sprintf( mkstr, BASEPATH_ALT"hats/fx%02x.raw", i+1 );
	else
#endif
		sprintf( mkstr, BASEPATH"hats/fx%02x.raw", i+1 );

    f = fopen( mkstr, "rb" );
    fseek( f, 0, SEEK_END );
    len = ftell( f );
    fseek( f, 0, SEEK_SET );
    if( len < 2 )
    {
      fclose( f );
      continue;
    }
    smp[i].data = malloc( len );
    if( !smp[i].data )
    {
      fclose( f );
      return FALSE;

    }
    fread( smp[i].data, len, 1, f );
    fclose( f );

    // Endian swap!
    dat = (unsigned char *)smp[i].data;
    for(j=0; j<len; j+=2)
    {
      t = dat[j];
      dat[j] = dat[j+1];
      dat[j+1] = t;
    }


    smp[i].len = len/2;
  }

  return TRUE;
}

void freesounds( void )
{
  int i;

  if( !audioavailable ) return;

  for( i=0; i<SND_LAST; i++ )
  {
    if( smp[i].data ) free(smp[i].data);
    smp[i].data = NULL;
    smp[i].len = 0;
  }
}

int audio_is_locked = 0;
void lockaudio( void )
{
  if( audio_is_locked ) return;
  SDL_LockAudio();
  audio_is_locked = 1;
}

void unlockaudio( void )
{
  if( !audio_is_locked ) return;
  SDL_UnlockAudio();
  audio_is_locked = 0;
}

int lastsound=-1;
void playsound( int chan, int sound, int volume )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 ) return;
  if( volume > 256 ) volume = 256;

  lockaudio();
  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/44100;
  sfxchan[chan].loops  = 0;
  sfxchan[chan].volume = volume;
}

void loopsound( int chan, int sound, int volume )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 )
    return;
  if( volume > 256 ) volume = 256;

  lockaudio();
  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/44100;
  sfxchan[chan].loops  = -1;
  sfxchan[chan].volume = volume;
}

void nloopsound( int chan, int sound, int volume, int loops )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 )
    return;
  if( volume > 256 ) volume = 256;

  lockaudio();
  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/44100;
  sfxchan[chan].loops  = loops;
  sfxchan[chan].volume = volume;
}

static int amsc = 0;
static int *amchp[2] = { NULL, NULL };
void ambientloop( int sound, int volume, int *chp )
{
  amsc = (amsc+1)%((C_AMBIENTLAST-C_AMBIENT1)+1);
  if( amchp[amsc] )
    *amchp[amsc] = -1;
  amchp[amsc] = chp;
  if( chp )
    *chp = C_AMBIENT1+amsc;
  loopsound( C_AMBIENT1+amsc, sound, volume );
}

void stopchannel( int chan )
{
  int i;

  if( !audioavailable ) return;

  for( i=C_AMBIENT1; i<=C_AMBIENTLAST; i++ )
    if( ( chan == i ) && ( amchp[i-C_AMBIENT1] ) ) { *amchp[i-C_AMBIENT1] = -1; amchp[i-C_AMBIENT1] = NULL; }

  sfxchan[chan].sample = NULL;
  sfxchan[chan].delta = 0;
}

void stopallchannels( void )
{
  int i;

  for( i=0; i<C_LAST; i++ )
    stopchannel( i );
}

void setvol( int chan, int volume )
{
  sfxchan[chan].volume = volume;
}

static int esc = 0;
void enemysound( int sound, int volume )
{
  esc = (esc+1)%((C_ENEMYLAST-C_ENEMY1)+1);
  playsound( C_ENEMY1+esc, sound, volume );
}

static int asc = 0;
void actionsound( int sound, int volume )
{
  asc = (asc+1)%((C_ACTIONLAST-C_ACTION1)+1);
  playsound( C_ACTION1+asc, sound, volume );
}

void lpactionsound( int sound, int volume )
{
  int j;
  for( j=0; j<=(C_ACTIONLAST-C_ACTION1); j++ )
  {
    asc = (asc+1)%((C_ACTIONLAST-C_ACTION1)+1);
    if( sfxchan[C_ACTION1+asc].sample == NULL )
      break;
  }

  if( j > (C_ACTIONLAST-C_ACTION1) ) return;
  playsound( C_ACTION1+asc, sound, volume );
}

static int isc = 0;
void incidentalsound( int sound, int volume )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  playsound( C_INCIDENTAL1+isc, sound, volume );
}

int incidentalloop( int sound, int volume )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  loopsound( C_INCIDENTAL1+isc, sound, volume );
  return C_INCIDENTAL1+isc;
}

int incidentalloops( int sound, int volume, int loops )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  nloopsound( C_INCIDENTAL1+isc, sound, volume, loops );
  return C_INCIDENTAL1+isc;
}
