# Game Texture Loader 3

### About

Despite the name this was the second generation of the Game Texture Loader library - version 2 was reserved for a breaking API change in the previous version.

This code dates from sometime around 2008 and was considered 'beta' at the time.

### Core Concepts

This version removes the heavy Boost dependency of the previous version, however it still retains the usage of the various shared point/array and function/bind functionality - this was due to the lack of those concepts in the C++ compiler in use at the time (Visual Studio 2005).

Like the previous version the library could flip loaded images so they matched any required orientation, this applied to all built in image formats - DXT images remained compressed during the flip operations.

The system also suppported async loading and would use a number of worker threads and a single callback function to indicate when the work was done. This function was optional and the state could be queried directly from an 'Image' object instead.

Unlike the previous version there was no DLL options; instead the library was designed to be statically linked.

The library also had built in libcurl support so that images could be fetched remotely if required.

### Notes

This was the final version of the code for this project.

It comes with a test program which tested/exercised the various loading requirements - bonus code; PNG save functionality using libpng.

This library depends on libpng, zlib, libjpg and libcurl to build - none of which are included.

Has Visual Studio 2005 solution.

Licensed under MIT - see the license text for details.