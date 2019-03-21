#include "pch.h"

bool waveLetBuffersReady;
int32_t * waveLetBuffer_1;
int32_t * waveLetBuffer_2;

void CardWaveletDecode_Sub7(							// 0x492410
	int32_t * tab1,
	int32_t * tab2,
	int32_t * tmp,
	int arg_C,
	int arg_10,
	int arg_14,
	int arg_18
)
{
	for (int i = 0; i < arg_10; i++)
	{
		int32_t * var_C = tab1;
		int32_t * var_10 = &tab1[arg_C];
		int32_t * var_8 = &tmp[arg_18 * 2];
		tab1++;

		while (tab1 < var_10)
		{
			*var_8 = *tab1 + *tab2;
			var_8[arg_18] = *tab1 - *tab2;

			tab1++;
			tab2++;
			var_8 += arg_18 * 2;
		}

		*tmp = *var_C + *tab2;
		tmp[arg_18] = *var_C - *tab2;

		tmp++;
		tab2++;
	}
}

void CardWaveletDecode_Sub8( 					// 0x4924E5
	int32_t * buf1,
	int32_t * buf2,
	int32_t * outTab,
	int arg_C,
	int arg_10,
	int arg_14,
	int arg_18)
{
	for (int i = 0; i < arg_10; i++)
	{
		int32_t * var_C = buf1;
		int32_t * var_10 = &buf1[arg_C];
		int32_t * var_8 = &outTab[arg_18 * 2];
		buf1++;

		while (buf1 < var_10)
		{
			*var_8 = (*buf1 + *buf2) / 2;
			var_8[arg_18] = (*buf1 - *buf2) / 2;

			buf1++;
			buf2++;
			var_8 += arg_18 * 2;
		}

		*outTab = (*var_C + *buf2) / 2;
		outTab[arg_18] = (*var_C - *buf2) / 2;

		outTab++;
		buf2++;
	}
}

void WaveletDecode (int32_t * ctab, int width, int tabSize) 					// 0x4922CB
{
	int32_t *buf1, *buf2;

	if (!waveLetBuffersReady)
	{
		buf1 = waveLetBuffer_1 = new int32_t[0x10000];
		buf2 = waveLetBuffer_2 = new int32_t[0x10000];
		waveLetBuffersReady = true;
	}
	else
	{
		buf1 = waveLetBuffer_1;
		buf2 = waveLetBuffer_2;
	}

	int size = tabSize;

	while (size < width)
	{
		CardWaveletDecode_Sub7(
			ctab,
			&ctab[size * size],
			buf1,
			size,
			size,
			2 * size,		// not used
			size);

		CardWaveletDecode_Sub7(
			&ctab[size * size * 2],
			&ctab[size * size * 3],
			buf2,
			size,
			size,
			2 * size,		// not used
			size);

		CardWaveletDecode_Sub8(
			buf1,
			buf2,
			ctab,
			size,
			2 * size,
			2 * size,		// not used
			2 * size);

		size *= 2;
	}
}

///////////////////////////////////////////////////////////

bool YCbCrTabReady;
uint8_t YCbCrTab[0x400 + 0x1c00];
uint8_t * YCbCrTabPtr = &YCbCrTab[0x400];

uint8_t * Decode_YCbCrToRGB(					// 0x4925E6
	uint8_t * rgbPtr, 					// always 0
	int32_t * YTab,
	int width,
	int height,
	int32_t * CbTab,
	int32_t * CrTab,
	int derivedWidth,
	int derivedHeight
)
{
	/// Prepare table

	if (!YCbCrTabReady)
	{
		int i = -0x400;

		while (i < 0x1c00)
		{
			if (i > 0)
			{
				int eax = i >> 2;
				if (eax >= 0xff)
				{
					eax = 0xff;
				}
				YCbCrTabPtr[i] = (uint8_t)eax;
			}
			else
			{
				YCbCrTabPtr[i] = 0;
			}

			i++;
		}

		YCbCrTabReady = true;
	}

	/// Allocate RGB buffer

	uint8_t * buf;

	if (rgbPtr)
	{
		buf = rgbPtr;
	}
	else
	{
		buf = rgbPtr = new uint8_t[width * width * 3 + 0x10]; 		/// Should be width * height (?)
	}

	for(int y=0; y<height; y++)
	{
		int32_t * CbPtr;
		int32_t * CrPtr;

		if (derivedHeight)
		{
			CbPtr = &CbTab[(y / 2) * derivedWidth];
			CrPtr = &CrTab[(y / 2) * derivedWidth];
		}
		else
		{
			CbPtr = &CbTab[y * derivedWidth];
			CrPtr = &CrTab[y * derivedWidth];
		}

		for (int x=0; x<width; x++)
		{
			int32_t yval = *YTab;
			int32_t CbVal;
			int32_t CrVal;

			////

			if (derivedHeight)
			{
				if (x & 1) 			/// Lerp
				{
					int eax = width - x - 1;
					eax = (eax == 0) ? -1 : 0;
					eax = CbPtr[eax + 1];
					CbVal = (CbPtr[0] + eax) / 2;

					eax = width - x - 1;
					eax = (eax == 0) ? -1 : 0;
					eax = CrPtr[eax + 1];
					CrVal = (CrPtr[0] + eax) / 2;
				}
				else
				{
					CbVal = CbPtr[0];
					CrVal = CrPtr[0];
				}
			}
			else
			{
				CbVal = CbPtr[0];
				CrVal = CrPtr[0];
			}

			////

			int r = yval + CrVal + CrVal / 2 + CrVal / 8 - 0x333;
			int b = yval + CbVal * 2 - 0x400;
			int g = yval * 2 - yval / 4 - r / 2 - b / 4 - b / 16;

			rgbPtr[0] = YCbCrTabPtr[b]; 		// B
			rgbPtr[1] = YCbCrTabPtr[g]; 		// G
			rgbPtr[2] = YCbCrTabPtr[r];  		// R

			/// Advance pointers

			if (derivedHeight)
			{
				if (x & 1)
				{
					CbPtr++;
					CrPtr++;
				}
			}
			else
			{
				CbPtr++;
				CrPtr++;
			}

			YTab++;
			rgbPtr += 3;
		}
	}

	return buf;
}
