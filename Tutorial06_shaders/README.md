Alright, so last time we did the interlaced vertex format.  This time, we're going to introduce shaders.  Everything that we've done, up until now, has used the fixed function capabilities of OpenGL.  Fixed function was the old fashioned way of rasterizing triangles.  It was great, in its time, of just drawing textured triangles (we haven't hit textures yet).  But for doing anything more advanced, we'll need to use shaders.

A shader is code that is run on the GPU.  For this introduction we will use a vertex shader and a fragment shader (there's also geometry, tessellation and compute shaders, but we'll get to those later).  Shader code is written in a language called GLSL.  GLSL is a very simple c-like language.

A vertex shader tells the GPU how to transform the individual vertices that we pass into OpenGL.

The triangles are then rasterized and converted into on screen pixels or fragments.  The fragment shader then tells the GPU how the final fragments (pixels) are lit/textured/colored/etc.

The inclusion of shaders allows us to perform much more advanced rendering techniques.  And this is how we'll do all of our rendering from here on out.

I also had to add a bunch more code to the project to get shaders working.  Don't worry too much about the code.  I've encapsulated it all in a class called Shader.  Right now, all the important code to worry about is located in Shader::UseProgram(), Shader::SetVertexAttribPointer() and Shader::LoadFromFile().

The shader code itself is located in simple text files located at:
data/shaders/colors.vsh
data/shaders/colors.fsh

Definitely look at the shader code to see what it does.  This introduction uses very simple shader code and is really the simplest shader code that can be written.  But this will allow us to eventually do more advanced things like lighting and shadowing.

Let me know if you have any questions.
