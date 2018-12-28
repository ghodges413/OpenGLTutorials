Texturing!

I know that I just sent a tutorial on shaders.  But I've already got this tutorial on texturing ready.

Normally, I'd explain texturing briefly here, but I found a very nice youtube video explaining it:

https://www.youtube.com/watch?v=Eh0HeTCCgnE

One comment that I need to make about the video is that he claims he never found a use for GL_CLAMP_TO_EDGE.  In the professional world, we always use clamp to edge.  We almost never use GL_REPEAT.  It's nice that the repeat mode is there and it has its uses... but we usually don't want it, because it can hide issues with bad texture coordinates in any model art that artists generate.

Now for a couple of comments on the code.  I had to add code for loading textures from file.  Here, we're loading targas (TGA) using the code in Targa.h/cpp.  The texture is then loaded to OpenGL using the code in Texture.h/cpp.

We also have new shader code for displaying the texture.

I also changed the vertex format.  We no longer need the color information in the vertex (since it's now in the texture).  But we do need to put texture coordinates in the vertex.  So check out the new vertex format too.
