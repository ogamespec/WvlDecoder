
#pragma once

void WaveletDecode(int32_t * ctab, int width, int tabSize);

uint8_t * Decode_YCbCrToRGB(
	uint8_t * rgbPtr,
	int32_t * YTab,
	int width,
	int height,
	int32_t * CbTab,
	int32_t * CrTab,
	int derivedWidth,
	int derivedHeight);
