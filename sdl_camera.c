// This is a simple program which will display webcam video in a window.
//
// Usage:
//    ./sdl_camera <device path e.g. "/dev/video0">
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "webcam_lib.h"

// The number of webcam resolutions to enumerate when checking resolutions.
#define MAX_RESOLUTION_COUNT (8)

// The number of seconds to wait between attempting to poll for frames.
#define SECONDS_PER_FRAME (1.0 / 10.0)

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

// Returns the current system time in seconds. Exits if an error occurs while
// getting the time.
static double CurrentSeconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    printf("Error getting time.\n");
    exit(1);
  }
  return ((double) ts.tv_sec) + (((double) ts.tv_nsec) / 1e9);
}

// Sleeps for the given floating-point number of seconds.
static void SleepSeconds(double to_sleep) {
  if (to_sleep <= 0) return;
  usleep(to_sleep * 1e6);
}

// Enumerates the resolutions provided by the webcam and selects the smallest
// available one.
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
  unsigned long long dropped_count = 0;
  unsigned long long total_count = 0;
  FrameBufferState frame_state;
  double overall_start, last_frame_start;
  WebcamInfo *webcam = &(g.webcam);
  // Load the first frame and sleep for the duration of a full cycle in order
  // to (hopefully) have a frame loaded on the first loop iteration.
  if (!BeginLoadingNextFrame(webcam)) {
    printf("Error loading initial frame: %s\n", ErrorString());
    goto error_exit;
  }
  SleepSeconds(SECONDS_PER_FRAME);
  last_frame_start = CurrentSeconds();
  overall_start = last_frame_start;
  while (!quit) {
    total_count++;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
        break;
      }
    }
    frame_state = GetFrameBuffer(webcam, &frame_bytes, &frame_size);
    if (frame_state == DEVICE_ERROR) {
      printf("Error getting frame from webcam: %s\n", ErrorString());
      goto error_exit;
    }
    // Drop this frame if the frame isn't ready--don't update the display.
    if (frame_state == FRAME_NOT_READY) {
      dropped_count++;
      // Sleep for the remaining time in this frame, then reset the sleep timer
      SleepSeconds(SECONDS_PER_FRAME - (CurrentSeconds() - last_frame_start));
      last_frame_start = CurrentSeconds();
      continue;
    }
    // The device didn't give an error and the frame was ready, so we can now
    // enqueue the next frame and update the displayed image.
    if (!BeginLoadingNextFrame(webcam)) {
      printf("Error getting webcam frame: %s\n", ErrorString());
      goto error_exit;
    }
    // To re-draw the window, "lock" the texture, update its pixel data,
    // "unlock" the texture, re-draw the texture, then re-draw the window.
    if (SDL_LockTexture(g.texture, NULL, &texture_pixels, &texture_pitch)
      < 0) {
      printf("Error locking SDL texture: %s\n", SDL_GetError());
      goto error_exit;
    }
    // The color conversion will write the RGBA pixel data directly into the
    // texture's buffer.
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
    // Finally, sleep for the remainder of the time in this frame.
    SleepSeconds(SECONDS_PER_FRAME - (CurrentSeconds() - last_frame_start));
    last_frame_start = CurrentSeconds();
  }
  printf("Attempted to display %llu frames in %f seconds (wanted %f FPS). "
    "Dropped %llu.\n", total_count, CurrentSeconds() - overall_start,
    1 / SECONDS_PER_FRAME, dropped_count);
  return;
error_exit:
  munlockall();
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
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    printf("Failed locking pages into memory: %s\n", ErrorString());
    CloseWebcam(&(g.webcam));
    CleanupSDL();
    return 1;
  }
  printf("Showing %dx%d video.\n", (int) g.w, (int) g.h);
  MainLoop();
  munlockall();
  CloseWebcam(&(g.webcam));
  CleanupSDL();
  return 0;
}
