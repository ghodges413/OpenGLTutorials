Hey Guys,

So this tutorial is going to do the same exact thing as the previous one.  The difference here is that I've changed the format of the vertex data.  The old way used two different arrays, one for positions and the other for colors:

// Positions array
float vertices[ 9 ] = {
	-0.5f, -0.5f, 0.0f,	// lower left corner
	0.5f, -0.5f, 0.0f,	// lower right corner
	0.0f, 0.5f, 0.0f	// top
};

// Create the color array
float colors[ 9 ] = {
	1.0f, 0.0f, 0.0f,	// color red
	0.0f, 1.0f, 0.0f,	// color green
	0.0f, 0.0f, 1.0f,	// color blue
};

This one uses a single array.  I've packed that position and color information into a single struct and then created an array of the struct.  This is called interlacing:

// The new interlaced format of the vertices
struct vert_t {
	Vec3d position;
	Vec3d color;
};

// Declaration of the vertices
const int gNumVertices = 3;
vert_t gVertices[ gNumVertices ];

This also means that we had to change, slightly, how we pass our vertex data to OpenGL:

// The spacing between each vertex
const int stride = sizeof( vert_t );

// Send the vertex data to opengl
glVertexPointer( 3, GL_FLOAT, stride, &gVertices[ 0 ].position[ 0 ] );
glColorPointer( 3, GL_FLOAT, stride, &gVertices[ 0 ].color[ 0 ] );

Here, we pass in the starting pointer for the vertex positions and the vertex colors.  But the really important part is the stride.  Before stride was zero because there wasn't any spacing between the positions in the position array and there wasn't any spacing between the colors in the color array.  But with this packed (interlaced) vertex format, there is spacing between them.  So we have to use the sizeof() function to let OpenGL know the size of that spacing.

Now, you might be asking "why are we doing this?".  There are two good reasons.  The first is organizational, it's simpler, as our vertex formats get more complicated, to pack them into a struct.  The other is for caching.

I won't try to get too into this.  But basically, a long time ago memory was faster than a CPU (think 80s).  And no one worried about their data structures too much, simply because the memory was faster than the CPU.  So the CPU could grab memory from anywhere without waiting for the data very long.

As time went on CPU speeds increased drastically faster than the speed of the memory in the computer (think 90s).  CPU manufacturers then started putting caches (small amounts of memory) on their processors.  And now, the data that the CPU needs to work on is copied from main memory to the cache and the CPU works exclusively on the cache... if the data isn't already present on the cache, then the data needs to be copied from main memory to cache.  This is called a cache miss and it takes a long time (hundreds and even thousands of clock cycles).  This is also true for the GPU.

Now, for a simple triangle, all the position and color information could be stored in the cache and it's not a big deal.  But when we start dealing with models that have a million vertices, there's no way for all the data to fit in the cache.  And if the color for a vert is a million points away from position for the same vert, then we'd have a cache miss for every single vert and it would take forever to render the model.  But, if we pack the color and position data into a single struct, then they're not far away from each other in memory and we won't have a cache miss when the GPU tries to process that vert.  And that's the real reason why we pack (interlace) our vertex information into a single struct.

This is also known as an Array of Structs vs Struct of Arrays.  Googling that will give more information.