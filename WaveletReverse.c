// Wavelet decoder

uint8_t * WPtr = WSomeBuf + 0x400; 		// 0x529510

bool waveletSomeOtherTableDoneInit; 	// 0x54F8FC

///.data:0054F900 wavelet_BufNeg400 db 400h dup(?)
///.data:0054FD00 byte_54FD00     db 400h dup(?)
///.data:00550100                 db    ? ;
///.data:00550101                 db    ? ;
///.data:00550102                 db    ? ;
///.data:00550103                 db    ? ;

// Первая половина буфера для отрицательных индексов (-0x400 ... -1)
// Вторая половина (буфер + 0x400) для индексов >= 0 ( 0 ... 0x400 )

byte WSomeBuf[0x800]; 		// 0x54F900

bool waveletSomeTableDoneInit; 			// 0x550104

LoadCardWavelet()
{
	/// ...

	entrySize = CatalogReadEntry (CurrentCardArt, catEntry /*name*/, ctx + 0x1a0);

	/// DataSize

	*(uint32_t *)(ctx + 0x1a4) = entrySize - 0x9c;

	/// Copy WVL header at ctx + 0

	memcpy ( ctx, *(uint32_t *)(ctx + 0x1a0), 0x9c);

	/// Skip header

	*(uint32_t *)(ctx + 0x1a0) += 0x9c;

	if ( *(uint32_t *)(ctx + 0x28) == 4 )
	{
		*(uint32_t *)(ctx + 0x1c) *= 2;
		*(uint32_t *)(ctx + 0x20) *= 2;
	}

	/// ...

	CardWaveletDecode(...);
}

void * CardWaveletDecode (ctx, arg_4) 		// 0x491BFF
{
	dontFree = false;

	/// Some init 1: prepare WSomeBuf

	if (!waveletSomeTableDoneInit)
	{
		/// Never Used...

		int i = -0x400;

		while ( i <= 0x400 )
		{
			if ( i < 0)
			{
				WPtr[i] = 0;
			}
			else
			{
				if ( i <= 248 )
				{
					WPtr[i] = (i * 255) / 248;
				}
				else
				{
					WPtr[i] = 0xff;
				}
			}

			i++;
		}

		waveletSomeTableDoneInit = true;
	}

	/// Init 2

	if (arg_4 == 0)
	{
		int bytes = *(uint32_t *)(ctx + 0x90) + 2000; 		// Decompressed size + overhead for safety

		allocated = malloc (bytes);

		fast_memset ( allocated, bytes / 4 );
	}
	else
	{
		dontFree = true;
	}

	/// Entropy decode

	dword_5EF988 = CardWaveletUncomp (allocated, ctx);

	/// Check

	switch ( *(uint32_t *)(ctx + 0x28))
	{
		case 1:
			piecesDerived = 1;
			break;
		case 4:
			piecesDerived = 2;
			break;
		case 16:
			piecesDerived = 4;
			break;
		default:
			/// "wavelet pieces has illegal value: %d"
			break;		
	}

	/// Save dimensions

	widthFromCtx = *(uint32_t *)(ctx + 0x1c) / piecesDerived;
	width = widthFromCtx;

	int smallTableSize = *(uint32_t *)(ctx + 0x24);

	height = *(uint32_t *)(ctx + 0x20) / piecesDerived;

	/// Loop by pieces

	count = 0;

	while ( count < *(uint32_t *)(ctx + 0x28) )
	{
		/// Block 1

		if ( *(uint32_t *)(ctx))
		{
			/// var_14
			eax = width / piecesDerived;
			ecx = *(uint32_t *)(ctx + 0x28) - 1;
			if ( ecx == 0)
				var_14 = eax / 2;
			else
				var_14 = eax;

			/// var_24
			eax = height / piecesDerived;
			ecx = *(uint32_t *)(ctx + 0x28) - 1;
			if ( ecx == 0)
				var_24 = eax / 2;
			else
				var_24 = eax;			
		}
		else
		{
			var_14 = width;
			var_24 = height;
		}

		/// Wavelet decode

		eax = (width * width + var_14 * var_14 * 2 + 0x40) * count * 4;
	
		ptr1 = allocated + eax;
		ptr2 = ptr1 + width * width * 4 + 0x80;
		ptr3 = ptr2 + var_14 * var_14 * 4 + 0x80;

		CardWaveletDecode_Sub6 (ptr1, width, smallTableSize);
		CardWaveletDecode_Sub6 (ptr2, var_14, smallTableSize);
		CardWaveletDecode_Sub6 (ptr3, var_14, smallTableSize);

		eax = *(uint32_t)(ctx + 0x28) / 2;

		/// YCbCr -> RGB

		if ( count >= eax )
		{
			if ( *(uint32_t *)(ctx + 0x28) <= 1)
			{
				resultOut = CardWaveletDecode_YCbCrToRGB (
					0,
					ptr1,
					width,
					height,
					ptr2,
					ptr3,
					var_14,
					var_24,
					*(uint32_t *)(ctx) );
			}
			else
			{
				resultOut = CardWaveletDecode_YCbCrToRGB (
					0,
					ptr1,
					width,
					*(uint32_t *)(ctx + 0x20) - widthFromCtx,
					ptr2,
					ptr3,
					var_14,
					var_24,
					*(uint32_t *)(ctx) );
			}
		}
		else
		{
			resultOut = CardWaveletDecode_YCbCrToRGB ( 
				0,
				ptr1,
				width,
				width,
				ptr2,
				ptr3,
				var_14,
				var_14,
				*(uint32_t *)(ctx) );
		}

		/// Block 4

		if (*(uint32_t *)(ctx + 0x28) <= 1)
		{
			if (dontFree == 0)
			{
				free (allocated);
			}

			allocated = resultOut;
		}
		else
		{
			var_10 = *(uint32_t *)(ctx + 0x1c);

			mov     eax, [ebp+var_10]
			push    eax
			mov     eax, [ebp+var_10]
			push    eax
			mov     eax, [ebp+width]
			push    eax
			mov     eax, [ebp+width]
			push    eax
			mov     eax, [ebp+var_10]
			cdq
			idiv    [ebp+piecesDerived]
			mov     ecx, eax
			mov     eax, [ebp+count]
			cdq
			idiv    [ebp+piecesDerived]
			imul    ecx, eax
			push    ecx
			mov     eax, [ebp+var_10]
			cdq
			idiv    [ebp+piecesDerived]
			mov     ecx, eax
			mov     eax, [ebp+count]
			cdq
			idiv    [ebp+piecesDerived]
			imul    ecx, edx
			push    ecx
			mov     eax, [ebp+resultOut]
			push    eax
			mov     eax, [ebp+allocated]
			push    eax
			call    CardWaveletCopySmth
			add     esp, 20h

			free (resultOut);
		}

		count++;
	}

	return allocated;
}

/// Entropy data decoder

int CardWaveletUncomp (uint8_t * allocated, void * ctx) 			// 0x4928AF
{
	/// Block 1

	ecx = *(uint32_t *)(ctx + 0x28) - 1;
	ecx = (ecx == 0) ? 1 : 2;

	eax = *(uint32_t *)(ctx + 0x1c);
	var_30 = eax / ecx;
	var_2C = var_30;

	/// Block 2

	ecx = *(uint32_t *)ctx == 0 ? 1 : 2;
	var_18 = var_2C / ecx;
	var_10 = var_18;

	uint8_t * dataPtr = *(uint32_t *)(ctx + 0x1a0); 			// WVL + 0x9c (some data)
	int smallTableSize = *(uint32_t *)(ctx + 0x24); 		// 0x9 ( 9 * 9 * 4 = small index table size)

	eax = var_18 * var_10;
	ecx = var_30 * var_2C;
	var_C = ecx + 2 * eax;

	/// Step 1

	int bigIndexCount = *var_8;
	dataPtr += 4;

	uint8_t * bigIndexTabPtr = dataPtr;
	*(uint32_t *)bigIndexTabPtr = 0x80000000;

	dataPtr += bigIndexCount * 4; 		// Skip large index table

	dataPtr += Uncomp_Sub5 ( dataPtr, bigIndexTabPtr, bigIndexCount);

	/// Loop

	int pieceNum = 0; 		/// piece num

	while ( pieceNum < *(uint32_t *)(ctx + 0x28) )
	{
		/// Calculate pointers

		var_24 = allocated + (var_C + 0x40) * pieceNum * 4;
		var_38 = var_24 + var_30 * var_2C * 4 + 0x80;
		var_34 = var_38 + var_18 * var_10 * 4 + 0x80;		

		/// Small index table + data 1

		memcpy ( var_24, dataPtr, smallTableSize * smallTableSize * 4);
		var_24 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		int size = *(uint32_t *)(ctx + pieceNum * 4 + 0x5c);
		Uncomp_Sub9 (var_24, dataPtr, size);
		dataPtr += size;

		/// Small index table + data 2

		memcpy ( var_38, dataPtr, smallTableSize * smallTableSize * 4);
		var_38 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		int size = *(uint32_t *)(ctx + pieceNum * 4 + 0x6c);
		Uncomp_Sub9 ( var_38, dataPtr, size);
		dataPtr += size;

		/// Small index table + data 3

		memcpy (var_34, dataPtr, smallTableSize * smallTableSize * 4);
		var_34 += smallTableSize * smallTableSize * 4;
		dataPtr += smallTableSize * smallTableSize * 4;

		int size = *(uint32_t *)(ctx + pieceNum * 4 + 0x7c);
		Uncomp_Sub9 ( var_34, dataPtr, size);
		dataPtr += size;

		pieceNum++;
	}

	return 0;
}

/// Haar

void CardWaveletDecode_Sub6 (int32_t * ctab, int width, int tabSize) 					// 0x4922CB
{
	if ( !waveLetBuffersReady)
	{
		buf1 = waveLetBuffer_1 = malloc (0x32000);
		buf2 = waveLetBuffer_2 = malloc (0x32000);
		waveLetBuffersReady = 1;
	}
	else
	{
		buf1 = waveLetBuffer_1;
		buf2 = waveLetBuffer_2;
	}

	int size = tabSize;

	while ( size < width)
	{
		var_18 = ctab;
		var_C = ctab + size * size * 4;

		CardWaveletDecode_Sub7 (
			ctab,
			&ctab[size * size],
			buf1,
			size,
			size,
			2 * size,
			size );

		CardWaveletDecode_Sub7 (
			&ctab[size * size * 2],
			&ctab[size * size * 3],
			buf2,
			size,
			size,
			2 * size,
			size );

		CardWaveletDecode_Sub8 (
			buf1,
			buf2,
			ctab,
			size,
			2 * size,
			2 * size,
			2 * size);

		size *= 2;
	}
}

void CardWaveletDecode_Sub7 (							// 0x492410
	int32_t * tab1,
	int32_t * tab2,
	int32_t * tmp,
	arg_C,
	arg_10,
	arg_14, 		// unused
	arg_18,
)
{
	for (int i=0; i<arg_10; i++)
	{
		var_C = tab1;
		var_10 = &tab1[arg_C];
		var_8 = tmp + arg_18 * 8;
		tab1 += 4;

		/// Loop 2

		while ( tab1 < var_10 )
		{
			*var_8 = *tab1 + *tab2;
			var_8[arg_18] = *tab1 - *tab2;

			tab1++;
			tab2++;
			var_8 += arg_18 * 8;
		}

		////

		*tmp = *var_C + *tab2;
		tmp[arg_18] = *var_C - *tab2;

		tmp++;
		tab2++;
	}
}

void CardWaveletDecode_Sub8 ( 					// 0x4924E5
	int32_t * buf1,
	int32_t * buf2,
	int32_t * outTab,
	arg_C,
	arg_10,
	arg_14, 		/// unused
	arg_18)
{
	for (int i=0; i<arg_10; i++)
	{
		////

		var_8 = outTab;
		var_C = buf1;
		var_10 = &buf1[arg_C];

		var_8 += arg_18 * 8;
		buf1 += 4;

		/// Loop 2

		while ( buf1 < var_10)
		{
			*var_8 = (*buf1 + *buf2) / 2;
			var_8[arg_18] = (*buf1 - *buf2) / 2;

			buf1++;
			buf2++;
			var_8 += arg_18 * 8;
		}

		////

		*outTab = (*var_C + *buf2) / 2;
		outTab[arg_18] = (*var_C - *buf2) / 2;

		outTab++;
		buf2++;
	}
}

uint8_t SomeOtherTab[0x400 + 0x1c00];
uint8_t * SomeOtherTabPtr = &SomeOtherTab[0x400];

uint8_t * CardWaveletDecode_YCbCrToRGB (					// 0x4925E6
	uint8_t * rgbPtr, 					// always 0
	int32_t * ptr1, 			// YTab
	int width,
	int height,
	int32_t * ptr2,			// CbTab
	int32_t * ptr3, 		// CrTab
	int widthDerived,
	bool heightDerived,
	arg_24 					/// Not used (*ctx)
)
{
	/// Loop 1

	if (!waveletSomeOtherTableDoneInit)
	{
		i = -0x400;

		while (i < 0x1c00)
		{
			if ( i > 0)
			{
				eax = i >> 2;
				if (eax >= 0xff )
				{
					eax = 0xff;
				}
				SomeOtherTabPtr[i] = (uint8_t)eax;
			}
			else
			{
				SomeOtherTabPtr[i] = 0;
			}

			i++;
		}

		waveletSomeOtherTableDoneInit = 1;
	}

	////

	uint8_t * buf;

	if ( rgbPtr)
	{
		buf = rgbPtr;
	}
	else
	{
		buf = rgbPtr = malloc ( width * width * 3 + 0x10); 		/// Should be width * height (?)
	}

	for (int y=0; y<height; y++)
	{
		if (heightDerived)
		{
			var_20 = &ptr2[(y / 2) * widthDerived];
			var_1C = &ptr3[(y / 2) * widthDerived];
		}
		else
		{
			var_20 = &ptr2[y * widthDerived];
			var_1C = &ptr3[y * widthDerived];
 		}

		/// Loop 2

 		for (int x=0; x<width; x++)
		{
			yval = *ptr1;

			////

			if (heightDerived)
			{
				if ( x & 1)
				{
					eax = width - x - 1;
					eax = (eax == 0) ? -1 : 0;
					eax = var_20[eax + 1];
					eax = var_20[0] + eax;
					CbVal = eax / 2;

					eax = width - x - 1;
					eax = (eax == 0) ? -1 : 0;
					eax = var_1C[eax + 1];
					eax = var_1C[0] + eax;
					CrVal = eax / 2;
				}
				else
				{
					CbVal = var_20[0];
					CrVal = var_1C[0];
				}
			}
			else
			{
				CbVal = var_20[0];
				CrVal = var_1C[0];
			}

			////

			r = yval + CrVal + CrVal / 2 + CrVal / 8 - 0x333;
			b = yval + CbVal * 2 - 0x400;
			g = yval * 2 - yval / 4 - r / 2 - b / 4 - b / 16;

			rgbPtr[0] = SomeOtherTabPtr[b]; 		// B
			rgbPtr[1] = SomeOtherTabPtr[g]; 		// G
			rgbPtr[2] = SomeOtherTabPtr[r];  		// R

			////

			if (heightDerived)
			{
				if ( x & 1)
				{
					var_20 += 4;
					var_1C += 4;
				}
			}
			else
			{
				var_20 += 4;
				var_1C += 4;
			}

			ptr1++;
			rgbPtr += 3;
		}
	}

	return buf;
}

//////////////////////////////////////////////////////////////////////////////

/// Seems buggy
fast_memset (void * ptr, int dwordsNum) 			// 0x505730
{
	edi = ptr;
	ecx = dwordsNum;

	if ( edi & 4)
	{
		*(uint8_t *)edi = 0;
		edi += 4;
		ecx--;

		if ( ecx <= 0)
			return;
	}

	push ecx;
	ecx /= 2; 			// Should be /= 8
	ecx--;

	if (ecx >= 0)
	{
		fldz;

		while (ecx--)
		{
			*(uint64_t *)edi = 0;
			edi += 8;
		}

		*(uint64_t *)edi = 0;
	}

	pop ecx;
	if (ecx & 1)
	{
		*(uint8_t *)edi = 0;
	}
}

int Uncomp_Sub5 (uint8_t * data, uint32_t * indexTab, int indexCount) 			// 0x505D9A
{
	int bitsProcessed = 13;

	int i = 0;
	while (i < 32)
	{
		eax = 0xFFFFFFFF >> i;
		bitsDwordMask[i] = eax;
		i++;
	}

	//// Setup parameters

	bitstreamStart = bitstreamPtr = data;
	bitstreamLimit = 100000;
	bitsDword = 0;
	bitsLeft = 0;
	
	dword_552CD4 = GetBits (13);

	/// Loop

	i = 0;
	while ( i < dword_552CD4)
	{
		dword_552CE0[i].field_0 = GetBits(13);
		dword_552CE0[i].field_1 = GetBits(13);

		bitsProcessed += 26;
		i++;		
	}

	/// Step 1

	dword_563968 = indexCount;
	dword_552CD0 = indexTab;

	Uncomp_Sub6 (dword_552CD4);

	return (bitsProcessed + 7) / 8; 		/// Rounded up
}

int Uncomp_Sub6 (int count) 			// 0x505EB2
{
	var_6C = 0;

	uint32_t var_64[] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400 };

	int i = 0;
	while (i < 32)
	{
		var_EC[i] = 1;
		i++;
	}

	var_8 = count - 1;

	int var_38[1000];
	var_38[0] = var_8;

	/// Loop

	do
	{
		/////

		var_FC = var_6C++;

		if ( var_EC[var_FC] )
		{
			var_4 = dword_552CE0[var_8].field_0;
		}
		else
		{
			var_4 = dword_552CE0[var_8].field_1;
		}

		////

		var_8 = var_4 - dword_563968; 			// indexCount
		var_38[var_6C] = var_8;

		if ( var_8 >= 0)
		{
			if ( var_6C == 8 )
			{
				var_F8 = 0;
				var_C = 0;

				while ( var_F8 < var_6C)
				{
					var_C |= var_EC[var_F8] << var_F8;
					var_F8++;
				}

				dword_562D68[3 * var_C] = 0x7FFFFFFF;
				dword_562D6C[3 * var_C] = var_6C;
				dword_562D70[3 * var_C] = var_8;

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
			var_F0 = 0;
			var_C = 0;

			while ( var_F0 < var_6C)
			{
				var_C |= var_EC[var_F0] << var_F0;
				var_F0++;
			}

			//// 

			i = 0;

			while ( i < var_64[8 - var_6C] )
			{
				eax = i << var_6C;
				var_F4 = eax;
				ecx = var_6C | var_F4;
				ecx = 3 * ecx;
				dword_562D6C[ecx] = eax;

				ecx = var_F4 | var_C;
				ecx = 3 * ecx;
				dword_562D68[ecx] = &dword_552CD0[var_4]; 		// indexTab

				eax = var_F4 | var_C;
				eax = 3 * eax;
				dword_562D70[eax] = 0xFFFFFFFF;

				i++;
			}

			////

			var_EC[var_6C] = 1;
			var_6C--;
			var_EC[var_6C]--;
			var_8 = var_38[var_6C];
		}

		/// Loop

		while ( var_EC[0] >= 0)
		{
			if ( var_EC[var_6C] < 0 )
			{
				var_EC[var_6C] = 1;
				var_6C--;
				var_EC[var_6C]--;
				var_8 = var_38[var_6C];
			}
		}		


	} while (var_EC[0] >= 0);

	return 0;
}

int Uncomp_Sub9 (int32_t * destPtr, uint8_t * data, int dataSize) 				// 0x506453
{
	uint32_t * savedPtr = destPtr;

	bitstreamPtr = data;
	bitstreamStart = data;
	bitstreamLimit = dataSize;

	if (data & 3)
	{
		/// Unaligned pointer

		int unalignedBytes = 4 - (data & 3);

		bitsLeft = unalignedBytes * 8;

		uint32_t mask = 0xFFFFFFFF >> (32 - bitsLeft);
		bitsDword = *(uint32_t *)data & mask;

		bitstreamPtr += unalignedBytes;
	}
	else
	{
		bitsLeft = 0;
		bitsDword = 0;
	}

	var_C = GetBits (8);

	while (var_C != 0xFFFFFFFF)
	{
		if (dword_562D68[3 * var_C] >= 0x7FFFFFFF )
		{
			var_14 = dword_562D70[3 * var_C];

			do
			{
				var_18 = GetNextBit();

				if (var_18 == 0xFFFFFFFF)
					goto loc_5066BB;

				if (var_18)
				{
					var_10 = dword_552CE0[var_14].field_0;
				}
				else
				{
					var_10 = dword_552CE0[var_14].field_1;
				}

				var_14 = var_10 - dword_563968; 			// big indexCount

			} while (var_14 >= 0);

			if ( var_10 )
			{
				*destPtr++ = dword_552CD0[var_10];
			}
			else
			{
				var_2C = GetBits (10);

				if (var_2C >= 0)
				{
					fast_memset (destPtr, var_2C);

					destPtr += var_2C;
				}
			}

loc_5066BB:
			var_C = GetBits (8);
		}
		else
		{
			var_24 = dword_562D6C[3 * var_C];

			if (dword_562D68[3 * var_C] == 0x80000000)
			{
				var_28 = GetBits (var_24 + 2);

				if (var_28 < 0)
					break;

				var_28 = (var_28 << (8 - var_24)) | (var_C >> var_24);

				fast_memset (destPtr, var_28);

				destPtr += var_28;

				var_C = GetBits (8);
			}
			else
			{
				*destPtr++ = dword_562D68[3 * var_C];
				var_C >>= var_24;
				var_20 = GetBits (var_24);

				if (var_20 == 0xFFFFFFFF)
					break;

				eax = var_20;
				edx = 8 - var_24;
				cl = dl;
				eax <<= cl;
				var_C |= eax;
			}

		}

	}

	return (destPtr - savedPtr) / 4;
}

uint32_t GetBits (int bits) 			// 0x506AD0
{
	uint32_t value;

	if ( bits <= bitsLeft)
	{
		value = bitsDword & bitsDwordMask[32 - bits];
		bitsDword >>= bits;
		bitsLeft -= bits;
		return value;
	}
	else
	{
		value = bitsDword;

		bits -= bitsLeft;
		int bytesLeft = bitstreamPtr - bitstreamStart + 4;

		if (bytesLeft < bitstreamLimit)
		{
			bitsDword = *(uint32_t *)bitstreamPtr;
			bitstreamPtr += 4;

			value |= (bitsDword & bitsDwordMask[32 - bits]) << bitsLeft;

			bitsDword >>= bits;
			bitsLeft = 32 - bits;
			
			return value;
		}
		else
		{
			return 0xFFFFFFFF;
		}
	}
}

int GetNextBit () 					// 0x506BF0
{
	int bitOut = 0;

	if (bitsLeft == 0)
	{
		if ( (bitstreamStart - bitstreamPtr) < bitstreamLimit)
		{
			bitsDword = *(uint32_t *)bitstreamPtr;
			bitstreamPtr += 4;
			bitsLeft = 32;
		}
		else
		{
			return 0xFFFFFFFF;
		}
	}

	if (bitsDword & 1)
		bitOut = 1;

	bitsDword >>= 1;
	bitsLeft--;

	return bitOut;
}
