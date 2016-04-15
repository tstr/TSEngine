/*
	Graphics texture source
*/

#include "pch.h"

#include <C3Ext\win32.h>
#include <C3E\gfx\graphics.h>
#include <C3E\gfx\abi\graphicsabi.h>

#include <C3E\filesystem\filexml.h>

#include <codecvt>
#include <string>

#include <gdiplus.h>
LINK_LIB("gdiplus.lib")

using namespace std;
using namespace C3E;

union BGRAtoRGBA
{
	struct BGRA
	{
		C3E::byte b;
		C3E::byte g;
		C3E::byte r;
		C3E::byte a;
	};

	struct RGBA
	{
		C3E::byte r;
		C3E::byte g;
		C3E::byte b;
		C3E::byte a;
	};

	BGRA bgra;

	uint32 colour;

	uint32 rgba() const
	{
		RGBA c;
		c.r = bgra.r;
		c.b = bgra.b;
		c.g = bgra.g;
		c.a = bgra.a;

		return reinterpret_cast<uint32&>(c);
	}

	BGRAtoRGBA(uint32 c) : colour(c) {}
};

static const class GDI
{
private:

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

public:

	GDI()
	{
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}

	~GDI()
	{
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
} g_gdi;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TGA loader
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//GDI+ doesn't support TGA files so we have to use a separate loading mechanism
static bool LoadTGAFile(const char* filename, ABI::ShaderResourceDescriptor& desc, vector<uint32>& databuffer)
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
	ZeroMemory(&tgaFile, sizeof(TGAFILE));

	FILE *filePtr = nullptr;
	unsigned char ucharBad;
	short int sintBad;
	long imageSize;
	int colorMode;
	//unsigned char colorSwap;
	
	// Open the TGA file.
	fopen_s(&filePtr,filename, "rb");
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
		using C3E::byte;

		byte bgra[4];

		bgra[0] = tgaFile.imageData[i + 0];
		bgra[1] = tgaFile.imageData[i + 1];
		bgra[2] = tgaFile.imageData[i + 2];
		bgra[3] = (colorMode == 4) ? tgaFile.imageData[i + 3] : (byte)255;

		//volatile auto x = *reinterpret_cast<int*>(&);
		volatile auto x = (int)tgaFile.imageData[i + 0];
		//cout << 128 * *reinterpret_cast<float*>(&bgra) << endl;

		databuffer[i / colorMode] = *reinterpret_cast<uint32*>(&bgra);
	}

	fclose(filePtr);

	desc.rscusage = EResourceUsage::UsageDynamic;
	desc.texformat = ETextureFormat::FormatBGRA8_INT;
	desc.textype = ETextureType::TypeTexture2D;
	desc.height = tgaFile.imageHeight;
	desc.width = tgaFile.imageWidth;
	desc.useMips = true;
	desc.arraySize = 1;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void PrintPixelFormat(Gdiplus::PixelFormat format);
static void PrintGDIstatus(Gdiplus::Status status);

Texture::Texture(Graphics* renderer, const char* file) :
	EffectResource(renderer)
{
	auto api = (ABI::IRenderApi*)m_renderer->api();

	if (!file || !FileSystem::FileExists(file))
	{
		m_success = false;
		cerr << "Texture file does not exist: " << file << "\n";
		return;
	}

	cout << "Loading texture: '" << file << "'...\n";

	strcpy_s(m_filename, file);

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(file));

	/////////////////////////////////////////////////////////////////////////

	string extension;
	string filename(file);

	auto pos = filename.find_last_of('.');
	if (pos == string::npos)
		extension = filename;
	else
		extension = filename.substr(pos, filename.size() - pos);

	if (compare_string_weak(extension, ".tga"))
	{
		ABI::ShaderResourceDescriptor desc;
		ABI::ShaderResourceData data;

		vector<uint32> buffer;

		if (m_success = LoadTGAFile(file, desc, buffer))
		{
			data.mem = (const void*)&buffer[0];
			data.memPitch = (uint32)(desc.width * sizeof(uint32));
			data.memSlicePitch = 0;

			m_rsc = api->CreateShaderResource(&data, desc);


			if (m_rsc)
			{
				api->SetResourceString(m_rsc, ((string)"ShaderTexture-" + m_filename).c_str());
				return;
			}
			else
			{
				cerr << "Unable to create texture resource.\n";
				return;
			}
		}

		cerr << "Texture failed to load.\n";
		return;
	}

	/////////////////////////////////////////////////////////////////////////

	using namespace Gdiplus;

	//Load bitmap from file
	Bitmap* bmp = Bitmap::FromFile(wfilename.c_str());
	BitmapData bmpData;

	if (bmp->GetLastStatus())
	{
		m_success = false;
		cerr << "Texture failed to load, GDI+ error: ";
		PrintGDIstatus(bmp->GetLastStatus());
		cerr << endl;
		return;
	}

	ETextureFormat format = FormatUnknown;
	PixelFormat gdi_format = PixelFormatDontCare;

	switch (bmp->GetPixelFormat())
	{
	case PixelFormat8bppIndexed:
	{
		format = ETextureFormat::FormatByte;
		gdi_format = PixelFormat8bppIndexed;
		break;
	}
	case PixelFormat24bppRGB:
	{
		//Graphics API doesn't support texture formats with bytewidths not a power of 2 so we need to force extra colour channel for this texture
		format = ETextureFormat::FormatBGRA8_INT;
		gdi_format = PixelFormat32bppARGB; 
		break;
	}
	case PixelFormat32bppARGB:
	default:
	{
		format = ETextureFormat::FormatBGRA8_INT;
		gdi_format = PixelFormat32bppARGB;
	}
	}

	cout << "GDI+ texture format: ";
	PrintPixelFormat(bmp->GetPixelFormat());
	cout << endl;

	/////////////////////////////////////////////////////////////////////////

	uint32 sz = Gdiplus::GetPixelFormatSize(bmp->GetPixelFormat());
	
	Gdiplus::Rect dim(0, 0, bmp->GetWidth(), bmp->GetHeight());
	Gdiplus::Status s = bmp->LockBits(&dim, ImageLockModeRead, gdi_format, &bmpData);
	
	if (s)
	{
		m_success = false;
		cerr << "Texture failed to lock bits, GDI+ error: ";
		PrintGDIstatus(s);
		cerr << endl;
		return;
	}

	ETextureFormat texformat = format;

	ABI::ShaderResourceDescriptor desc;
	ABI::ShaderResourceData data;

	desc.rscusage = EResourceUsage::UsageDynamic;
	desc.texformat = texformat;
	desc.textype = ETextureType::TypeTexture2D;
	desc.height = bmpData.Height;
	desc.width = bmpData.Width;
	desc.useMips = true;
	desc.arraySize = 1;

	data.memPitch = bmpData.Stride;
	data.mem = bmpData.Scan0;
	data.memSlicePitch = 0;

	m_rsc = api->CreateShaderResource(&data, desc);

	api->SetResourceString(m_rsc, ((string)"ShaderTexture-" + m_filename).c_str());

	s = bmp->UnlockBits(&bmpData);
	delete bmp;

	if (s)
	{
		m_success = false;
		return;
	}

	if (m_rsc)
	{
		m_success = true;
		cout << "Texture loaded.\n";
		return;
	}
	
	cerr << "Texture failed to load.\n";

	m_success = false;
}

Texture::Texture(Graphics* renderer, ResourceHandle h) :EffectResource(renderer)
{
	m_rsc = h;
}

Texture::~Texture()
{
	auto api = (ABI::IRenderApi*)m_renderer->api();

	api->DestroyShaderResource(m_rsc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureCube::TextureCube(Graphics* renderer, const char* file) :
	EffectResource(renderer)
{
	auto api = (ABI::IRenderApi*)m_renderer->api();

	if (!file || !FileSystem::FileExists(file))
	{
		m_success = false;
		return;
	}

	cout << "Loading cube map: '" << file << "'...\n";

	strcpy_s(m_filename, file);

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(file));

	/////////////////////////////////////////////////////////////////////////

	using namespace Gdiplus;

	//Load bitmap from file
	Bitmap* bmp = Bitmap::FromFile(wfilename.c_str());

	if (bmp->GetLastStatus())
	{
		m_success = false;
		cerr << "Texture failed to load, GDI+ error: ";
		PrintGDIstatus(bmp->GetLastStatus());
		cerr << endl;
		return;
	}

	ETextureFormat format = FormatUnknown;
	PixelFormat gdi_format = PixelFormatDontCare;

	switch (bmp->GetPixelFormat())
	{
	case PixelFormat8bppIndexed:
	{
		format = ETextureFormat::FormatByte;
		gdi_format = PixelFormat8bppIndexed;
		break;
	}
	case PixelFormat24bppRGB:
	{
		//Graphics API doesn't support texture formats with bytewidths not a power of 2 so we need to force extra colour channel for this texture
		format = ETextureFormat::FormatBGRA8_INT;
		gdi_format = PixelFormat32bppARGB;
		break;
	}
	case PixelFormat32bppARGB:
	default:
	{
		format = ETextureFormat::FormatBGRA8_INT;
		gdi_format = PixelFormat32bppARGB;
	}
	}

	cout << "GDI+ texture format: ";
	PrintPixelFormat(gdi_format);
	cout << endl;

	/////////////////////////////////////////////////////////////////////////

	ETextureFormat texformat = format;

	ABI::ShaderResourceDescriptor desc;
	ABI::ShaderResourceData data[6];

	float offsetsX[] =
	{
		2,
		0,
		1,
		1,
		1,
		3,
	};

	float offsetsY[] =
	{
		1,
		1,
		0,
		2,
		1,
		1,
	};

	float res = (float)bmp->GetHeight() / 3;

	desc.rscusage = EResourceUsage::UsageDynamic;
	desc.texformat = texformat;
	desc.textype = ETextureType::TypeTexture2D;
	desc.height = (uint32)res;
	desc.width = (uint32)res;
	desc.useMips = false;
	desc.arraySize = 6;
	desc.isCubemap = true;

	MemoryBuffer m_buffers[6];

	for (uint32 i = 0; i < 6; i++)
	{
		Gdiplus::Rect dim(
			(int)(res * offsetsX[i]),
			(int)(res * offsetsY[i]),
			(int)res,
			(int)res
		);

		BitmapData subbmpData;

		Gdiplus::Status s = bmp->LockBits(&dim, ImageLockModeRead, gdi_format, &subbmpData);
		if (s)
		{
			m_success = false;
			cerr << "Cube face failed to lock, GDI+ error: ";
			PrintGDIstatus(s);
			cerr << endl;
			cerr << "X = " << offsetsX[i] << " Y = " << offsetsY[i] << endl;
			return;
		}

		m_buffers[i] = move(MemoryBuffer(subbmpData.Scan0, subbmpData.Height * subbmpData.Stride));

		data[i].memPitch = subbmpData.Stride;
		data[i].mem = m_buffers[i].pointer();
		data[i].memSlicePitch = 0;

		s = bmp->UnlockBits(&subbmpData);
		if (s)
		{
			m_success = false;
			cerr << "Cube face failed to unlock, GDI+ error: ";
			PrintGDIstatus(s);
			cerr << endl;
			cerr << "X = " << offsetsX[i] << " Y = " << offsetsY[i] << endl;
			return;
		}
	}

	m_rsc = api->CreateShaderResource(data, desc);

	api->SetResourceString(m_rsc, ((string)"ShaderTexture-" + m_filename).c_str());

	delete bmp;

	cout << "Texture loaded.\n";

	if (m_rsc)
	{
		m_success = true;
		return;
	}

	m_success = false;
}

TextureCube::~TextureCube()
{
	auto api = (ABI::IRenderApi*)m_renderer->api();

	api->DestroyShaderResource(m_rsc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ppf(x) case x: { cout << #x; break; }

void PrintGDIstatus(Gdiplus::Status status)
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
}

void PrintPixelFormat(Gdiplus::PixelFormat format)
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
}
