# Printelbrot
Print out Mandelbrot set fractals in a terminal window

## Motivation

There must be a thousand Mandelbrot programs out there. This one focuses on
simplicity and speed. It's designed to be run from the command line in a
terminal window. It prints the familiar view of a Mandelbrot set, and then
enters a loop where it prompts for commands to change parameters for viewing
the fractal, and then redraws the screen after receiving commands.

## Usage

    $ ./mandelbrot.c [starting-point-parameters]

### Example

`X`, `Y`, and `S` (x offset, y offset, and size) take floating point numbers
with decimal point and optionally an exponent part. `M` (max iterations) takes
an integer argument. These values are always printed before the command prompt
when an image is rendered, so they can me copied from there and saved for later
invocations.

    $ ./mandelbrot.c X:-1.189 Y:0.3 S:2.750000e-01 M:200

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
X:-0.745332 Y:0.105208 S:3.356934e-05 M:250 (U)p,(D)own,(L)eft,(R)ight,(I)n,(O)ut,(P)lus,(M)inus > ilo
```

Changing the resolution depends on which terminal program is used, but it will
typically be `Crtl` `+` and `Ctrl` `-`. After changing resolution, type
`Return` to redraw the screen.
