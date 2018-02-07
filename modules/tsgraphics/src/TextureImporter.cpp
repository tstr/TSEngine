/*
	Texture Importer source
*/

#include "TextureImporter.h"

#include <tsgraphics/api/RenderDef.h>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//todo: replace gdi+ with a custom texture loader
#include <Windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <codecvt>
#include <string>

using namespace std;
using namespace ts;
using namespace Gdiplus;

static std::string PrintPixelFormat(Gdiplus::PixelFormat format);
static std::string PrintGDIstatus(Gdiplus::Status status);
static bool LoadTGAFile(const char* filename, STextureResourceDesc& desc, vector<uint32>& databuffer);

enum EState
{
	eStateFree = 0,
	eStateLoad = -1,
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTextureImporter::CTextureImporter()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	tsassert(!Gdiplus::GdiplusStartup(&m_token, &gdiplusStartupInput, 0));
}

CTextureImporter::~CTextureImporter()
{
	Gdiplus::GdiplusShutdown(m_token);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Load a texture from a given file
int CTextureImporter::load(const Path& filepath, STextureLoadInfo& info)
{
	//tsassert(m_state != EState::eStateLoad);
	//m_state = EState::eStateLoad;

	tsinfo("Loading texture: \"%\"...", filepath.str());

	m_buffer.clear();

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(filepath.str()));

	/////////////////////////////////////////////////////////////////////////

	string extension;
	string filename(filepath.str());

	auto pos = filename.find_last_of('.');
	if (pos == string::npos)
		extension = filename;
	else
		extension = filename.substr(pos, filename.size() - pos);

	if (compare_string_weak(extension, ".tga"))
	{
		if (!LoadTGAFile(filename.c_str(), info.desc, m_buffer))
		{
			tswarn("Texture failed to load.");
			return false;
		}

		info.data = (const void*)&m_buffer[0];
		info.byteWidth = (uint32)(info.desc.width * sizeof(uint32));
		info.byteDepth = 0;
	}
	else
	{
		/////////////////////////////////////////////////////////////////////////
		//If the file is not a .tga file then load the texture using GDI+
		using namespace Gdiplus;

		//Load bitmap from file
		Bitmap* bmp = Bitmap::FromFile(wfilename.c_str());
		BitmapData bmpData;

		if (bmp->GetLastStatus())
		{
			tswarn("Texture failed to load, GDI+ error: %", PrintGDIstatus(bmp->GetLastStatus()));
			return false;
		}

		ETextureFormat format = eTextureFormatUnknown;
		PixelFormat gdi_format = PixelFormatDontCare;

		switch (bmp->GetPixelFormat())
		{
		case PixelFormat8bppIndexed:
		{
			format = ETextureFormat::eTextureFormatByte;
			gdi_format = PixelFormat8bppIndexed;
			break;
		}
		case PixelFormat24bppRGB:
		{
			//Graphics API doesn't support texture formats with bytewidths not a power of 2 so we need to force extra colour channel for this texture
			format = ETextureFormat::eTextureFormatColourARGB;
			gdi_format = PixelFormat32bppARGB;
			break;
		}
		case PixelFormat32bppARGB:
		default:
		{
			format = ETextureFormat::eTextureFormatColourARGB;
			gdi_format = PixelFormat32bppARGB;
		}
		}

		tsinfo("GDI+ texture format: %", PrintPixelFormat(bmp->GetPixelFormat()));

		/////////////////////////////////////////////////////////////////////////

		uint32 sz = Gdiplus::GetPixelFormatSize(bmp->GetPixelFormat());

		Gdiplus::Rect dim(0, 0, bmp->GetWidth(), bmp->GetHeight());
		Gdiplus::Status s = bmp->LockBits(&dim, ImageLockModeRead, gdi_format, &bmpData);

		if (s)
		{
			tswarn("Texture failed to lock bits, GDI+ error: %", PrintGDIstatus(s));
			return false;
		}

		ETextureFormat texformat = format;

		info.desc.texformat = texformat;
		info.desc.textype = ETextureResourceType::eTypeTexture2D;
		info.desc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		info.desc.height = bmpData.Height;
		info.desc.width = bmpData.Width;
		info.desc.useMips = true;
		info.desc.arraySize = 1;

		m_buffer.resize(bmpData.Width * bmpData.Height);
		memcpy(&m_buffer[0], bmpData.Scan0, bmpData.Stride * bmpData.Height);

		info.data = (const void*)&m_buffer[0];
		info.byteWidth = bmpData.Stride;
		info.byteDepth = 0;

		s = bmp->UnlockBits(&bmpData);
		delete bmp;

		if (s)
		{
			tserror("Texture failed to load.");
			return false;
		}
	}

	return true;
}

//Unload a texture
void CTextureImporter::unload(STextureLoadInfo& info)
{
	//tsassert(m_state == EState::eStateLoad);
	//m_state = EState::eStateFree;

	info.byteDepth = 0;
	info.byteWidth = 0;
	info.data = nullptr;
	info.desc = STextureResourceDesc();

	m_buffer.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TGA loader
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//GDI+ doesn't support TGA files so we have to use a separate loading mechanism
static bool LoadTGAFile(const char* filename, STextureResourceDesc& desc, vector<uint32>& databuffer)
{
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

	databuffer.clear();
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

		databuffer[i / colorMode] = *reinterpret_cast<uint32*>(&bgra);
	}

	fclose(filePtr);
	free(tgaFile.imageData);

	desc.texformat = eTextureFormatColourARGB;
	desc.textype = eTypeTexture2D;
	desc.texmask = eTextureMaskShaderResource;
	desc.height = tgaFile.imageHeight;
	desc.width = tgaFile.imageWidth;
	desc.useMips = true;
	desc.arraySize = 1;
	desc.multisampling.count = 1;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define ppf(x) case x: { return std::string(#x); }

std::string PrintGDIstatus(Gdiplus::Status status)
{
	using namespace Gdiplus;
	switch (status)
	{
		ppf(Status::Aborted)
			ppf(Status::AccessDenied)
			ppf(Status::FileNotFound)
			ppf(Status::FontFamilyNotFound)
			ppf(Status::FontStyleNotFound)
			ppf(Status::GdiplusNotInitialized)
			ppf(Status::GenericError)
			ppf(Status::InsufficientBuffer)
			ppf(Status::InvalidParameter)
			ppf(Status::NotImplemented)
			ppf(Status::NotTrueTypeFont)
			ppf(Status::ObjectBusy)
			ppf(Status::Ok)
			ppf(Status::OutOfMemory)
			ppf(Status::PropertyNotFound)
			ppf(Status::PropertyNotSupported)
			ppf(Status::UnknownImageFormat)
			ppf(Status::UnsupportedGdiplusVersion)
			ppf(Status::ValueOverflow)
			ppf(Status::Win32Error)
			ppf(Status::WrongState)
	}

	return "Unknown";
}

std::string PrintPixelFormat(Gdiplus::PixelFormat format)
{
	switch (format)
	{
		ppf(PixelFormat16bppARGB1555)
			ppf(PixelFormat16bppGrayScale)
			ppf(PixelFormat16bppRGB555)
			ppf(PixelFormat16bppRGB565)
			ppf(PixelFormat1bppIndexed)
			ppf(PixelFormat24bppRGB)
			ppf(PixelFormat32bppARGB)
			ppf(PixelFormat32bppCMYK)
			ppf(PixelFormat32bppPARGB)
			ppf(PixelFormat32bppRGB)
			ppf(PixelFormat48bppRGB)
			ppf(PixelFormat4bppIndexed)
			ppf(PixelFormat64bppARGB)
			ppf(PixelFormat64bppPARGB)
			ppf(PixelFormat8bppIndexed)
	}

	return "Unknown";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
