# binaryimagelibrary
Portable library for binary (bi-valued) image processing

This is a general-pupose library for processing binary images. Anything
the operates essentially on set / unset pixels belongs here.

The principles are that all source is clean ANSI C with no dependencies
other than the standard library, and that binary images are passed
about as int foo_binaryfunc(unsigned char *binary, int width, int height).
That is, we use one byte per pixel. We avoid const, because we'll 
often be using the extra seven bits as temporary buffers for the
more complex algorithms.

We will try not to make the files dependent on each other, so anyone
can take a source and drop it into his code.

The objective is to be a service to the development community.

