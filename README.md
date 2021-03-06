## Light Speed!

This is a fork of Light Speed! by @iskunk trying to port the app to GTK2 and, potentially, to GTK3 at some point. The original project's website is http://lightspeed.sourceforge.net/.

#### Build instructions:

~~~
autoreconf -i
./configure
make
make install
~~~

#### The README from the original project

==== Light Speed! ====


DESCRIPTION

This is a simulator designed to illustrate relativistic effects on the
appearance of objects moving at ultra-high velocities.

It simulates a geometrical lattice object traveling at a given speed in a
straight line through space. Specifically, the object moves along the
x-axis of this space, in the positive-x direction; at time t=0, the object
is precisely centered on the origin.

The simulator displays, in essence, a still image of the vicinity of the
origin snapped at the exact moment that the object is *seen* to be at the
origin; i.e. the moment that light reflected off the object, when the
object was exactly at the origin (at t=0), eventually reaches the camera.
This moment will usually be a few billionths of a second after t=0,
accounting for the time needed for the light to traverse this distance.
(The camera's info display will explicitly indicate the time of the
snapshot, to 0.001 microsecond accuracy).

Light Speed! will allow you to move around and see interactively how the
object appears to change with observer position. At low velocities, this
isn't much the case, but up in the several-million-meters-per-second
range, the object as viewed from one angle to another can vary
significantly.

To get started, drag the slider at the right of the window upward.
Relativistic effects will immediately become noticeable. Click and drag in
the graphics window to move around.


CONTROLS

Most interactive control is performed with the mouse. By holding down a
particular button, and dragging the pointer around, various camera motions
can be obtained:

  Left button:  Revolve camera around view target

  Shift key + Left button:  Revolve view target around camera

  Middle button:  Translate camera left, right, up or down

  Right button:  Dolly in or out

The first and last motions are generally the most useful. Should the
camera become difficult to control at any point, it may be re-initialized
by selecting Camera -> Reset View.


THEORY

Light Speed! takes into account the following consequences of special
relativity:

  1. Lorentz contraction     (fast objects appear shorter)
  2. Doppler red/blue shift  (the Doppler effect on light/color)
  3. Headlight effect        (redistribution of reflected light)
  4. Optical aberration      (warping due to the finite speed of light)

A lengthier treatment of the above phenomena, complete with the formulae
used (really, they're not that bad!) may be found in the file MATH.


LIMITATIONS

The only major weakness of this simulator is color. The Doppler shifts
produced are only approximations, and in certain cases may be wildly
inaccurate. If color fidelity is a must, Antony Searle's BACKLIGHT
raytracer program may be a better choice. (See the Acknowledgements
section below for details).


REQUIREMENTS

Light Speed! requires the X Window System with OpenGL or Mesa3D support,
as well as the GTK+ libraries to function. Compilation will additionally
need Janne Lof's GtkGLArea widget. See the file INSTALL for details.


ACKNOWLEDGEMENTS

I would like to thank Antony Searle, of the Australian National
University, for allowing me to make use of the Doppler-shift code from his
well-received relativistic raytracer, BACKLIGHT. This program illustrates
relativistic effects via a four-dimensional raytracing engine, and I
highly recommend it in case greater scientific rigor or creative
flexibility is desired. You may find it at:
http://www.anu.edu.au/Physics/Searle/


=========================================
Daniel Richard G. // <skunk@alum.mit.edu>
