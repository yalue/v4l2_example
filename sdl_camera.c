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
  if (!SetResolution(webcam, 1344, 376)) {
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
  uint32_t w, h;
  GetResolution(webcam, &w, &h);
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("SDL error: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.window = SDL_CreateWindow("Webcam view", SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
  if (!g.window) {
    printf("SDL error creating window: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
  if (!g.renderer) {
    printf("Failed creating SDL renderer: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.texture = SDL_CreateTexture(g.renderer, SDL_PIXELFORMAT_YUY2,
    SDL_TEXTUREACCESS_STREAMING, w, h);
  if (!g.texture) {
    printf("Failed getting SDL texture: %s\n", SDL_GetError());
    goto error_exit;
  }
  g.w = w;
  g.h = h;
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
    if (!LoadFrame(webcam)) {
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
    if (texture_pitch != (g.w * 2)) {
      printf("Bad texture pitch. Wanted %d, got %d\n", (int) (g.w * 2),
        texture_pitch);
      goto error_exit;
    }
    // By this point, the image data should have finished loading, so get a
    // pointer to its location.
    if (!GetFrameBuffer(webcam, &frame_bytes, &frame_size)) {
      printf("Error getting frame from webcam: %s\n", ErrorString());
      goto error_exit;
    }
    // Directly copy the image data from the camera to the texture buffer--
    // they're configured to use the same format (YUYV).
    memcpy(texture_pixels, frame_bytes, frame_size);
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
  MainLoop();
  CloseWebcam(&(g.webcam));
  CleanupSDL();
  return 0;
}
