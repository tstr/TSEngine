/*
	Texture management source
*/

#include <tsgraphics/rendermodule.h>
#include <tsgraphics/texturemanager.h>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/filesystem/pathhelpers.h>

//todo: replace gdi+ with a custom texture loader
#include <Windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <codecvt>
#include <string>

using namespace ts;
using namespace std;

static std::string PrintPixelFormat(Gdiplus::PixelFormat format);
static std::string PrintGDIstatus(Gdiplus::Status status);
static bool LoadTGAFile(const char* filename, STextureResourceDescriptor& desc, vector<uint32>& databuffer);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Texture classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTexture2D::CTexture2D(CTextureManager* manager, const STextureResourceData& data, const STextureResourceDescriptor& desc) :
	m_manager(manager)
{
	STextureViewDescriptor vdesc;
	vdesc.arrayCount = 1;
	vdesc.arrayIndex = 0;
	ERenderStatus rstatus = eOk;

	IRenderApi* api = m_manager->getModule()->getApi();

	rstatus = api->createResourceTexture(m_texRsc, &data, desc);

	if (rstatus)
	{
		tswarn("Unable to create texture resource.");
		return;
	}

	rstatus = api->createViewTexture2D(m_texView, m_texRsc, vdesc);

	if (rstatus)
	{
		tswarn("Unable to create texture view.");
		return;
	}

	m_width = desc.width;
	m_height = desc.height;
}

CTextureCube::CTextureCube(CTextureManager* manager, const STextureResourceData* data, const STextureResourceDescriptor& desc) :
	m_manager(manager)
{
	STextureViewDescriptor vdesc;
	vdesc.arrayCount = 1;
	vdesc.arrayIndex = 0;
	ERenderStatus rstatus = eOk;

	IRenderApi* api = m_manager->getModule()->getApi();

	rstatus = api->createResourceTexture(m_texCubeRsc, data, desc);

	if (rstatus)
	{
		tswarn("Unable to create texture resource.");
		return;
	}

	rstatus = api->createViewTextureCube(m_texCubeView, m_texCubeRsc, vdesc);

	if (rstatus)
	{
		tswarn("Unable to create texture cube view.");
		return;
	}

	for (int i = 0; i < 6; i++)
	{
		STextureViewDescriptor facedesc;
		vdesc.arrayCount = 1;
		vdesc.arrayIndex = i;
		rstatus = api->createViewTexture2D(m_texCubeFaceViews[i], m_texCubeRsc, facedesc);

		if (rstatus)
		{
			tswarn("Unable to create texture cube face(%) view.", i);
			return;
		}
	}

	m_facewidth = desc.width;
	m_faceheight = desc.height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Texture manager class
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTextureManager::CTextureManager(CRenderModule* module, const Path& rootpath) :
	m_renderModule(module),
	m_rootpath(rootpath)
{
	tsassert(module);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	tsassert(!Gdiplus::GdiplusStartup(&m_token, &gdiplusStartupInput, 0));
}


CTextureManager::~CTextureManager()
{
	Gdiplus::GdiplusShutdown(m_token);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Loaders
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CTextureManager::loadTexture2D(const Path& file, CTexture2D& texture)
{
	using namespace Gdiplus;

	IRenderApi* api = m_renderModule->getApi();

	Path filepath = m_rootpath;

	if (isAbsolutePath(file))
	{
		filepath = file;
	}
	else
	{
		filepath.addDirectories(file);
	}

	tsinfo("Loading texture: \"%\"...", filepath.str());

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(filepath.str()));

	/////////////////////////////////////////////////////////////////////////

	STextureResourceData data;
	STextureResourceDescriptor desc;
	STextureViewDescriptor vdesc;
	vector<uint32> buffer;

	ResourceProxy tex;
	ResourceProxy texview;


	string extension;
	string filename(filepath.str());

	auto pos = filename.find_last_of('.');
	if (pos == string::npos)
		extension = filename;
	else
		extension = filename.substr(pos, filename.size() - pos);

	if (compare_string_weak(extension, ".tga"))
	{

		if (!LoadTGAFile(filename.c_str(), desc, buffer))
		{
			tswarn("Texture failed to load.");
			return false;
		}

		data.memory = (const void*)&buffer[0];
		data.memoryByteWidth = (uint32)(desc.width * sizeof(uint32));
		data.memoryByteDepth = 0;
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

		desc.texformat = texformat;
		desc.textype = ETextureResourceType::eTypeTexture2D;
		desc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		desc.height = bmpData.Height;
		desc.width = bmpData.Width;
		desc.useMips = true;
		desc.arraySize = 1;

		buffer.resize(bmpData.Width * bmpData.Height);
		memcpy(&buffer[0], bmpData.Scan0, bmpData.Stride * bmpData.Height);

		data.memory = (const void*)&buffer[0];
		data.memoryByteWidth = bmpData.Stride;
		data.memoryByteDepth = 0;

		s = bmp->UnlockBits(&bmpData);
		delete bmp;

		if (s)
		{
			tserror("Texture failed to load.");
			return false;
		}
	}

	texture = CTexture2D(this, data, desc);

	tsinfo("Texture loaded.");

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CTextureManager::loadTextureCube(const Path& file, CTextureCube& texture)
{
	using namespace Gdiplus;

	IRenderApi* api = m_renderModule->getApi();

	Path filepath = m_rootpath;

	if (isAbsolutePath(file))
	{
		filepath = file;
	}
	else
	{
		filepath.addDirectories(file);
	}

	tsinfo("Loading texture cube: \"%\"...", filepath.str());

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wfilename(converter.from_bytes(filepath.str()));
	
	/////////////////////////////////////////////////////////////////////////
	
	STextureResourceData data[6];
	STextureResourceDescriptor desc;
	//vector<uint32> buffers[6];
	MemoryBuffer buffers[6];
	
	string extension;
	string filename(filepath.str());

	auto pos = filename.find_last_of('.');
	if (pos == string::npos)
		extension = filename;
	else
		extension = filename.substr(pos, filename.size() - pos);

	{
		/////////////////////////////////////////////////////////////////////////
		using namespace Gdiplus;

		//Load bitmap from file
		Bitmap* bmp = Bitmap::FromFile(wfilename.c_str());

		if (bmp->GetLastStatus())
		{
			tswarn("Texture cube failed to load, GDI+ error: %", PrintGDIstatus(bmp->GetLastStatus()));
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
		
		auto res = (uint32)bmp->GetHeight() / 3;
		
		desc.texformat = format;
		desc.textype = ETextureResourceType::eTypeTextureCube;
		desc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		desc.height = res;
		desc.width = res;
		desc.useMips = false;
		desc.arraySize = 1;
		
		for (int i = 0; i < 6; i++)
		{
			Gdiplus::Rect dim(
				(int)(res * offsetsX[i]),
				(int)(res * offsetsY[i]),
				(int)res,
				(int)res
			);

			Bitmap* subBmp = bmp->Clone(dim, gdi_format);
			
			BitmapData subBmpData;
			Gdiplus::Status s = subBmp->LockBits(nullptr, ImageLockModeRead, gdi_format, &subBmpData);

			if (s)
			{
				tswarn("Texture cube failed to lock bits, GDI+ error: %", PrintGDIstatus(s));
				return false;
			}

			//buffers[i].resize(subbmpData.Width * subbmpData.Height);
			//memcpy((void*)&((buffers[i])[0]), subbmpData.Scan0, subbmpData.Stride * subbmpData.Height);
			buffers[i] = MemoryBuffer(subBmpData.Scan0, subBmpData.Stride * subBmpData.Height);

			//data[i].memory = (const void*)&(buffers[i])[0];
			data[i].memory = buffers[i].pointer();
			data[i].memoryByteWidth = subBmpData.Stride;
			data[i].memoryByteDepth = 0;

			s = subBmp->UnlockBits(&subBmpData);

			if (s)
			{
				tserror("Texture cube failed to load.");
				return false;
			}

			delete subBmp;
		}

		//Delete the bitmap image
		delete bmp;
	}

	texture = CTextureCube(this, data, desc);

	tsinfo("Texture cube loaded.");
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TGA loader
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//GDI+ doesn't support TGA files so we have to use a separate loading mechanism
static bool LoadTGAFile(const char* filename, STextureResourceDescriptor& desc, vector<uint32>& databuffer)
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