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
double size = 4.4;
double y_offset = 0.1080729;
double x_offset = -0.6386042;
int max = 60;

const char* COLORS[] = {
  "\e[48;5;18m \e[0m",   // midnightblue,
  "\e[48;5;19m \e[0m",   // darkblue,
  "\e[48;5;20m \e[0m",   // mediumblue,
  "\e[48;5;28m \e[0m",   // darkgreen,
  "\e[48;5;34m \e[0m",   // forestgreen,
  "\e[48;5;37m \e[0m",   // darkcyan,
  "\e[48;5;39m \e[0m",   // dodgerblue,
  "\e[48;5;43m \e[0m",   // lightseagreen,
  "\e[48;5;44m \e[0m",   // darkturquoise,
  "\e[48;5;45m \e[0m",   // deepskyblue,
  "\e[48;5;46m \e[0m",   // lime,
  "\e[48;5;48m \e[0m",   // springgreen,
  "\e[48;5;49m \e[0m",   // mediumspringgreen,
  "\e[48;5;51m \e[0m",   // aqua,
  "\e[48;5;55m \e[0m",   // indigo,
  "\e[48;5;59m \e[0m",   // darkslategray,
  "\e[48;5;61m \e[0m",   // darkslateblue,
  "\e[48;5;65m \e[0m",   // darkolivegreen,
  "\e[48;5;69m \e[0m",   // royalblue,
  "\e[48;5;72m \e[0m",   // seagreen,
  "\e[48;5;74m \e[0m",   // steelblue,
  "\e[48;5;77m \e[0m",   // limegreen,
  "\e[48;5;78m \e[0m",   // mediumseagreen,
  "\e[48;5;80m \e[0m",   // mediumturquoise,
  "\e[48;5;86m \e[0m",   // turquoise,
  "\e[48;5;88m \e[0m",   // webmaroon,
  "\e[48;5;90m \e[0m",   // webpurple,
  "\e[48;5;97m \e[0m",   // rebeccapurple,
  "\e[48;5;102m \e[0m",  // dimgray,
  "\e[48;5;104m \e[0m",  // slateblue,
  "\e[48;5;105m \e[0m",  // mediumslateblue,
  "\e[48;5;106m \e[0m",  // olivedrab,
  "\e[48;5;109m \e[0m",  // cadetblue,
  "\e[48;5;111m \e[0m",  // cornflower,
  "\e[48;5;115m \e[0m",  // mediumaquamarine,
  "\e[48;5;118m \e[0m",  // lawngreen,
  "\e[48;5;122m \e[0m",  // aquamarine,
  "\e[48;5;124m \e[0m",  // brown,
  "\e[48;5;127m \e[0m",  // darkmagenta,
  "\e[48;5;128m \e[0m",  // darkviolet,
  "\e[48;5;129m \e[0m",  // purple,
  "\e[48;5;130m \e[0m",  // saddlebrown,
  "\e[48;5;131m \e[0m",  // sienna,
  "\e[48;5;134m \e[0m",  // darkorchid,
  "\e[48;5;135m \e[0m",  // blueviolet,
  "\e[48;5;141m \e[0m",  // mediumpurple,
  "\e[48;5;142m \e[0m",  // olive,
  "\e[48;5;145m \e[0m",  // darkgray,
  "\e[48;5;149m \e[0m",  // yellowgreen,
  "\e[48;5;151m \e[0m",  // darkseagreen,
  "\e[48;5;153m \e[0m",  // skyblue,
  "\e[48;5;157m \e[0m",  // lightgreen,
  "\e[48;5;157m \e[0m",  // palegreen,
  "\e[48;5;160m \e[0m",  // firebrick,
  "\e[48;5;163m \e[0m",  // mediumvioletred,
  "\e[48;5;168m \e[0m",  // maroon,
  "\e[48;5;170m \e[0m",  // mediumorchid,
  "\e[48;5;172m \e[0m",  // chocolate,
  "\e[48;5;174m \e[0m",  // indianred,
  "\e[48;5;178m \e[0m",  // darkgoldenrod,
  "\e[48;5;179m \e[0m",  // peru,
  "\e[48;5;181m \e[0m",  // rosybrown,
  "\e[48;5;186m \e[0m",  // darkkhaki,
  "\e[48;5;187m \e[0m",  // tan,
  "\e[48;5;188m \e[0m",  // gray,
  "\e[48;5;189m \e[0m",  // lightsteelblue,
  "\e[48;5;191m \e[0m",  // greenyellow,
  "\e[48;5;195m \e[0m",  // lightblue,
  "\e[48;5;197m \e[0m",  // crimson,
  "\e[48;5;199m \e[0m",  // deeppink,
  "\e[48;5;201m \e[0m",  // fuchsia,
  "\e[48;5;202m \e[0m",  // orangered,
  "\e[48;5;209m \e[0m",  // coral,
  "\e[48;5;211m \e[0m",  // palevioletred,
  "\e[48;5;212m \e[0m",  // hotpink,
  "\e[48;5;213m \e[0m",  // orchid,
  "\e[48;5;214m \e[0m",  // darkorange,
  "\e[48;5;216m \e[0m",  // darksalmon,
  "\e[48;5;217m \e[0m",  // lightcoral,
  "\e[48;5;219m \e[0m",  // violet,
  "\e[48;5;223m \e[0m",  // burlywood,
  "\e[48;5;224m \e[0m",  // lightpink,
  "\e[48;5;225m \e[0m",  // thistle,
  "\e[48;5;226m \e[0m",  // gold,
  "\e[48;5;229m \e[0m",  // khaki,
  "\e[48;5;230m \e[0m",  // bisque,
  "\e[48;5;231m \e[0m",  // aliceblue,
  "\e[40m \e[0m",        // black,
  "\e[40m \e[0m",        // black,
  "\e[40m \e[0m"         // black
};

const int NR_OF_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);
const int NR_OF_THREADS = 7;

// The central algorithm. Calculate zₙ+₁ = zₙ² + c until zₙ starts to grow
// exponentially, which is when |zₙ| >= 2. Return the number of iterations
// until we reach that point, or max if we break early.
long iterations(double x, double y) {
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
    double y = coordinate(size, 0.6, row, window_height, y_offset);
    for (int col = index; col < window_width; col += NR_OF_THREADS) {
      double x = coordinate(size, 1.0, col, window_width, -x_offset);
      result[row][col] = (iterations(x, y) - 1) * NR_OF_COLORS / max;
    }
  }
  return NULL;
}

// Do the calulations for the entire picture in threads. Then print the result.
void draw(double x_offset, double y_offset) {
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
    draw(x_offset, y_offset);
    char line[1024];

    printf("X:%e Y:%e S:%e M:%d (U)p,(D)own,(L)eft,(R)ight,(I)n,(O)ut,(P)lus,(M)inus > ",
           x_offset, y_offset, size, max);
    if (fgets(line, sizeof(line), stdin)) {
      for (const char* ptr = &line[0]; *ptr; ++ptr) {
        const char c = tolower(*ptr);
        switch (c) {
        case 'u':
        case 'd':
          y_offset += (c == 'u') ? size/3 : -size/3;
          break;
        case 'l':
        case 'r':
          x_offset += (c == 'l') ? -size/3 : size/3;
          break;
        case 'i':
        case 'o':
          size *= (c == 'i') ? 0.5 : 2;
          break;
        case 'p':
        case 'm':
          max = (c == 'p') ? max * 6 / 5 : max * 5 / 6;
          break;
        }
      }
    }
  }
}
