#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int N = 801;
int **grid;

// you can also use this function to debug print your sandpile
static void render(char *filename) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  int stride = N * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (grid[i][j]) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }

      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }
  if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
    printf("Successfully saved sandpile identity to: %s\n", filename);
  } else {
    die("Error: stb_image_write failed to save png file\n");
  }
  free(pixels);
}

static void stabilize() {
  int *first_col = (int*)malloc(N * sizeof(int));
  int *last_col = (int*)malloc(N * sizeof(int));
  for (int i = 0; i < N; ++i) {
    bool found = false;
    first_col[i] = 1;
    last_col[i] = 0;
    for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
      if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
          (N / 2) * (N / 2)) {
        if (found == false) {
          first_col[i] = j;
          found = true;
        }
        last_col[i] = j;
      }
    }
  }
  int M = N / 2;
  int changed = 0;
  int forward = 1;
  do {
    changed = 0;
    for (int k = 0; k <= M; ++k) {
      int i = forward ? k : M - k;
      for (int l = first_col[i]; l <= last_col[i]; ++l) {
        int j = forward ? l : first_col[i] + last_col[i] - l;
        int topple_cnt = grid[i][j] >> 2;  // total number of times its gonna need to be toppled
        if (topple_cnt == 0) continue;
        changed = 1; // topple it
        grid[i][j] %= 4;
        if (i > 0)
          grid[i - 1][j] += topple_cnt;
        if (j > 0)
          grid[i][j - 1] += topple_cnt;
        if (i < M - 1)
          grid[i + 1][j] += topple_cnt;
        else if (i == M - 1) 
          grid[i + 1][j] += 2 * topple_cnt;
        if (j < N - 1)
          grid[i][j + 1] += topple_cnt;
      }
    }
    forward = !forward;
  } while (changed);
  free(first_col);
  free(last_col);
}


void find_identity() {
  int M = N / 2;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6;
    }
  }
  stabilize();
  for (int i = 0; i <= M; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6 - grid[i][j];
    }
  }
  stabilize();
  for (int i = 0; i < M; ++i) {
    memcpy(grid[N-1-i], grid[i], N*sizeof(int));
  }
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 2) {
    fprintf(stderr, "Usage: %s N\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *end;
  uintmax_t parsed = strtoumax(argv[1], &end, 10);
  if (errno || argv[1][0] < '0' || argv[1][0] > '9' || *end || !parsed ||
      parsed > INT32_MAX)
    die("grid size must be a positive int");
  N = (int)parsed;

  grid = (int **)malloc(N * sizeof(int *));
  int *block = (int*)malloc((size_t)N * N * sizeof(int));
  for (int i = 0; i < N; ++i) {
    grid[i] = block + (size_t)i * N;
  }

  find_identity();

  render("identity.png");
  free(grid[0]);
  free(grid);
  return EXIT_SUCCESS;
}
