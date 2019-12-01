# Printelbrot
Print out Mandelbrot set fractals in a terminal window

## Motivation

There must be a thousand Mandelbrot programs out there. This one focuses on
simplicity and speed. It's designed to be run from the command line in a
terminal window. It prints the familiar view of a Mandelbrot set, and then
enters a loop where it prompts for commands to change parameters for viewing
the fractal, and then redraws the screen after receiving commands.

## Commands

|Letter|Function|
|------|--------|
|U|Move up|
|D|Move down|
|L|Move left|
|R|Move right|
|I|Zoom in|
|O|Zoom out|
|P|Increase (plus) the maximum number of iterations|
|M|Decrease (minus) the maximum number of iterations|

Note that case is not important, and that multiple commands can be entered at a
time. For example

```
(U)p,(D)own,(L)eft,(R)ight,(I)n,(O)ut,(P)lus,(M)inus > ilo
```

Changing the resolution depends on which terminal program is used, but it will
typically be `Crtl` `+` and `Ctrl` `-`. After changing resolution, type
`Return` to redraw the screen.