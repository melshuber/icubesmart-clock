# icubesmart-clock
iCubeSmart clock

Based on the DIY this repository contains a clock application for the
cube.

## Resources

The following link contains Schematics and original software

* https://drive.google.com/file/d/1YYw6fDU2zbKVlcF7XnJDQR83HCsZ6HyP/view

Note: The schematic states that the MCU is an STC89C51RD+ however my
kit uses the newer replacement part STC12C5A60S2. The implementation
requires the later since it used the PCA as timer fore timekeeping.

Usefull Youtube link:

* https://www.youtube.com/watch?v=4a3OMpjdTn8

# Compile

## Toolchain

The project compiles with sdcc version 4.1.0 (http://sdcc.sourceforge.net, https://sourceforge.net/p/sdcc/code/HEAD/tree/tags/sdcc-4.1.). I compiled the toolchain myself and had some problems with autotools. To fix them:

```
pushd sdcc/device/lib/pic14
autoreconf
popd
pushd sdcc/device/lib/pic16
autoreconf
popd
```

or just exclude them by:

```
./configure --disable-pic14-port --disable-pic16-port
```

## Build

The `/src` directory contains a `Makefile` with relevant targets. All
variables defined within the `Makefile` default to sensible values,
but can be overridden using `make VAR=VALUE [target] ..`.

* `make all`:  Build `icube.ihx`
* `make clean`:  Clean build artifacts
* `make flash`: Flash `icube.ihx` onto the device. *Do not forget to
  properly set the variable `TTY`*. Flashing requires to tool
  `stcgal`. It is available as python package: `pip3 install stcgal`.
* `make sim`: Start the simulation.
* `make console`: Start a UART console connected to the device.#
* `make settime`: Use the host time to set the current time on the device.

# Usage

The device exposes a simple command interface on the UART (115200
BAUD/8N1). The following commands are implemented.

* *Set time* to set a time send the following string and a newline
  (either CR or LF work) onto the UART console:

  `S:YYYYMMDDhhmmss`

* *Reboot to IAP* to reboot into IAP send the following string and a
  newline (either CR or LF work) onto the UART console:

  `R`

  The implementation should work, but when combining with `stcgal -a
  -r <script>` `stcgal` still does not detect the device, although
  reboot is delayed within a busy wait loop for a couple of hundreds ms.

# Implementation

HW Ressource allocation:

* Timer 0: Framebuffer output.
* primary UART: Command Interface and debug output (stdout) (115200 BAUD/8N1)
* PCA channel 1: Timekeeping for clock.

## frambuffer.c

The device allocates two framebuffers, a front and a back framebuffer,
to a void tearing.

The framebuffer is driven by Timer 0 configured to run at 800Hz. On
each tick the ISR copies one plane into the external buffer registers (U1,
U2, U4, U6, U8, U10, U14, U16).

When reaching the last plane, the ISR checks if the back buffer
contains a new frame and flips the framebuffers if appropriate.

## time.c

This files contains the functions for timekeeping. The current time is
stored within `_time`. Fields intended to display a digit are encoded
as BCD. The field `tick` counts sub-seconds (up to `TIME_TICK_HZ`). The
field `ticks` is a 16 Bit wrapping tick counter.

The time is derived from PCA Channel 1, which is configured to run at
`TIME_TICK_HZ` (50Hz).

## uart.c

The UART is used as debug output when running on the device. Since
transmission is interrupt driven **characters transmitted within a
critical section are discarded**. To output data C-API call like
`printf` can be used.

### putchar

`putchar` is implemented to transmit data either to the UART or to the
simulation interface. The simulation interface is used if detected
during `uart_init`.

## render.c, fixed-point.c

Why? Because we can! The application implements a render engine with
the core function `render_tex2D`.

### Coordinate System

The coordinate system is defined as follows. Cube coordinates range
from -4 to 4 and each axes. The LEDs are positioned in the center of
the pixels.

```

	-4  -3  -2  -1   0   1   2   3   4
  -4 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
  -3 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
  -2 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
  -1 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
   0 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
   1 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
   2 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
   3 +---+---+---+---+---+---+---+---+
	 | L | L | L | L | L | L | L | L |
   4 +---+---+---+---+---+---+---+---+
```

The x-axis addresses bits within the external buffer registers (U1,
U2, U4, U6, U8, U10, U14, U16). Thy y-axis addresses the buffer
registers (Schematics L1 to L8) and the z-axis addresses the vertical
plane anodes (Schematics G1 to G8)

Example: (-3.5, -3.5, -3.5) maps to the LED addressed by (Bit 0 in
U1, lowest plane driven by G1).

### render_tex2D

This function takes a transformation
matrix a texture and a framebuffer. The matrix is used to transform
framebuffer coordinates into texture coordinates. *Note: the cube is
transformed into texture coordinates not vice-versa.*

Assuming the matrix is the identity the texture is spans (-4, -4, 0) -
(4, 4, 0) where as the LSB of texture[0] will be transformed on to
(-3.5, -3.5, 0) and the MSB of texture[7] will be transformed to (3.5,
3.5, 0).

The assembly version will render a texture at a rate off ~100Hz.  The
implementation avoids multiplications within the loops. During the
loop base vectors (`ux`, `uy`, `uz`) are used. Those are computed at
the in a preprocessing step. The initial translation is stored in
`pz`. `pz` along with `px` and `py` are used as counters.

For sure there is still room for optimizations, e.g.: in general it is
not necessary to iterate over the whole 512 LEDs in the cube, when
rendering a single plane.

### fixed-point.c

This is a support library for fixed point scalar and matrix
operations (floats wont do the job on this MCU).

#### sPI equals 2.0 isn't it?

No, its not, PI equals ~3.14. Therefor the constant is called sPI. The
constant is PI scaled to value exactly representable by fixed point
numbers. 2.0 is selected since a quarter circumference of a circle
well thus be 1.0. The advantages are:

* Integer wrap-around will not break continuity of *sin* and *cos*
  function.
* Looking up in tables can be done by simply shifting the value.

## sim.c

Utility functions interfacing with the simulator interface (see HTML
documentation of SDCC).
