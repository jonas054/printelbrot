#if 0
gcc -pthread -O2 -o mandelbrot mandelbrot.c && ./mandelbrot "$@"
exit
#endif

// Good spots:
// X:-2.349932e-01 Y:8.281560e-01 S:2.622605e-07 M:4644

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const int COLORS[] = {
  18,   // midnightblue,
  19,   // darkblue,
  20,   // mediumblue,
  28,   // darkgreen,
  34,   // forestgreen,
  37,   // darkcyan,
  39,   // dodgerblue,
  43,   // lightseagreen,
  44,   // darkturquoise,
  45,   // deepskyblue,
  46,   // lime,
  48,   // springgreen,
  49,   // mediumspringgreen,
  51,   // aqua,
  55,   // indigo,
  59,   // darkslategray,
  61,   // darkslateblue,
  65,   // darkolivegreen,
  69,   // royalblue,
  72,   // seagreen,
  74,   // steelblue,
  77,   // limegreen,
  78,   // mediumseagreen,
  80,   // mediumturquoise,
  86,   // turquoise,
  88,   // webmaroon,
  90,   // webpurple,
  97,   // rebeccapurple,
  102,  // dimgray,
  104,  // slateblue,
  105,  // mediumslateblue,
  106,  // olivedrab,
  109,  // cadetblue,
  111,  // cornflower,
  115,  // mediumaquamarine,
  118,  // lawngreen,
  122,  // aquamarine,
  124,  // brown,
  127,  // darkmagenta,
  128,  // darkviolet,
  129,  // purple,
  130,  // saddlebrown,
  131,  // sienna,
  134,  // darkorchid,
  135,  // blueviolet,
  141,  // mediumpurple,
  142,  // olive,
  145,  // darkgray,
  149,  // yellowgreen,
  151,  // darkseagreen,
  153,  // skyblue,
  157,  // lightgreen,
  157,  // palegreen,
  160,  // firebrick,
  163,  // mediumvioletred,
  168,  // maroon,
  170,  // mediumorchid,
  172,  // chocolate,
  174,  // indianred,
  178,  // darkgoldenrod,
  179,  // peru,
  181,  // rosybrown,
  186,  // darkkhaki,
  187,  // tan,
  188,  // gray,
  189,  // lightsteelblue,
  191,  // greenyellow,
  195,  // lightblue,
  197,  // crimson,
  199,  // deeppink,
  201,  // fuchsia,
  202,  // orangered,
  209,  // coral,
  211,  // palevioletred,
  212,  // hotpink,
  213,  // orchid,
  214,  // darkorange,
  216,  // darksalmon,
  217,  // lightcoral,
  219,  // violet,
  223,  // burlywood,
  224,  // lightpink,
  225,  // thistle,
  226,  // gold,
  229,  // khaki,
  230,  // bisque,
  231,  // aliceblue,
  0,    // black,
  0,    // black,
  0     // black
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
  for (int row = 0; row < window_height; row += 2) {
    int previous_fg = -1, previous_bg = -1;
    for (int col = 0; col < window_width; ++col) {
      int bg = COLORS[result[row][col]];
      int fg = COLORS[result[row + 1][col]];
      if (previous_bg == bg && previous_fg == fg)
        printf("▄");
      else {
        printf("\e[38;5;%dm\e[48;5;%dm▄", fg, bg);
        previous_fg = fg;
        previous_bg = bg;
      }
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

  window_height = 2 * (w.ws_row - 1);
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
    char line[1024];
    char legend[200];

    set_window_size();
    calculate();
    draw();

    sprintf(legend, "X:%.10f Y:%.10f S:%e M:%d [%dx%d]", x_offset, y_offset, size,
            max, window_width, window_height);
    printf("\e];%s\007", legend);
    printf("%s (U)p,(D)own,(L)eft,(R)ight,(I)n,(O)ut,(P)lus,(M)inus > ",
           legend);
    if (fgets(line, sizeof(line), stdin)) {
      for (const char* ptr = &line[0]; *ptr; ++ptr) {
        char c = tolower(*ptr);
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
          c = (c == 'i') ? 'p' : 'm';
          // Fall through
        case 'p':
        case 'm':
          max = (c == 'p') ? max * 5 / 4 : max * 4 / 5;
          break;
        }
      }
    }
  }
}
