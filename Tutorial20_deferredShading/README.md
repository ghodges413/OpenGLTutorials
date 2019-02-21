Deferred Shading

I know I'm going really really fast.  Especially as these tutorials get more and more complicated.  But I have another friend that wanted a copy of these, so I'm going fast for his benefit.  Of course it also works out for you guys... even if it's difficult to consume these as quickly as I'm moving.

Anyway, so far anytime we've rendered a model, we've had to light it immediately.  In fact, up until 10 years ago, this was how lighting was performed.  Anytime you wanted to render a model with a light, you had to render the whole model for that one light.  So if you had 3 lights affecting a model, then you'd have to render that model 3 different times.  In fact, that's how the Doom 3 renderer worked.  Fabien Sanglard gives a great over view of this:

http://fabiensanglard.net/doom3/renderer.php

Basically, the rendering loop looked like this:

foreach( model in scene ) {
   foreach( light affecting model ) {
      Draw( model, light );
   }
}

This was actually rather wasteful, but it was the only way to do it back then.  However, starting with the Xbox 360 and PS3, a new rendering technique became available and very popular.  It's called deferred shading or deferred lighting.  Games like Alan Wake used this technique.

http://en.wikipedia.org/wiki/Deferred_shading

The technique works basically like this.. you render all your models to an off screen buffer called a geometry buffer or g-buffer.  We've rendered to off screen buffers before but we only ever had a color buffer and depth buffer attached.  This time, we have a color buffer (for the diffuse color) a position buffer (for the world space positions of the meshes) and a normal buffer that stores the world space normal vectors of the rendered models.

Once the off screen g-buffer is filled.  We render our lights, using the information that's stored in the g-buffer.  Now our rendering looks like this:

foreach( model in scene ) {
   DrawToGBuffer( model );
}

foreach( light in scene ) {
   Draw( light );
}

This makes our rendering loop much more efficient.

In this sample, we're only rendering non-shadowing point lights, but the technique extends to spotlights and directional lights.  Since we're only using point lights, you just render a sphere, and use the sphere's screen space coordinates to look up the information in the g-buffer.  Then the lighting is calculated as usual.

I've also attached a screenshot of the program in action.  On the left, you can see the g-buffer.  And then in the center, you can see the fully lit scene in action.
