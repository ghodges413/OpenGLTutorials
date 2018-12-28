Projective Texturing

Projective texturing is a very important texturing technique.  The basic concept works the same way that a film projector or a flashlight works.

Basically, if you had an image (like a film negative) and attached to the front of a flashlight.  Then you'd see that image projected onto any surface that you shine the light on.

So how do we do this?  Well, we simply create a view matrix and a projection matrix for the flashlight.  Then when we render the model, we also transform it by the light's matrices and then use the transformed vertices as texturing coordinates.

Here's a white paper that briefly describe it.  It also covers what our next tutorial will be.
http://cseweb.ucsd.edu/~ravir/6160/papers/shadow_mapping.pdf