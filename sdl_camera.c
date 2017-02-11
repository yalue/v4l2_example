// This is a simple program which will display webcam video in a window.
//
// Usage:
//    ./sdl_camera <device path e.g. "/dev/video0">
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "webcam_lib.h"

// The number of webcam resolutions to enumerate when checking resolutions.
#define MAX_RESOLUTION_COUNT (8)

static struct {
  WebcamInfo webcam;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t w;
  uint32_t h;
} g;

static char* ErrorString(void) {
  return strerror(errno);
}

static void CleanupSDL(void) {
  if (g.renderer) {
    SDL_DestroyRenderer(g.renderer);
    g.renderer = NULL;
  }
  if (g.window) {
    SDL_DestroyWindow(g.window);
    g.window = NULL;
  }
  if (g.texture) {
    SDL_DestroyTexture(g.texture);
    g.texture = NULL;
  }
}

// Enumerates the resolutions provided by the webcam and selects a suitable
// width and height. To be called during SetupWebcam.
static void SelectResolution(void) {
  WebcamResolution resolutions[MAX_RESOLUTION_COUNT];
  WebcamInfo *webcam = &(g.webcam);
  int selected_index = -1;
  int selected_size = 0x7fffffff;
  int current_size, i;
  memset(resolutions, 0, sizeof(resolutions));
  if (!GetSupportedResolutions(webcam, resolutions, MAX_RESOLUTION_COUNT)) {
    printf("Error getting supported resolutions: %s\n", ErrorString());
    CloseWebcam(webcam);
    exit(1);
  }
  for (i = 0; i < MAX_RESOLUTION_COUNT; i++) {
    current_size = resolutions[i].width * resolutions[i].height;
    if (current_size == 0) break;
    if (current_size > selected_size) continue;
    selected_size = current_size;
    selected_index = i;
  }
  if (selected_index < 0) {
    printf("Error: Found no valid resolutions.\n");
    CloseWebcam(webcam);
    exit(1);
  }
  g.w = resolutions[selected_index].width;
  g.h = resolutions[selected_index].height;
}

// Initializes the webcam struct. Exits on error.
static void SetupWebcam(char *path) {
  WebcamInfo *webcam = &(g.webcam);
  if (!OpenWebcam(path, webcam)) {
    printf("Error opening webcam: %s\n", ErrorString());
    exit(1);
  }
  if (!PrintCapabilityDetails(webcam)) {
    printf("Error printing camera capabilities: %s\n", ErrorString());
    exit(1);
  }
  if (!PrintVideoFormatDetails(webcam)) {
    printf("Error printing video format details: %s\n", ErrorString());
    goto error_exit;
  }
  SelectResolution();
  if (!SetResolution(webcam, g.w, g.h)) {
    printf("Error setting video resolution: %s\n", ErrorString());
    goto error_exit;
  }
  return;
error_exit:
  CloseWebcam(webcam);
  exit(1);
}

// Once the webcam has been opened, call this to set up the SDL info necessary
// for displaying the image.
static void SetupSDL(void) {
  WebcamInfo *webcam = &(g.webcam);
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("SDL error: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.window = SDL_CreateWindow("Webcam view", SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED, g.w, g.h, SDL_WINDOW_SHOWN |
    SDL_WINDOW_RESIZABLE);
  if (!g.window) {
    printf("SDL error creating window: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
  if (!g.renderer) {
    printf("Failed creating SDL renderer: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.texture = SDL_CreateTexture(g.renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING, g.w, g.h);
  if (!g.texture) {
    printf("Failed getting SDL texture: %s\n", SDL_GetError());
    goto error_exit;
  }
  return;
error_exit:
  CloseWebcam(webcam);
  CleanupSDL();
  exit(1);
}

// Copy images from the camera to the window, until an SDL quit event is
// detected.
static void MainLoop(void) {
  SDL_Event event;
  size_t frame_size = 0;
  void *frame_bytes = NULL;
  void *texture_pixels = NULL;
  int texture_pitch = 0;
  int quit = 0;
  WebcamInfo *webcam = &(g.webcam);
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
        break;
      }
    }
    // Start loading a new frame from the webcam.
    if (!BeginLoadingNextFrame(webcam)) {
      printf("Error getting webcam frame: %s\n", ErrorString());
      goto error_exit;
    }
    // Sleep for 10 ms while the frame gets loaded.
    usleep(10000);
    // Lock the texture in order to modify its pixel data directly.
    if (SDL_LockTexture(g.texture, NULL, &texture_pixels, &texture_pitch)
      < 0) {
      printf("Error locking SDL texture: %s\n", SDL_GetError());
      goto error_exit;
    }
    // By this point, the image data should have finished loading, so get a
    // pointer to its location.
    if (!GetFrameBuffer(webcam, &frame_bytes, &frame_size)) {
      printf("Error getting frame from webcam: %s\n", ErrorString());
      goto error_exit;
    }
    if (!ConvertYUYVToRGBA(frame_bytes, texture_pixels, g.w, g.h, g.w * 2,
      texture_pitch)) {
      printf("Failed converting YUYV to RGBA color.\n");
      goto error_exit;
    }
    // Finalize the texture changes, re-draw the texture, re-draw the window
    SDL_UnlockTexture(g.texture);
    if (SDL_RenderCopy(g.renderer, g.texture, NULL, NULL) < 0) {
      printf("Error rendering texture: %s\n", SDL_GetError());
      goto error_exit;
    }
    SDL_RenderPresent(g.renderer);
  }
  return;
error_exit:
  CloseWebcam(webcam);
  CleanupSDL();
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <device path e.g. \"/dev/video0\">\n", argv[0]);
    return 1;
  }
  memset(&g, 0, sizeof(g));
  SetupWebcam(argv[1]);
  SetupSDL();
  printf("Showing %dx%d video.\n", (int) g.w, (int) g.h);
  MainLoop();
  CloseWebcam(&(g.webcam));
  CleanupSDL();
  return 0;
}
