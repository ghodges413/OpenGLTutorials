Skinning

Alright, this one is also not really an opengl tutorial.  Instead it's a tutorial on how to skinned meshes.

In a typical game, you'll have characters that run around.  Mostly, these characters are human, but they can also be animals.

Now, how do we render a character?  Well, we could have a static model for every single frame in an animation and then just render that particular static model.  However, that adds to a massive increase in storage usage, since we have to keep a ton of static models.  Or we can also define a skeleton each for character and then have an animation that simply defines different skeletons.  We then have a mesh that's attached to the skeleton and as the skeleton moves, it deforms the mesh.  This is also called skinned mesh animation or skeletal animation.

http://en.wikipedia.org/wiki/Skeletal_animation

Now, since we've decided on skeletal animation, we need to decide on a format.  For this tutorial I've decided on the md5mesh/md5anim format.  This was the format that was used in Doom3.  And thanks to the modding community, it's a very well supported format.  And there's a very wonderful write up explaining the format:

http://tfc.duke.free.fr/coding/md5-specs-en.html

Now, in order to support this, we need even more math.  Specifically we need quaternions.

http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation

It's a pretty simple mathematical object that can be used to represent rotations.  And it does this from its algebra.

Quaternions are an extension of the complex numbers.  A complex number is an imaginary number.  Let's define i:
i * i = -1

Then a complex number is:
c = a + i * b

where a and b are just typical numbers.

If we also require a * a + b * b = 1.  Then complex numbers can be used to represent rotations in the xy-plane.

Then multiplying two complex numbers is the same as a rotation.  Let:
d = x + i * y

Then:
c * d = ( a + i * b ) * ( x + i * y )
= a * x + a * i * y + i * b * x + i * b * i * y
= a * x - b * y + i ( a * y + b * x)

This turns out to be a rotation in the xy-plane.  Check for yourself with a = 1, b = 0, x = 0, y = 1.  And try a few others like a = -1, b = 0, x = 0, y = 1.

Quaternions are similar... where:
i * i = -1
j * j = -1
k * k = -1
i * j = k
j * k = i
k * i = j
j * i = -k
k * j = -i
i * k = -j
i * j * k = -1

Then a quaternion is defined as:
q = a + i * b + j * c + k * d

And if a * a + b * b + c * c + d * d = 1, then multiplying two quaternions together is a rotation.

Anyway, this sample takes the Hell knight from Doom3 and animates him in his idle animation.  The real magic code takes place in MD5Model.h/cpp and MD5Anim.h/cpp.  But I did have to add a quaternion class and a matrix class for supporting the extra math that has to happen.