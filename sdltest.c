// Compile with:
//   gcc -I/usr/local/include -L/usr/local/lib -lSDL2 sdltest.c

#include <SDL2/SDL.h>

#include <math.h>
#ifndef M_PI
  #define M_PI 3.1415926535897932384
#endif

double offset = 0;

void generateWaveform(void *userdata, Uint8 *stream, int len) {
  float *dest = (float*)stream;
  int size = len/sizeof(float);
  int freq = *(int*)userdata;
  for (int i=0; i<size; i++) {
    dest[i] = sin(freq*(2*M_PI)*i/48000.0 + offset);
  }
  offset = fmod(freq*(2*M_PI)*size/48000.0 + offset, 2*M_PI);
  (*(int *)userdata)++;
}

int main(int argc, char* argv[]) {
  // Rendering vars
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  // Audio vars
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;
  int frequency = 1000;

  // Initialize with appropriate flags
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    return 1;

  // Audio settings
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 48000;
  want.format = AUDIO_F32;
  want.channels = 1;
  want.samples = 800;
  want.callback = generateWaveform;
  want.userdata = &frequency;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) 
    printf("HELP ME IT'S %s\n", SDL_GetError());
  SDL_PauseAudioDevice(dev, 0);

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
