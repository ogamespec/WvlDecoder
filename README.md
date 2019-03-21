# WvlDecoder

Microprose Magic: The Gathering (1997) wavelet decoder.

![Aswan Jaguar](/images/0868.bmp)

## Overview

Surprisingly, this old game contains an implementation of Haar wavelet image compression.

The .WVL files stored in the .CAT archives are compressed using VLC compression, similar to Huffman.

The data of the image itself is stored in YCbCr fixed-point pixel format (FPU is not used in the decompression algorithm at all).

The sample application (Decomp) is based on the reverse engineering of the executable file (WaveletReverse.c).
