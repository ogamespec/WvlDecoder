// Decomp.cpp: WVL decompress
//

#include "pch.h"

// https://www.technical-recipes.com/2011/creating-bitmap-files-from-raw-pixel-data-in-c/

// Save the bitmap to a bmp file  
void SaveBitmapToFile(BYTE* pBitmapBits,
	LONG lWidth,
	LONG lHeight,
	WORD wBitsPerPixel,
	const unsigned long& padding_size,
	LPCTSTR lpszFileName)
{
	// Some basic bitmap parameters  
	unsigned long headers_size = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER);

	unsigned long pixel_data_size = lHeight * ((lWidth * (wBitsPerPixel / 8)) + padding_size);

	BITMAPINFOHEADER bmpInfoHeader = { 0 };

	// Set the size  
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);

	// Bit count  
	bmpInfoHeader.biBitCount = wBitsPerPixel;

	// Use all colors  
	bmpInfoHeader.biClrImportant = 0;

	// Use as many colors according to bits per pixel  
	bmpInfoHeader.biClrUsed = 0;

	// Store as un Compressed  
	bmpInfoHeader.biCompression = BI_RGB;

	// Set the height in pixels  
	bmpInfoHeader.biHeight = lHeight;

	// Width of the Image in pixels  
	bmpInfoHeader.biWidth = lWidth;

	// Default number of planes  
	bmpInfoHeader.biPlanes = 1;

	// Calculate the image size in bytes  
	bmpInfoHeader.biSizeImage = pixel_data_size;

	BITMAPFILEHEADER bfh = { 0 };

	// This value should be values of BM letters i.e 0x4D42  
	// 0x4D = M 0×42 = B storing in reverse order to match with endian  
	bfh.bfType = 0x4D42;
	//bfh.bfType = 'B'+('M' << 8); 

	// <<8 used to shift ‘M’ to end  */  

	// Offset to the RGBQUAD  
	bfh.bfOffBits = headers_size;

	// Total size of image including size of headers  
	bfh.bfSize = headers_size + pixel_data_size;

	// Create the file in disk to write  
	HANDLE hFile = CreateFile(lpszFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// Return if error opening file  
	if (!hFile) return;

	DWORD dwWritten = 0;

	// Write the File header  
	WriteFile(hFile,
		&bfh,
		sizeof(bfh),
		&dwWritten,
		NULL);

	// Write the bitmap info header  
	WriteFile(hFile,
		&bmpInfoHeader,
		sizeof(bmpInfoHeader),
		&dwWritten,
		NULL);

	// Write the RGB Data  
	WriteFile(hFile,
		pBitmapBits,
		bmpInfoHeader.biSizeImage,
		&dwWritten,
		NULL);

	// Close the file handle  
	CloseHandle(hFile);
}

void BmpDemo()
{
	BYTE* buf = new BYTE[128 * 3 * 128];
	int c = 0;

	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			buf[c + 0] = (BYTE)255;
			buf[c + 1] = (BYTE)0;
			buf[c + 2] = (BYTE)0;

			c += 3;
		}
	}

	SaveBitmapToFile((BYTE*)buf,
		128,
		128,
		24,
		4,
		"image_created.bmp" );

	delete[] buf;
}

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		printf("Use: Decomp <file.wvl>\n");
		return -1;
	}

	/// Load WVL

	FILE *f = NULL;
	fopen_s(&f, argv[1], "rb");
	if (!f)
		return -2;

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t * image = new uint8_t[size];

	fread(image, 1, size, f);
	fclose(f);

	/// Allocate uncompressed data buffer

	int bytes = *(uint32_t *)(image + 0x90) + 20000;

	uint8_t * dest = new uint8_t[bytes];

	memset(dest, 0, bytes);

	/// Uncompress

	VlcDecompress (dest, image);

	/// Wavelet

	int width = *(uint32_t *)(image + 0x1c);
	int height = *(uint32_t *)(image + 0x20);
	int smallTableSize = *(uint32_t *)(image + 0x24);

	int var_14;
	int var_24;

	if (*(uint32_t *)(image))
	{
		/// var_14
		int eax = width;
		int ecx = *(uint32_t *)(image + 0x28) - 1;
		if (ecx == 0)
			var_14 = eax / 2;
		else
			var_14 = eax;

		/// var_24
		eax = height;
		ecx = *(uint32_t *)(image + 0x28) - 1;
		if (ecx == 0)
			var_24 = eax / 2;
		else
			var_24 = eax;
	}
	else
	{
		var_14 = width;
		var_24 = height;
	}

	uint8_t * ptr1 = dest;
	uint8_t * ptr2 = ptr1 + width * width * 4 + 0x80;
	uint8_t * ptr3 = ptr2 + var_14 * var_14 * 4 + 0x80;

	WaveletDecode ((int32_t *)ptr1, width, smallTableSize);
	WaveletDecode ((int32_t *)ptr2, var_14, smallTableSize);
	WaveletDecode ((int32_t *)ptr3, var_14, smallTableSize);

	/// YCbCr -> RGB

	uint8_t * rgbBuf = Decode_YCbCrToRGB(nullptr,
		(int32_t *)ptr1,
		width,
		height,
		(int32_t *)ptr2,
		(int32_t *)ptr3,
		var_14,
		var_24);

	/// Save image

	SaveBitmapToFile((BYTE*)rgbBuf,
		width,
		height,
		24,
		4,
		"output.bmp");

	delete rgbBuf;

	return 0;
}
