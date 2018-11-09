Matrices!!!

Matrices can be used to transform vectors.  And this is exactly how we use them in OpenGL.  Again, I could explain this, or provide a nice video tutorial:

Watch this one on vectors first:
https://www.youtube.com/watch?v=HYSNSoJZCgY

Then watch this one on matrices:
https://www.youtube.com/watch?v=V6sJpjYzfaQ

(one note on the matrix video is that at 7:12, he accidentally puts the translation in the wrong column of the matrix.  He accidentally put it in the 3rd column, but it's supposed to be in the 4th column)

And then this one for example use cases of matrices:
https://www.youtube.com/watch?v=pQcC2CqReSA

In the sample code, I've added some code for basic matrix operations.  I also modified the shader code to accept a matrix and then transform the incoming vertex positions by the matrix.

This sample will rotate our triangle about the Y axis.  I'd recommend modifying the code to rotate it about the x or z axis.