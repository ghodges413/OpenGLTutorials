Hey guys,

I know it's been a while since I've sent a programming email.  I figured that's probably over due to finally get around to fulfilling the promise of providing OpenGL tutorials.

Anyway, this one is crazy simple.  It almost doesn't even do anything.  All this does is setup GLUT, GLEW, and then renders a green window.

GLUT is a free cross-platform windowing API (application programming interface).  It simply allows you to create a window and can be used to grab mouse and keyboard input; that's it.  It's not very powerful, but it is super simple.  And it allows us to create an OpenGL context with great ease.  So we'll be using it for these tutorials since we don't need much other than that.

http://en.wikipedia.org/wiki/OpenGL_Utility_Toolkit


GLEW is a free library that allows us to connect our calls to OpenGL to the installed driver implementation.  OpenGL is nothing more than a specification for interfacing with a graphics card.  It's up to the vendor (Nvidia or ATI) to supply an implementation of OpenGL in their drivers.  And GLEW is a means of connecting our application to the driver at runtime (without it, we'd have to do this ourselves).

http://en.wikipedia.org/wiki/OpenGL_Extension_Wrangler_Library


So, this tutorial is basically just setting these things up.

The code will create a window and then fill it with green.  By all means, I recommend building/running it and then try modifying the code.  At least change the color from green to red or purple or blue or something other than green.

Let me know if there's any issues.

The next tutorial will also be super simple.

Note:  There's a 3rd party compiled dll contained in this... so I had to change the zip file extension to zop... just rename it to a zip in order to use it.