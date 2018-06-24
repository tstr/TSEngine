/*
	ImageLib extension
*/

#include <codecvt>
#include <fstream>
#include <algorithm>

#include <tscore/strings.h>
#include <tsgraphics/Driver.h>
#include <tsgraphics/schemas/Image.rcs.h>

using namespace std;

#define NOMINMAX
#include <Windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

static bool LoadTGAFile(const char* filename, tsr::ImageBuilder& builder);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool convertTexture2D(const std::string& textureFileName, const std::string& outputFileName)
{
	using ts::ImageFormat;
	using ts::ImageType;

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(textureFileName));

	/////////////////////////////////////////////////////////////////////////
	
	string extension;
	string filename(textureFileName);

	auto pos = filename.find_last_of('.');
	if (pos == string::npos)
		extension = filename;
	else
		extension = filename.substr(pos, filename.size() - pos);
	
	tsr::ImageBuilder builder;

	if (ts::compare_string_weak(extension, ".tga"))
	{
		if (!LoadTGAFile(filename.c_str(), builder))
		{
			return false;
		}
	}
	else
	{
		/////////////////////////////////////////////////////////////////////////
		//If the file is not a .tga file then load the texture using GDI+
		using namespace Gdiplus;

		vector<byte> buffer;
		
		//Load bitmap from file
		Bitmap* bmp = Bitmap::FromFile(wfilename.c_str());
		BitmapData bmpData;

		if (bmp->GetLastStatus())
		{
			return false;
		}

		ImageFormat format = ImageFormat::UNKNOWN;
		PixelFormat gdi_format = PixelFormatDontCare;

		switch (bmp->GetPixelFormat())
		{
		case PixelFormat8bppIndexed:
		{
			format = ImageFormat::BYTE;
			gdi_format = PixelFormat8bppIndexed;
			break;
		}
		case PixelFormat24bppRGB:
		{
			//Graphics API doesn't support texture formats with bytewidths not a power of 2 so we need to force extra colour channel for this texture
			format = ImageFormat::ARGB;
			gdi_format = PixelFormat32bppARGB;
			break;
		}
		case PixelFormat32bppARGB:
		default:
		{
			format = ImageFormat::ARGB;
			gdi_format = PixelFormat32bppARGB;
		}
		}

		/////////////////////////////////////////////////////////////////////////

		uint32_t sz = Gdiplus::GetPixelFormatSize(bmp->GetPixelFormat());

		Gdiplus::Rect dim(0, 0, bmp->GetWidth(), bmp->GetHeight());

		if (bmp->LockBits(&dim, ImageLockModeRead, gdi_format, &bmpData))
		{
			return false;
		}

		builder.set_format((uint32_t)format);
		builder.set_type((uint32_t)ImageType::_2D);

		builder.set_height(bmpData.Height);
		builder.set_width(bmpData.Width);
		builder.set_length(1);
		builder.set_mips(1);

		builder.set_data(builder.createArray((const ::byte*)bmpData.Scan0, bmpData.Stride * bmpData.Height));

		builder.set_byteWidth(bmpData.Stride);
		builder.set_byteDepth(0);

		auto s = bmp->UnlockBits(&bmpData);
		delete bmp;

		if (s)
		{
			return false;
		}
	}

	builder.set_signature(*reinterpret_cast<const uint32_t*>("TSTX"));

	ofstream outputFile(outputFileName, ios::binary | ios::out);
	builder.build(outputFile);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//GDI+ doesn't support TGA files so we have to use a separate loading mechanism
static bool LoadTGAFile(const char* filename, tsr::ImageBuilder& builder)
{
	using ts::ImageFormat;
	using ts::ImageType;

	struct TGAFILE
	{
		unsigned char imageTypeCode;
		short int imageWidth;
		short int imageHeight;
		unsigned char bitCount;
		unsigned char *imageData;
	};

	TGAFILE tgaFile;
	memset(&tgaFile, 0, sizeof(TGAFILE));

	FILE *filePtr = nullptr;
	unsigned char ucharBad;
	short int sintBad;
	long imageSize;
	int colorMode;
	//unsigned char colorSwap;

	// Open the TGA file.
	fopen_s(&filePtr, filename, "rb");
	if (filePtr == NULL)
	{
		return false;
	}

	// Read the two first bytes we don't need.
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

	// Which type of image gets stored in imageTypeCode.
	fread(&tgaFile.imageTypeCode, sizeof(unsigned char), 1, filePtr);

	// For our purposes, the type code should be 2 (uncompressed RGB image)
	// or 3 (uncompressed black-and-white images).
	if (tgaFile.imageTypeCode != 2 && tgaFile.imageTypeCode != 3)
	{
		fclose(filePtr);
		return false;
	}

	// Read 13 bytes of data we don't need.
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);

	// Read the image's width and height.
	fread(&tgaFile.imageWidth, sizeof(short int), 1, filePtr);
	fread(&tgaFile.imageHeight, sizeof(short int), 1, filePtr);

	// Read the bit depth.
	fread(&tgaFile.bitCount, sizeof(unsigned char), 1, filePtr);

	// Read one byte of data we don't need.
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

	// Color mode -> 3 = BGR, 4 = BGRA.
	colorMode = tgaFile.bitCount / 8;
	imageSize = tgaFile.imageWidth * tgaFile.imageHeight * colorMode;

	// Allocate memory for the image data.
	tgaFile.imageData = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

	// Read the image data.
	fread(tgaFile.imageData, sizeof(unsigned char), imageSize, filePtr);

	/*
	// Change from BGR to RGB so OpenGL can read the image data.
	for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
	{
	colorSwap = tgaFile.imageData[imageIdx];
	tgaFile.imageData[imageIdx] = tgaFile.imageData[imageIdx + 2];
	tgaFile.imageData[imageIdx + 2] = colorSwap;
	}
	*/

	vector<uint32_t> databuffer;
	databuffer.resize(tgaFile.imageHeight * tgaFile.imageWidth);


	for (int i = 0; i < imageSize; i += colorMode)
	{
		BYTE bgra[4];

		bgra[0] = tgaFile.imageData[i + 0];
		bgra[1] = tgaFile.imageData[i + 1];
		bgra[2] = tgaFile.imageData[i + 2];
		bgra[3] = (colorMode == 4) ? tgaFile.imageData[i + 3] : (BYTE)255;

		//volatile auto x = *reinterpret_cast<int*>(&);
		volatile auto x = (int)tgaFile.imageData[i + 0];
		//cout << 128 * *reinterpret_cast<float*>(&bgra) << endl;

		databuffer[i / colorMode] = *reinterpret_cast<uint32_t*>(&bgra);
	}

	fclose(filePtr);
	free(tgaFile.imageData);

	builder.set_format((uint32_t)ImageFormat::ARGB);
	builder.set_type((uint32_t)ImageType::_2D);

	builder.set_height(tgaFile.imageHeight);
	builder.set_width(tgaFile.imageWidth);
	builder.set_length(1);
	builder.set_mips(1);

	builder.set_data(builder.createArray((const uint8_t*)&databuffer[0], databuffer.size() * sizeof(uint32_t)));

	builder.set_byteWidth((uint32_t)(tgaFile.imageWidth * sizeof(uint32_t)));
	builder.set_byteDepth(0);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uintptr_t gdi_token;

PYBIND11_MODULE(imagelib, m)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup((ULONG_PTR*)&gdi_token, &gdiplusStartupInput, 0);

	m.def("convert2D", &convertTexture2D);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
