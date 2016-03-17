/*=====================*
 |    Theremin Hero    |
 |  fseidel, jocelynh  |
 |     03/14/2016      |
 *=====================*/

/* Have you ever wanted to be a theremin-playing superhero? Well, you still
 * can't, but at least you can pretend to play a theremin.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_TTF.h>
#include <math.h>
#ifndef M_PI
  #define M_PI 3.1415926535897932384
#endif

#define TAU (2*M_PI)

/*==========<< GLOBALS >>===========*/

int quit = 0;  /* Did the user hit quit? */
// Settings
int colorblind = 0;
int mute = 0;

/* AUDIO wavedata/userdata struct */
typedef struct {
  double carrier_phase;       // Sine phase for callback to continue w.o clicks
  double modulator_phase;
  double modulator_amplitude; // Amount of modulation
  int carrier_pitch;          // Frequency of carrier that determines pitch
  int modulator_pitch;        // Frequency of modulator
} wavedata;

/* Functions */
void createWant(SDL_AudioSpec *wantpoint, wavedata *userdata);

/*=========<< END GLOBALS >>=========*/

/*=======<< generateWaveform (Callback Function) >>=======*
 * Fill audio buffer w/ glorious FM synth!                *
 * We take a sine wave (the carrier) and modulate it with *
 * another sine wave (the modulator).                     *
 *                                                        *
 * => sin(sin(t + p1) + t + p2) where p1, p2 are phases   *
 *                                                        *
 * This can create complex sounds with simple waveforms!  *
 * You hear the "outer" (carrier) sine wave, as expected, *
 * but you also hear the wave produced by the modulation  *
 * of the carrier.                                        *
 *========================================================*/
void generateWaveform(void *userdata, Uint8 *stream, int len) {
  float *dest = (float*)stream;       // Destination of values generated
  int size = len/sizeof(float);       // Buffer size

  wavedata *wave_data = (wavedata*)userdata;  // Get info from wavedata
  int c_pitch = wave_data->carrier_pitch;     // Wave that actually plays
  double c_phase = wave_data->carrier_phase;
  int m_pitch = wave_data->modulator_pitch;   // Wave that modulates carrier
  double m_phase = wave_data->modulator_phase;
  double m_amplitude = wave_data->modulator_amplitude;

  // Fill buffer
  for (int i=0; i<size; i++) {
    dest[i] =
      sin( m_amplitude * sin( m_pitch*TAU*i/48000 + m_phase )
           + c_pitch*TAU*i/48000 + c_phase);  // <- Modulation
  }

  // Update phase s.t. next frame of audio starts at same point in wave
  wave_data->carrier_phase =
    fmod(c_pitch*TAU*size/48000 + c_phase, TAU);
  wave_data->modulator_phase =
    fmod(m_pitch*TAU*size/48000 + m_phase, TAU);

  /* Change modulator amplitude to vary the amount of modulation.
   * A decay of 1 second means 0.4/60 = 0.066 repeating.
   * 0.4 is the max amplitude (completely arbitrary, it just sounds good).
   * 60 is frames per second.
   */
  if(m_amplitude > 0) wave_data->modulator_amplitude -= 0.0066666666;
  else wave_data->modulator_amplitude = 0.4; //reset if we hit 0
}

/*=============<< main >>==============*
 * Get that party started!             *
 * Initialize for rendering and audio. *
 *=====================================*/
int main(int argc, char* argv[]) {
  // Rendering vars
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  // Audio vars
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;
  wavedata my_wavedata;

  // Keycode for key presses
  SDL_Keycode key;

  /*******<Initial Settings>*******/

  // Initialize with appropriate flags
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0 ||
      TTF_Init() < 0)
    return 1;
  atexit(SDL_Quit); // Set exit function s.t. SDL resources deallocated on quit



  /* ======<< AUDIO SETTINGS >>======= */
  SDL_memset(&want, 0, sizeof(want));
  createWant(&want, &my_wavedata);    // Call function to initialize vals
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) 
    printf("Error opening audio device: %s\n", SDL_GetError());



  /* ======<< RENDERING SETTINGS >====== */

  // Create window and renderer
  window = SDL_CreateWindow("SDL_RenderClear",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);
  renderer = SDL_CreateRenderer(window, -1, 0);

  /* Text */

  // Opens font
  TTF_Font* font = TTF_OpenFont("/Library/Fonts/Arial.ttf", 12);
  if(font == NULL) {
    printf("Font not found\n");
    return 1;
  }
  SDL_Color normalFontColor = {50, 170, 255};   // Darker blue
  SDL_Color cbFontColor = {80, 100, 80};        // Weird green
  SDL_Color fontColor = normalFontColor;
  

  /*********< Okay, game time! >***********/
  while (!quit) {

    /* ==========<< Poll for events >>============ */
    while (SDL_PollEvent(&event)) {
      switch (event.type) {

        /* Key pressed */
        case SDL_KEYDOWN:
          key = event.key.keysym.sym;

          if (key == SDLK_ESCAPE) {
            quit = 1;
          }
          else if (key == SDLK_BACKSPACE) {
            colorblind = (colorblind+1)%2;
          }
          else if (key == SDLK_m) {
            mute = (mute+1)%2;
          }
          break;
        /* Exit */
        case SDL_QUIT:
          quit = 1;
          break;
        default:
          break;
      }
    }

    /* ========<< Text >>======== */

    // Set font color
    fontColor = normalFontColor;
    if (colorblind) {
      fontColor = cbFontColor;
    }
    
    // Create surface and convert it to texture
    SDL_Surface* surfaceMessage =
      TTF_RenderText_Solid(font, "Theremin Hero!", fontColor);
    if (colorblind) {
      surfaceMessage = TTF_RenderText_Solid(font, "Colorblind Mode ;D", fontColor);
    }
    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    // {xPos, yPos, width, height}
    SDL_Rect message_rect = {150,200,200,80};


    /* ========<< Background >>========= */

    // Choose background color
    SDL_SetRenderDrawColor(renderer, 170, 200, 215, 255);   // Light blue
    if (colorblind) {
      SDL_SetRenderDrawColor(renderer, 90, 60, 80, 255);    // Dark magenta
    }

    // Set background color
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 200, 255, 0, 255); // Green
    SDL_RenderDrawLine(renderer, 5, 5, 340, 340);

    // Render message texture
    SDL_RenderCopy(renderer, message, NULL, &message_rect);

    // Move to foreground
    SDL_RenderPresent(renderer);

    /* =========<< Audio >>========== */
    SDL_PauseAudioDevice(dev, mute);
  }

  // CLEAN YO' ROOM (Cleanup)
  TTF_CloseFont(font);
  SDL_CloseAudioDevice(dev);
  SDL_Quit();

  return 0;
}


/********<< Setup Functions >>*********/

/*==========< createWant >===========*
 * Initialize the "want" Audiospec,  *
 * and set its values appropriately  *
 *===================================*/
void createWant(SDL_AudioSpec *wantpoint, wavedata *userdata) {

  wantpoint->freq = 48000;        // Sample rate of RasPi's sound system
  wantpoint->format = AUDIO_F32;  // 32-bit floating point samples, little-endian
  wantpoint->channels = 1;
  wantpoint->samples = 800;   // (48000 samples/sec)/(60 frames/sec) = 800 samp/frame
  wantpoint->callback = generateWaveform;

  // Set info in wavedata struct
  userdata->carrier_pitch = 1000;
  userdata->modulator_pitch = 500;
  userdata->modulator_phase = 0.0;
  userdata->carrier_phase = 0.0;
  userdata->modulator_amplitude = 0.4;

  wantpoint->userdata = userdata;
}

