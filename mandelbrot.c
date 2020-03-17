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
  "\e[48;5;18m ",   // midnightblue,
  "\e[48;5;19m ",   // darkblue,
  "\e[48;5;20m ",   // mediumblue,
  "\e[48;5;28m ",   // darkgreen,
  "\e[48;5;34m ",   // forestgreen,
  "\e[48;5;37m ",   // darkcyan,
  "\e[48;5;39m ",   // dodgerblue,
  "\e[48;5;43m ",   // lightseagreen,
  "\e[48;5;44m ",   // darkturquoise,
  "\e[48;5;45m ",   // deepskyblue,
  "\e[48;5;46m ",   // lime,
  "\e[48;5;48m ",   // springgreen,
  "\e[48;5;49m ",   // mediumspringgreen,
  "\e[48;5;51m ",   // aqua,
  "\e[48;5;55m ",   // indigo,
  "\e[48;5;59m ",   // darkslategray,
  "\e[48;5;61m ",   // darkslateblue,
  "\e[48;5;65m ",   // darkolivegreen,
  "\e[48;5;69m ",   // royalblue,
  "\e[48;5;72m ",   // seagreen,
  "\e[48;5;74m ",   // steelblue,
  "\e[48;5;77m ",   // limegreen,
  "\e[48;5;78m ",   // mediumseagreen,
  "\e[48;5;80m ",   // mediumturquoise,
  "\e[48;5;86m ",   // turquoise,
  "\e[48;5;88m ",   // webmaroon,
  "\e[48;5;90m ",   // webpurple,
  "\e[48;5;97m ",   // rebeccapurple,
  "\e[48;5;102m ",  // dimgray,
  "\e[48;5;104m ",  // slateblue,
  "\e[48;5;105m ",  // mediumslateblue,
  "\e[48;5;106m ",  // olivedrab,
  "\e[48;5;109m ",  // cadetblue,
  "\e[48;5;111m ",  // cornflower,
  "\e[48;5;115m ",  // mediumaquamarine,
  "\e[48;5;118m ",  // lawngreen,
  "\e[48;5;122m ",  // aquamarine,
  "\e[48;5;124m ",  // brown,
  "\e[48;5;127m ",  // darkmagenta,
  "\e[48;5;128m ",  // darkviolet,
  "\e[48;5;129m ",  // purple,
  "\e[48;5;130m ",  // saddlebrown,
  "\e[48;5;131m ",  // sienna,
  "\e[48;5;134m ",  // darkorchid,
  "\e[48;5;135m ",  // blueviolet,
  "\e[48;5;141m ",  // mediumpurple,
  "\e[48;5;142m ",  // olive,
  "\e[48;5;145m ",  // darkgray,
  "\e[48;5;149m ",  // yellowgreen,
  "\e[48;5;151m ",  // darkseagreen,
  "\e[48;5;153m ",  // skyblue,
  "\e[48;5;157m ",  // lightgreen,
  "\e[48;5;157m ",  // palegreen,
  "\e[48;5;160m ",  // firebrick,
  "\e[48;5;163m ",  // mediumvioletred,
  "\e[48;5;168m ",  // maroon,
  "\e[48;5;170m ",  // mediumorchid,
  "\e[48;5;172m ",  // chocolate,
  "\e[48;5;174m ",  // indianred,
  "\e[48;5;178m ",  // darkgoldenrod,
  "\e[48;5;179m ",  // peru,
  "\e[48;5;181m ",  // rosybrown,
  "\e[48;5;186m ",  // darkkhaki,
  "\e[48;5;187m ",  // tan,
  "\e[48;5;188m ",  // gray,
  "\e[48;5;189m ",  // lightsteelblue,
  "\e[48;5;191m ",  // greenyellow,
  "\e[48;5;195m ",  // lightblue,
  "\e[48;5;197m ",  // crimson,
  "\e[48;5;199m ",  // deeppink,
  "\e[48;5;201m ",  // fuchsia,
  "\e[48;5;202m ",  // orangered,
  "\e[48;5;209m ",  // coral,
  "\e[48;5;211m ",  // palevioletred,
  "\e[48;5;212m ",  // hotpink,
  "\e[48;5;213m ",  // orchid,
  "\e[48;5;214m ",  // darkorange,
  "\e[48;5;216m ",  // darksalmon,
  "\e[48;5;217m ",  // lightcoral,
  "\e[48;5;219m ",  // violet,
  "\e[48;5;223m ",  // burlywood,
  "\e[48;5;224m ",  // lightpink,
  "\e[48;5;225m ",  // thistle,
  "\e[48;5;226m ",  // gold,
  "\e[48;5;229m ",  // khaki,
  "\e[48;5;230m ",  // bisque,
  "\e[48;5;231m ",  // aliceblue,
  "\e[40m ",        // black,
  "\e[40m ",        // black,
  "\e[40m "         // black
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
double coordinate(double scale, int pos, int full, double offset) {
  return size * scale * (pos - full / 2.0) / full - offset;
}

// Called by each thread to fill in the thread's allotted slots in the result
// matrix.
void* executor(void* ptr) {
  long index = (long) ptr;
  for (int row = 0; row < window_height; ++row) {
    double y = coordinate(0.6, row, window_height, y_offset);
    for (int col = index; col < window_width; col += NR_OF_THREADS) {
      double x = coordinate(1.0, col, window_width, -x_offset);
      result[row][col] = (iterations(x, y) - 1) * NR_OF_COLORS / max;
    }
  }
  return NULL;
}

// Do the calulations for the entire picture in threads.
void calculate() {
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
}

// Print the result.
void draw() {
  printf("\033[0;0H"); // Set cursor at top left.
  for (int row = 0; row < window_height; ++row) {
    const char* previous = 0;
    for (int col = 0; col < window_width; ++col) {
      const char* current = COLORS[result[row][col]];
      printf(current == previous ? " " : current);
      previous = current;
    }
    printf("\e[0m\n");
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
    calculate();
    draw();
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
