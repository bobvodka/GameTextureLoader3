========================================================================
Game Texture Loader 3 Beta 1 Release
========================================================================

This is the Beta 1 release for GTL3; it should be stable/production ready
however it hasn't had many real world tests.

GTL3 Tester is a test application which shows basic usage of the API.

Real docs are forthcoming :)

eLutHeaderGenerator & HalfHeaderGenerator are used to generate header files
required for 16bit floating point support and will built and executed before 
Game Texture Loader 3 builds.

For Win32 you'll need to link to the winsock library in your app provided you
don't comment out/otherwise disable the remote downloading (will be some kind
of compile option in future).

Key API facts:

LoadTexture can throw GameTextureLoader3::DecoderNotFoundException if you pass
it a fail it doesn't understand.

Callbacks are called in the context of the loading thread, so if you perform an
async load then these loading threads will call the callback; important when it
comes to OpenGL usage!

State progression for loading is:
- Pending
- Loading
- Loaded
- Processing
- Completed

Anytime after 'Loading' and before 'Completed' can result in an error. 

Formats handled:
- Uncompressed BMP (1,4,8,16,24 & 32bbp resampled to 24bpp as required)
- DDS including
-- extra images
-- mipmap support
-- 3D images (NOT re-origined)
-- Compressed and uncompressed formats, including 16 and 32bit Floating point
- JPG (RGB)
- PNG (RGB, RGBA, Grey, Grey Alpha in 8 and 16bit per channel)
- TGA (compressed and uncompressed RGBA and BGR)


To use:
- Compile library and put in library path somewhere
- Place includes directory into path
- place #include <glt/GameTextureLoader.hpp> in source file
- Link to gtl3.lib
- Link to Ws2_32.lib

/////////////////////////////////////////////////////////////////////////////
