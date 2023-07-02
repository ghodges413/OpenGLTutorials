#Virtual Texturing

The implementation here is outlined in JP Van Waveren's white paper, "Software Virtual Textures".  The paper can be found on his website: https://mrelusive.com/publications/papers/Software-Virtual-Textures.pdf

This is only half of a proper virtual texturing implementation.  It does not stream texture pages from file, transcoding them from a compressed format on disk to compressed BC5.  Instead this only displays cpu generated debug texture pages.