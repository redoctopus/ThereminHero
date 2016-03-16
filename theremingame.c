/*=====================*
 |    Theremin Hero    |
 |  fseidel, jocelynh  |
 |     03/14/2016      |
 *=====================*/

/* Have you ever wanted to be a theremin-playing superhero? Well, you still
 * can't, but at least you can pretend to play a theremin.
 */

#include <SDL2/SDL.h>
#include <math.h>
#ifndef M_PI
  #define M_PI 3.1415926535897932384
#endif

/*==========<< GLOBALS >>===========*/

int quit = 0;  /* Did the user hit quit? */

/* AUDIO wavedata/userdata struct containing:
 *  phase:      Sine phase for callback function to continue where it left off
 *              such that there isn't any clicking
 *  freq_pitch: Frequency of the pitch given (arbitrary)
 */
typedef struct {
  double phase;
  int freq_pitch;
} wavedata;

/* Functions */
void createWant(SDL_AudioSpec *wantpoint, wavedata *userdata);

/*=========<< END GLOBALS >>=========*/

/*==<< Sine wave generator -- Callback function >>==*/
void generateWaveform(void *userdata, Uint8 *stream, int len) {
  float *dest = (float*)stream;       // Destination of values generated
  int size = len/sizeof(float);       // Buffer size

  wavedata *wave_data = (wavedata*)userdata;  // Get info from wavedata
  int freq_pitch = wave_data->freq_pitch;
  double phase = wave_data->phase;

  // Fill audio buffer w/sine value
  for (int i=0; i<size; i++) {
    dest[i] = sin(freq_pitch*(2*M_PI)*i/48000 + phase);
  }

  // Update phase s.t. next frame of audio starts at same point in wave
  wave_data->phase = fmod(freq_pitch*(2*M_PI)*size/48000 + phase, 2*M_PI);
  // Swept sine wave; increment freq. by 1 each frame
  wave_data->freq_pitch += 1;
}

/*=============<< Main >>==============*
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

  /*******<Initial Settings>*******/

  // Initialize with appropriate flags
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    return 1;
  atexit(SDL_Quit); // Set exit function s.t. SDL resources deallocated on quit

  // AUDIO SETTINGS
  SDL_memset(&want, 0, sizeof(want));
  createWant(&want, &my_wavedata);    // Call function to initialize vals
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) 
    printf("Error opening audio device: %s\n", SDL_GetError());
  SDL_PauseAudioDevice(dev, 0);

  /*******<Rendering/Drawing Setup>*******/

  // Create window in which we will draw
  window = SDL_CreateWindow("SDL_RenderClear",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, 0);

  SDL_SetRenderDrawColor(renderer, 170, 200, 215, 255);

  // Set background color
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 200, 255, 0, 255); // Green
  SDL_RenderDrawLine(renderer, 5, 5, 300, 300);

  // Move to foreground
  SDL_RenderPresent(renderer);

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_KEYDOWN:
          quit = 1;
          break;
        default:
          break;
      }
    }
  }

  // CLEAN YO' ROOM (Cleanup)
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
  userdata->freq_pitch = 1000;
  userdata->phase = 0.0;
  wantpoint->userdata = userdata;
}

