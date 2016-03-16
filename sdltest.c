// Compile with:
//   gcc -I/usr/local/include -L/usr/local/lib -lSDL2 sdltest.c

#include <SDL2/SDL.h>

#include <math.h>
#ifndef M_PI
  #define M_PI 3.1415926535897932384
#endif

double phase = 0;  /* sine phase for callback function to continue where it left
                       off and prevent clicking */

/* Wavedata/userdata struct containing:
 *  phase:      Sine phase for callback function to continue where it left off
 *              such that there isn't any clicking
 *  freq_pitch: Frequency of the pitch given (arbitrary)
 */
typedef struct {
  double phase;
  int freq_pitch;
} wavedata;

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

/*=======<< Main >>=======*/
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

  // Set exit function so that all SDL resources are deallocated on quit
  atexit(SDL_Quit);

  // Set audio settings
  SDL_memset(&want, 0, sizeof(want));

  /*******<Set Audio Settings>*******/
  want.freq = 48000;        // Sample rate of RasPi's sound system
  want.format = AUDIO_F32;  // 32-bit floating point samples, little-endian
  want.channels = 1;
  want.samples = 800;   // (48000 samples/sec)/(60 frames/sec) = 800 samp/frame
  want.callback = generateWaveform;

  wavedata *userdata = &my_wavedata;  // Set info in wavedata struct
  userdata->freq_pitch = 1000;
  userdata->phase = 0.0;
  want.userdata = userdata;

  // Alright audio is a go
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) 
    printf("Error opening audio device: %s\n", SDL_GetError());
  SDL_PauseAudioDevice(dev, 0);

  /*******<Rendering/Drawing>*******/

  // Create window in which we will draw
  window = SDL_CreateWindow("SDL_RenderClear",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, 0);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red

  // Set screen to red
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
  SDL_RenderDrawLine(renderer, 5, 5, 300, 300);

  // Move to foreground
  SDL_RenderPresent(renderer);

  while(1) {
    if(SDL_PollEvent(&event)) {
      if(event.type == SDL_KEYDOWN)
        break;
    }
  }

  // CLEAN YO' ROOM
  SDL_CloseAudioDevice(dev);
  SDL_Quit();

  return 0;
}
