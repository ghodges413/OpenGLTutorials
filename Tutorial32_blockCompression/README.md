# Block Compression
![Block Compression](screenshot.jpg)

Block compression is a common lossy texture compression format that's supported by modern gpus.  It gets its name by compressing 4x4 blocks of texels at a time.
Each block finds a line of best fit in the color space of the texels and maps each texel value to the line.

References:

https://mrelusive.com/publications/papers/Real-Time-YCoCg-DXT-Compression.pdf

https://mrelusive.com/publications/papers/Real-Time-Normal-Map-Dxt-Compression.pdf

"Real-time BC6H Compresson on GPU" by Krzysztof Narkowicz in GPU Pro 7

https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11

https://learn.microsoft.com/en-us/windows/win32/direct3d11/bc7-format

https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/

https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt