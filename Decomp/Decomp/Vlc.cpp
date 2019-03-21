
#include "pch.h"

static int bitsLeft;
static uint8_t * bitstreamStart;
static uint8_t * bitstreamPtr;
static int bitstreamLimit;
static uint32_t bitsDword;
static uint32_t bitsDwordMask[32];

int Tab1Count;

typedef struct _Tab1Entry
{
	int32_t field_0;
	int32_t field_1;
} Tab1Entry;

static Tab1Entry Tab1[0x2000];

static int SavedIndexCount;
static int32_t * SavedIndexTab;

typedef struct _Tab2Entry
{
	int32_t field_0;
	int32_t field_1;
	int32_t field_2;
} Tab2Entry;

static Tab2Entry Tab2[0x100];

//////////////////////////////////////////////////////////////////////////////

int32_t GetBits(int bits) 			// 0x506AD0
{
	uint32_t value;

	if (bits <= bitsLeft)
	{
		value = bitsDword & bitsDwordMask[32 - bits];
		bitsDword >>= bits;

		bitsLeft -= bits;
		return (int32_t)value;
	}
	else
	{
		value = bitsDword;

		bits -= bitsLeft;
		int bytesLeft = (int)(bitstreamPtr - bitstreamStart + 4);

		if (bytesLeft < bitstreamLimit)
		{
			bitsDword = *(uint32_t *)bitstreamPtr;
			bitstreamPtr += 4;

			value |= (bitsDword & bitsDwordMask[32 - bits]) << bitsLeft;

			bitsDword >>= bits;
			bitsLeft = 32 - bits;

			return (int32_t)value;
		}
		else
		{
			return -1;
		}
	}
}

int GetNextBit() 					// 0x506BF0
{
	int bitOut = 0;

	if (bitsLeft == 0)
	{
		if ((bitstreamStart - bitstreamPtr) < bitstreamLimit)
		{
			bitsDword = *(uint32_t *)bitstreamPtr;
			bitstreamPtr += 4;
			bitsLeft = 32;
		}
		else
		{
			return -1;
		}
	}

	if (bitsDword & 1)
		bitOut = 1;

	bitsDword >>= 1;
	bitsLeft--;

	return bitOut;
}

void DumpTab1(int entries)
{
	for (int i = 0; i < entries; i++)
	{
		printf("%i: [%i, %i]\n", i, Tab1[i].field_0, Tab1[i].field_1);
	}
}

void DumpTab2()
{
	for (int i = 0; i < 0x100; i++)
	{
		printf("%i: [0x%08X, 0x%08X, 0x%08X]\n", i, 
			Tab2[i].field_0,
			Tab2[i].field_1, 
			Tab2[i].field_2);
	}
}

int Vlc_GenTab2(int count) 			// 0x505EB2
{
	int32_t var_6C = 0;

	uint32_t powersOfTwo[] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400 };
	int32_t var_EC[32];

	uint32_t i = 0;
	while (i < 32)
	{
		var_EC[i] = 1;
		i++;
	}

	int32_t var_8 = count - 1;

	int32_t var_38[100];
	var_38[0] = var_8;

	/// Loop

	do
	{
		/////

		int var_FC = var_6C++;
		int32_t var_4;

		if (var_EC[var_FC])
		{
			var_4 = Tab1[var_8].field_0;
		}
		else
		{
			var_4 = Tab1[var_8].field_1;
		}

		////

		var_8 = var_4 - SavedIndexCount;
		var_38[var_6C] = var_8;

		if (var_8 >= 0)
		{
			if (var_6C == 8)
			{
				int var_F8 = 0;
				int var_C = 0;

				while (var_F8 < var_6C)
				{
					var_C |= var_EC[var_F8] << var_F8;
					var_F8++;
				}

				Tab2[var_C].field_0 = (int32_t)0x7FFFFFFF;
				Tab2[var_C].field_1 = var_6C;
				Tab2[var_C].field_2 = var_8;

				var_EC[var_6C] = 1;
				var_6C--;
				var_EC[var_6C]--;
				var_8 = var_38[var_6C];
			}
			else
			{
				continue;
			}
		}
		else
		{
			int var_F0 = 0;
			int var_C = 0;

			while (var_F0 < var_6C)
			{
				var_C |= var_EC[var_F0] << var_F0;
				var_F0++;
			}

			//// 

			i = 0;

			while (i < powersOfTwo[8 - var_6C])
			{
				int var_F4 = i << var_6C;

				Tab2[var_F4 | var_C].field_0 = SavedIndexTab[var_4];
				Tab2[var_F4 | var_C].field_1 = var_6C;
				Tab2[var_F4 | var_C].field_2 = -1;

				i++;
			}

			////

			var_EC[var_6C] = 1;
			var_6C--;
			var_EC[var_6C]--;
			var_8 = var_38[var_6C];
		}

		/// Loop

		while (var_EC[0] >= 0)
		{
			if (var_EC[var_6C] < 0)
			{
				var_EC[var_6C] = 1;
				var_6C--;
				var_EC[var_6C]--;
				var_8 = var_38[var_6C];
			}
			else
				break;
		}

	} while (var_EC[0] >= 0);

	return 0;
}

int Vlc_GenTabs(uint8_t * data, int32_t * indexTab, int indexCount) 			// 0x505D9A
{
	int bitsProcessed = 13;

	int i = 0;
	while (i < 32)
	{
		bitsDwordMask[i] = 0xFFFFFFFF >> i;
		i++;
	}

	/// Setup bitstream

	bitstreamStart = bitstreamPtr = data;
	bitstreamLimit = 100000;
	bitsDword = 0;
	bitsLeft = 0;

	Tab1Count = GetBits(13);

	/// Loop

	i = 0;
	while (i < Tab1Count)
	{
		Tab1[i].field_0 = GetBits(13);
		Tab1[i].field_1 = GetBits(13);

		bitsProcessed += 26;
		i++;
	}

	DumpTab1(Tab1Count);

	/// Step 1

	SavedIndexCount = indexCount;
	SavedIndexTab = indexTab;

	Vlc_GenTab2(Tab1Count);

	DumpTab2();

	return (bitsProcessed + 7) / 8;
}

int Vlc_DecompressChunk (int32_t * destPtr, uint8_t * data, int dataSize) 				// 0x506453
{
	int32_t * savedPtr = destPtr;

	bitstreamPtr = data;
	bitstreamStart = data;
	bitstreamLimit = dataSize;

	if ((unsigned)data & 3)
	{
		/// Unaligned pointer

		int unalignedBytes = 4 - ((unsigned)data & 3);

		bitsLeft = unalignedBytes * 8;

		uint32_t mask = 0xFFFFFFFF >> (32 - bitsLeft);
		bitsDword = (*(uint32_t *)data) & mask;

		bitstreamPtr += unalignedBytes;
	}
	else
	{
		bitsLeft = 0;
		bitsDword = 0;
	}

	int32_t var_C = GetBits(8);

	while (var_C != -1)
	{
		if ( Tab2[var_C].field_0 == (int32_t)0x7FFFFFFF)
		{
			int32_t var_14 = (int)Tab2[var_C].field_2;
			int32_t var_10;

			do
			{
				int var_18 = GetNextBit();

				if (var_18 == -1)
					goto loc_5066BB;

				if (var_18)
				{
					var_10 = Tab1[var_14].field_0;
				}
				else
				{
					var_10 = Tab1[var_14].field_1;
				}

				var_14 = var_10 - SavedIndexCount;

			} while (var_14 >= 0);

			if (var_10)
			{
				/// Set as SavedIndex

				*destPtr = SavedIndexTab[var_10];
				destPtr++;
			}
			else
			{
				/// Clear

				int zeroes = GetBits(10);

				if (zeroes >= 0)
				{
					memset(destPtr, 0, 4 * zeroes);
					destPtr += zeroes;
				}
			}

		loc_5066BB:
			var_C = GetBits(8);
		}
		else
		{
			int32_t var_24 = Tab2[var_C].field_1;

			if ( Tab2[var_C].field_0 == (int32_t)0x80000000)
			{
				int var_28 = GetBits(var_24 + 2);
				if (var_28 < 0)
					break;

				var_28 = (var_28 << (8 - var_24)) | (var_C >> (uint8_t)var_24);

				/// Clear

				memset(destPtr, 0, var_28 * 4);
				destPtr += var_28;

				var_C = GetBits(8);
			}
			else
			{
				/// Set as F0

				*destPtr = Tab2[var_C].field_0;
				destPtr++;

				var_C >>= (uint8_t)var_24;

				int var_20 = GetBits(var_24);
				if (var_20 == -1)
					break;

				var_C |= var_20 << (8 - var_24);
			}
		}
	}

	return (destPtr - savedPtr) / (int)sizeof(savedPtr[0]);
}

int VlcDecompress(uint8_t * destPtr, uint8_t * srcPtr) 			// 0x4928AF
{
	/// Block 1

	int ecx = *(uint32_t *)(srcPtr + 0x28) - 1;
	ecx = (ecx == 0) ? 1 : 2;

	int eax = *(uint32_t *)(srcPtr + 0x1c);
	int var_30 = eax / ecx;
	int var_2C = var_30;

	/// Block 2

	ecx = *(uint32_t *)srcPtr == 0 ? 1 : 2;
	int var_18 = var_2C / ecx;
	int var_10 = var_18;

	uint8_t * dataPtr = srcPtr + 0x9c; 			// skip header
	int smallTableSize = *(uint32_t *)(srcPtr + 0x24);

	eax = var_18 * var_10;
	ecx = var_30 * var_2C;
	int var_C = ecx + 2 * eax;

	/// Step 1

	int bigIndexCount = *(uint32_t *)dataPtr;
	dataPtr += 4;

	int32_t * bigIndexTabPtr = (int32_t *)dataPtr;
	*bigIndexTabPtr = (int32_t)0x80000000;

	dataPtr += bigIndexCount * 4; 		// Skip large index table

	dataPtr += Vlc_GenTabs(dataPtr, bigIndexTabPtr, bigIndexCount);

	/// Loop

	uint32_t pieceNum = 0; 		/// piece num

	while (pieceNum < *(uint32_t *)(srcPtr + 0x28))
	{
		int size;

		/// Calculate pointers

		uint8_t * var_24 = destPtr + (var_C + 0x40) * pieceNum * 4;
		uint8_t * var_38 = var_24 + var_30 * var_2C * 4 + 0x80;
		uint8_t * var_34 = var_38 + var_18 * var_10 * 4 + 0x80;

		/// Small index table + data 1

		memcpy(var_24, dataPtr, smallTableSize * smallTableSize * 4);
		var_24 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		size = *(uint32_t *)(srcPtr + pieceNum * 4 + 0x5c);
		Vlc_DecompressChunk ((int32_t *)var_24, dataPtr, size);
		dataPtr += size;

		/// Small index table + data 2

		memcpy(var_38, dataPtr, smallTableSize * smallTableSize * 4);
		var_38 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		size = *(uint32_t *)(srcPtr + pieceNum * 4 + 0x6c);
		Vlc_DecompressChunk ((int32_t *)var_38, dataPtr, size);
		dataPtr += size;

		/// Small index table + data 3

		memcpy(var_34, dataPtr, smallTableSize * smallTableSize * 4);
		var_34 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		size = *(uint32_t *)(srcPtr + pieceNum * 4 + 0x7c);
		Vlc_DecompressChunk ((int32_t *)var_34, dataPtr, size);
		dataPtr += size;

		pieceNum++;
	}

	return 0;
}
