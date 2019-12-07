#if 0
gcc -pthread -O2 -o mandelbrot mandelbrot.c && ./mandelbrot "$@"
exit
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>

int window_height = 0;
int window_width = 0;
int** result;
double size = 1.1;
double y_offset = 0.3;
double x_offset = 1.189;
int max = 200;

const char* COLORS[] = {
  "\e[44m \e[0m",       /* blue */
  "\e[43m \e[0m",       /* yellow */
  "\e[42m \e[0m",       /* green */
  "\e[41m \e[0m",       /* red */
  "\e[48;5;230m \e[0m", /* moccasin */
  "\e[46m \e[0m",       /* cyan */
  "\e[48;5;77m \e[0m",  /* limegreen */
  "\e[48;5;214m \e[0m", /* orange */
  "\e[43m \e[0m",       /* yellow */
  "\e[40m \e[0m"        /* black */
};

const int NR_OF_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);
const int NR_OF_THREADS = 7;

// The central algorithm. Calculate zₙ+₁ = zₙ² + c until zₙ starts to grow
// exponentially, which is when |zₙ| >= 2. Return the number of iterations
// until we reach that point, or max if we break early.
long iterations(double x, double y, int max) {
  int count = 0;
  // The real and imaginary parts of z: zr and zi. Calulating the squares zr2
  // and zi2 one time per loop is both an optimization and a convenience.
  double zr = 0, zi = 0, zr2 = 0, zi2 = 0;
  for (; zr2 + zi2 < 4.0 && count < max; ++count) {
    // (a+b)² = a² + 2ab + b², so... (zr+zi)² = zr² + 2 * zr * zi + zi²
    // The new zr and zi will be the real and imaginary parts of that result.
    zi = 2 * zr * zi + y;
    zr = zr2 - zi2 + x;
    zr2 = zr * zr;
    zi2 = zi * zi;
  }
  return count;
}

// Convert a position (pos) on the screen (full), either horizontally or
// verically, into a floating point number, which is the real or imaginary
// number the position represents. The scale factor is to compensate for the
// non-square-ness of the pixels.
double coordinate(double size, double scale, int pos, int full, double offset) {
  return size * scale * (pos - full / 2.0) / full - offset;
}

// Called by each thread to fill in the thread's allotted slots in the result
// matrix.
void* executor(void* ptr) {
  long index = (long) ptr;
  for (int row = 0; row < window_height; ++row) {
    double y = coordinate(size, 2.4, row, window_height, y_offset);
    for (int col = index; col < window_width; col += NR_OF_THREADS) {
      double x = coordinate(size, 3.0, col, window_width, -x_offset);
      result[row][col] = (iterations(x, y, max) - 1) * NR_OF_COLORS / max;
    }
  }
  return NULL;
}

// Do the calulations for the entire picture in threads. Then print the result.
void draw(double size, double x_offset, double y_offset, int max) {
  pthread_t threads[NR_OF_THREADS];
  for (long t = 0; t < NR_OF_THREADS; ++t) {
    if (pthread_create(&threads[t], NULL, executor, (void*) t)) {
      fprintf(stderr, "Error creating thread\n");
      return;
    }
  }
  for (long t = 0; t < NR_OF_THREADS; ++t) {
    if (pthread_join(threads[t], NULL)) {
      fprintf(stderr, "Error joining thread\n");
      return;
    }
  }
  for (int row = 0; row < window_height; ++row) {
    for (int col = 0; col < window_width; ++col)
      printf("%s", COLORS[result[row][col]]);
    printf("\n");
  }
}

// Find out the current number of rows and columns in the terminal, set global
// variables, and reallocate the result matrix.
void set_window_size() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  for (int row = 0; row < window_height; ++row)
    free(result[row]);
  if (window_height > 0)
    free(result);

  window_height = w.ws_row - 1;
  window_width = w.ws_col;

  result = malloc(window_height * sizeof(int*));
  for (int row = 0; row < window_height; ++row)
    result[row] = malloc(window_width * sizeof(int));
}

int main(int argc, char** argv) {
  for (int ix = 1; ix < argc; ++ix) {
    const char* arg = argv[ix];
    switch (arg[0]) {
    case 'X': sscanf(arg, "X:%le", &x_offset); break;
    case 'Y': sscanf(arg, "Y:%le", &y_offset); break;
    case 'S': sscanf(arg, "S:%le", &size);     break;
    case 'M': sscanf(arg, "M:%d", &max);       break;
    }
  }

  while (1) {
    set_window_size();
    draw(size, x_offset, y_offset, max);
    char line[1024];

    printf("X:%e Y:%e S:%e M:%d (U)p,(D)own,(L)eft,(R)ight,(I)n,(O)ut,(P)lus,(M)inus > ",
           x_offset, y_offset, size, max);
    if (fgets(line, sizeof(line), stdin)) {
      for (const char* ptr = &line[0]; *ptr; ++ptr) {
        const char c = tolower(*ptr);
        switch (c) {
        case 'u':
        case 'd':
          y_offset += (c == 'u') ? size : -size;
          break;
        case 'l':
        case 'r':
          x_offset += (c == 'l') ? -size : size;
          break;
        case 'i':
        case 'o':
          size *= (c == 'i') ? 0.5 : 2;
          break;
        case 'p':
        case 'm':
          max = (c == 'p') ? max * 4 / 3 : max * 3 / 4;
          break;
        }
      }
    }
  }
}
