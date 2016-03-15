// Compile with:
//   gcc -I/usr/local/include -L/usr/local/lib -lSDL2 sdltest.c

#include <SDL2/SDL.h>

#include <math.h>
#ifndef M_PI
  #define M_PI 3.1415926535897932384
#endif

double offset = 0;  /* Offset for callback function to continue where it left
                       off and prevent clicking */

/***<< Sine wave generator -- Callback function >>***/
void generateWaveform(void *userdata, Uint8 *stream, int len) {
  float *dest = (float*)stream;       // Destination of values generated
  int size = len/sizeof(float);       // Buffer size
  int *freq_pitch = (int*)userdata;   // The pitch given

  // Fill buffer
  for (int i=0; i<size; i++) {
    // Stick sine value in buffer
    dest[i] = sin(*freq_pitch*(2*M_PI)*i/48000 + offset);
  }
  // Update offset
  offset = fmod(*freq_pitch*(2*M_PI)*size/48000 + offset, 2*M_PI);

  // Swept sine wave; increment freq. by 1 each frame
  *freq_pitch += 1;
}

/********<< Main >>********/
int main(int argc, char* argv[]) {
  // Rendering vars
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  // Audio vars
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;
  int freq_pitch = 1000;     // Arbitrary -- pitch

  /***************/

  // Initialize with appropriate flags
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    return 1;

  // Set audio settings
  SDL_memset(&want, 0, sizeof(want));

  want.freq = 48000;        // Sample rate of RasPi's sound system
  want.format = AUDIO_F32;  // 32-bit floating point samples, little-endian
  want.channels = 1;
  want.samples = 800;   // (48000 samples/sec)/(60 frames/sec) = 800 samp/frame
  want.callback = generateWaveform;
  want.userdata = &freq_pitch;

  // Alright audio is a go
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) 
    printf("HELP ME IT'S %s\n", SDL_GetError());
  SDL_PauseAudioDevice(dev, 0);

  /***************/

  // Create window in which we will draw
  window = SDL_CreateWindow("SDL_RenderClear",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, 0);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red

  // Set screen to red
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderDrawLine(renderer, 5, 5, 300, 300);

  // Move to foreground
  SDL_RenderPresent(renderer);

  while(1) {
    if(SDL_PollEvent(&event)) {
      if(event.type == SDL_KEYDOWN)
        break;
    }
  }

  // Up for 5 seconds
  //SDL_Delay(5000);

  // CLEAN YO' ROOM
  SDL_CloseAudioDevice(dev);
  SDL_Quit();

  return 0;
}
