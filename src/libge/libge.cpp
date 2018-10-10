// libge.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "libge.h"
#include "curl/curl.h"
#include "zlib.h"
#include <io.h>
#include <direct.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string.h>
#include <fstream>
#include "qtutils.h"
#include "CacheManager.h"
#include "gdal_priv.h"
#include "gdal_vrt.h"
#include "gdal_proxy.h"

LIBGE_NAMESPACE_BEGINE
char GOOGLE_EARTH_CRYPT_KEY[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0xF4, 0xBD, 0x0B, 0x79, 0xE2, 0x6A, 0x45,
	0x22, 0x05, 0x92, 0x2C, 0x17, 0xCD, 0x06, 0x71, 0xF8, 0x49, 0x10, 0x46, 0x67, 0x51, 0x00, 0x42,
	0x25, 0xC6, 0xE8, 0x61, 0x2C, 0x66, 0x29, 0x08, 0xC6, 0x34, 0xDC, 0x6A, 0x62, 0x25, 0x79, 0x0A,
	0x77, 0x1D, 0x6D, 0x69, 0xD6, 0xF0, 0x9C, 0x6B, 0x93, 0xA1, 0xBD, 0x4E, 0x75, 0xE0, 0x41, 0x04,
	0x5B, 0xDF, 0x40, 0x56, 0x0C, 0xD9, 0xBB, 0x72, 0x9B, 0x81, 0x7C, 0x10, 0x33, 0x53, 0xEE, 0x4F,
	0x6C, 0xD4, 0x71, 0x05, 0xB0, 0x7B, 0xC0, 0x7F, 0x45, 0x03, 0x56, 0x5A, 0xAD, 0x77, 0x55, 0x65,
	0x0B, 0x33, 0x92, 0x2A, 0xAC, 0x19, 0x6C, 0x35, 0x14, 0xC5, 0x1D, 0x30, 0x73, 0xF8, 0x33, 0x3E,
	0x6D, 0x46, 0x38, 0x4A, 0xB4, 0xDD, 0xF0, 0x2E, 0xDD, 0x17, 0x75, 0x16, 0xDA, 0x8C, 0x44, 0x74,
	0x22, 0x06, 0xFA, 0x61, 0x22, 0x0C, 0x33, 0x22, 0x53, 0x6F, 0xAF, 0x39, 0x44, 0x0B, 0x8C, 0x0E,
	0x39, 0xD9, 0x39, 0x13, 0x4C, 0xB9, 0xBF, 0x7F, 0xAB, 0x5C, 0x8C, 0x50, 0x5F, 0x9F, 0x22, 0x75,
	0x78, 0x1F, 0xE9, 0x07, 0x71, 0x91, 0x68, 0x3B, 0xC1, 0xC4, 0x9B, 0x7F, 0xF0, 0x3C, 0x56, 0x71,
	0x48, 0x82, 0x05, 0x27, 0x55, 0x66, 0x59, 0x4E, 0x65, 0x1D, 0x98, 0x75, 0xA3, 0x61, 0x46, 0x7D,
	0x61, 0x3F, 0x15, 0x41, 0x00, 0x9F, 0x14, 0x06, 0xD7, 0xB4, 0x34, 0x4D, 0xCE, 0x13, 0x87, 0x46,
	0xB0, 0x1A, 0xD5, 0x05, 0x1C, 0xB8, 0x8A, 0x27, 0x7B, 0x8B, 0xDC, 0x2B, 0xBB, 0x4D, 0x67, 0x30,
	0xC8, 0xD1, 0xF6, 0x5C, 0x8F, 0x50, 0xFA, 0x5B, 0x2F, 0x46, 0x9B, 0x6E, 0x35, 0x18, 0x2F, 0x27,
	0x43, 0x2E, 0xEB, 0x0A, 0x0C, 0x5E, 0x10, 0x05, 0x10, 0xA5, 0x73, 0x1B, 0x65, 0x34, 0xE5, 0x6C,
	0x2E, 0x6A, 0x43, 0x27, 0x63, 0x14, 0x23, 0x55, 0xA9, 0x3F, 0x71, 0x7B, 0x67, 0x43, 0x7D, 0x3A,
	0xAF, 0xCD, 0xE2, 0x54, 0x55, 0x9C, 0xFD, 0x4B, 0xC6, 0xE2, 0x9F, 0x2F, 0x28, 0xED, 0xCB, 0x5C,
	0xC6, 0x2D, 0x66, 0x07, 0x88, 0xA7, 0x3B, 0x2F, 0x18, 0x2A, 0x22, 0x4E, 0x0E, 0xB0, 0x6B, 0x2E,
	0xDD, 0x0D, 0x95, 0x7D, 0x7D, 0x47, 0xBA, 0x43, 0xB2, 0x11, 0xB2, 0x2B, 0x3E, 0x4D, 0xAA, 0x3E,
	0x7D, 0xE6, 0xCE, 0x49, 0x89, 0xC6, 0xE6, 0x78, 0x0C, 0x61, 0x31, 0x05, 0x2D, 0x01, 0xA4, 0x4F,
	0xA5, 0x7E, 0x71, 0x20, 0x88, 0xEC, 0x0D, 0x31, 0xE8, 0x4E, 0x0B, 0x00, 0x6E, 0x50, 0x68, 0x7D,
	0x17, 0x3D, 0x08, 0x0D, 0x17, 0x95, 0xA6, 0x6E, 0xA3, 0x68, 0x97, 0x24, 0x5B, 0x6B, 0xF3, 0x17,
	0x23, 0xF3, 0xB6, 0x73, 0xB3, 0x0D, 0x0B, 0x40, 0xC0, 0x9F, 0xD8, 0x04, 0x51, 0x5D, 0xFA, 0x1A,
	0x17, 0x22, 0x2E, 0x15, 0x6A, 0xDF, 0x49, 0x00, 0xB9, 0xA0, 0x77, 0x55, 0xC6, 0xEF, 0x10, 0x6A,
	0xBF, 0x7B, 0x47, 0x4C, 0x7F, 0x83, 0x17, 0x05, 0xEE, 0xDC, 0xDC, 0x46, 0x85, 0xA9, 0xAD, 0x53,
	0x07, 0x2B, 0x53, 0x34, 0x06, 0x07, 0xFF, 0x14, 0x94, 0x59, 0x19, 0x02, 0xE4, 0x38, 0xE8, 0x31,
	0x83, 0x4E, 0xB9, 0x58, 0x46, 0x6B, 0xCB, 0x2D, 0x23, 0x86, 0x92, 0x70, 0x00, 0x35, 0x88, 0x22,
	0xCF, 0x31, 0xB2, 0x26, 0x2F, 0xE7, 0xC3, 0x75, 0x2D, 0x36, 0x2C, 0x72, 0x74, 0xB0, 0x23, 0x47,
	0xB7, 0xD3, 0xD1, 0x26, 0x16, 0x85, 0x37, 0x72, 0xE2, 0x00, 0x8C, 0x44, 0xCF, 0x10, 0xDA, 0x33,
	0x2D, 0x1A, 0xDE, 0x60, 0x86, 0x69, 0x23, 0x69, 0x2A, 0x7C, 0xCD, 0x4B, 0x51, 0x0D, 0x95, 0x54,
	0x39, 0x77, 0x2E, 0x29, 0xEA, 0x1B, 0xA6, 0x50, 0xA2, 0x6A, 0x8F, 0x6F, 0x50, 0x99, 0x5C, 0x3E,
	0x54, 0xFB, 0xEF, 0x50, 0x5B, 0x0B, 0x07, 0x45, 0x17, 0x89, 0x6D, 0x28, 0x13, 0x77, 0x37, 0x1D,
	0xDB, 0x8E, 0x1E, 0x4A, 0x05, 0x66, 0x4A, 0x6F, 0x99, 0x20, 0xE5, 0x70, 0xE2, 0xB9, 0x71, 0x7E,
	0x0C, 0x6D, 0x49, 0x04, 0x2D, 0x7A, 0xFE, 0x72, 0xC7, 0xF2, 0x59, 0x30, 0x8F, 0xBB, 0x02, 0x5D,
	0x73, 0xE5, 0xC9, 0x20, 0xEA, 0x78, 0xEC, 0x20, 0x90, 0xF0, 0x8A, 0x7F, 0x42, 0x17, 0x7C, 0x47,
	0x19, 0x60, 0xB0, 0x16, 0xBD, 0x26, 0xB7, 0x71, 0xB6, 0xC7, 0x9F, 0x0E, 0xD1, 0x33, 0x82, 0x3D,
	0xD3, 0xAB, 0xEE, 0x63, 0x99, 0xC8, 0x2B, 0x53, 0xA0, 0x44, 0x5C, 0x71, 0x01, 0xC6, 0xCC, 0x44,
	0x1F, 0x32, 0x4F, 0x3C, 0xCA, 0xC0, 0x29, 0x3D, 0x52, 0xD3, 0x61, 0x19, 0x58, 0xA9, 0x7D, 0x65,
	0xB4, 0xDC, 0xCF, 0x0D, 0xF4, 0x3D, 0xF1, 0x08, 0xA9, 0x42, 0xDA, 0x23, 0x09, 0xD8, 0xBF, 0x5E,
	0x50, 0x49, 0xF8, 0x4D, 0xC0, 0xCB, 0x47, 0x4C, 0x1C, 0x4F, 0xF7, 0x7B, 0x2B, 0xD8, 0x16, 0x18,
	0xC5, 0x31, 0x92, 0x3B, 0xB5, 0x6F, 0xDC, 0x6C, 0x0D, 0x92, 0x88, 0x16, 0xD1, 0x9E, 0xDB, 0x3F,
	0xE2, 0xE9, 0xDA, 0x5F, 0xD4, 0x84, 0xE2, 0x46, 0x61, 0x5A, 0xDE, 0x1C, 0x55, 0xCF, 0xA4, 0x00,
	0xBE, 0xFD, 0xCE, 0x67, 0xF1, 0x4A, 0x69, 0x1C, 0x97, 0xE6, 0x20, 0x48, 0xD8, 0x5D, 0x7F, 0x7E,
	0xAE, 0x71, 0x20, 0x0E, 0x4E, 0xAE, 0xC0, 0x56, 0xA9, 0x91, 0x01, 0x3C, 0x82, 0x1D, 0x0F, 0x72,
	0xE7, 0x76, 0xEC, 0x29, 0x49, 0xD6, 0x5D, 0x2D, 0x83, 0xE3, 0xDB, 0x36, 0x06, 0xA9, 0x3B, 0x66,
	0x13, 0x97, 0x87, 0x6A, 0xD5, 0xB6, 0x3D, 0x50, 0x5E, 0x52, 0xB9, 0x4B, 0xC7, 0x73, 0x57, 0x78,
	0xC9, 0xF4, 0x2E, 0x59, 0x07, 0x95, 0x93, 0x6F, 0xD0, 0x4B, 0x17, 0x57, 0x19, 0x3E, 0x27, 0x27,
	0xC7, 0x60, 0xDB, 0x3B, 0xED, 0x9A, 0x0E, 0x53, 0x44, 0x16, 0x3E, 0x3F, 0x8D, 0x92, 0x6D, 0x77,
	0xA2, 0x0A, 0xEB, 0x3F, 0x52, 0xA8, 0xC6, 0x55, 0x5E, 0x31, 0x49, 0x37, 0x85, 0xF4, 0xC5, 0x1F,
	0x26, 0x2D, 0xA9, 0x1C, 0xBF, 0x8B, 0x27, 0x54, 0xDA, 0xC3, 0x6A, 0x20, 0xE5, 0x2A, 0x78, 0x04,
	0xB0, 0xD6, 0x90, 0x70, 0x72, 0xAA, 0x8B, 0x68, 0xBD, 0x88, 0xF7, 0x02, 0x5F, 0x48, 0xB1, 0x7E,
	0xC0, 0x58, 0x4C, 0x3F, 0x66, 0x1A, 0xF9, 0x3E, 0xE1, 0x65, 0xC0, 0x70, 0xA7, 0xCF, 0x38, 0x69,
	0xAF, 0xF0, 0x56, 0x6C, 0x64, 0x49, 0x9C, 0x27, 0xAD, 0x78, 0x74, 0x4F, 0xC2, 0x87, 0xDE, 0x56,
	0x39, 0x00, 0xDA, 0x77, 0x0B, 0xCB, 0x2D, 0x1B, 0x89, 0xFB, 0x35, 0x4F, 0x02, 0xF5, 0x08, 0x51,
	0x13, 0x60, 0xC1, 0x0A, 0x5A, 0x47, 0x4D, 0x26, 0x1C, 0x33, 0x30, 0x78, 0xDA, 0xC0, 0x9C, 0x46,
	0x47, 0xE2, 0x5B, 0x79, 0x60, 0x49, 0x6E, 0x37, 0x67, 0x53, 0x0A, 0x3E, 0xE9, 0xEC, 0x46, 0x39,
	0xB2, 0xF1, 0x34, 0x0D, 0xC6, 0x84, 0x53, 0x75, 0x6E, 0xE1, 0x0C, 0x59, 0xD9, 0x1E, 0xDE, 0x29,
	0x85, 0x10, 0x7B, 0x49, 0x49, 0xA5, 0x77, 0x79, 0xBE, 0x49, 0x56, 0x2E, 0x36, 0xE7, 0x0B, 0x3A,
	0xBB, 0x4F, 0x03, 0x62, 0x7B, 0xD2, 0x4D, 0x31, 0x95, 0x2F, 0xBD, 0x38, 0x7B, 0xA8, 0x4F, 0x21,
	0xE1, 0xEC, 0x46, 0x70, 0x76, 0x95, 0x7D, 0x29, 0x22, 0x78, 0x88, 0x0A, 0x90, 0xDD, 0x9D, 0x5C,
	0xDA, 0xDE, 0x19, 0x51, 0xCF, 0xF0, 0xFC, 0x59, 0x52, 0x65, 0x7C, 0x33, 0x13, 0xDF, 0xF3, 0x48,
	0xDA, 0xBB, 0x2A, 0x75, 0xDB, 0x60, 0xB2, 0x02, 0x15, 0xD4, 0xFC, 0x19, 0xED, 0x1B, 0xEC, 0x7F,
	0x35, 0xA8, 0xFF, 0x28, 0x31, 0x07, 0x2D, 0x12, 0xC8, 0xDC, 0x88, 0x46, 0x7C, 0x8A, 0x5B, 0x22 
};

char GOOGLE_EARTH_GEAUTH1[] = { 
	0x03, 0x00, 0x00, 0x00, 0x02, 0xf1, 0x5b, 0x5e, 0x34, 0x86, 0x84, 0x38, 0x4f, 0xb9, 0x04, 0x0a,
    0x3a, 0xbf, 0x5e, 0x6a, 0x8d, 0x85, 0x3c, 0x6a, 0x3f, 0xaa, 0xd0, 0xf1, 0x77, 0x47, 0x6f, 0x6f, 
	0x67, 0x6c, 0x65, 0x45, 0x61, 0x72, 0x74, 0x68, 0x57, 0x69, 0x6e, 0x2e, 0x65, 0x78, 0x65, 0x00 };
char GOOGLE_EARTH_GEAUTH2[] = {
	0x01, 0x00, 0x00, 0x00, 0x02, 0xf1, 0x5b, 0x5e, 0x34, 0x86, 0x84, 0x38, 0x4f, 0xb9, 0x04, 0x0a,
	0x3a, 0xbf, 0x5e, 0x6a, 0x8d, 0xec, 0xc2, 0xa8, 0x1c, 0x43, 0x08, 0xc5, 0x77, 0x58, 0xe0, 0x48,
	0x9d, 0x8b, 0x80, 0xdb, 0x4d, 0x00, 0x06, 0x25, 0x31, 0x93, 0xaf, 0x8e, 0xf6, 0xfb, 0x0a, 0xa9,
	0x8b };
char GOOGLE_EARTH_GEAUTH3[] = {
	0x01, 0x00, 0x00, 0x00, 0x02, 0x72, 0xb7, 0x97, 0x7b, 0xae, 0x42, 0x3e, 0x43, 0x8b, 0x26, 0x19,
	0xca, 0xae, 0x24, 0x5b, 0x9f, 0x03, 0x29, 0xf2, 0xa6, 0xc4, 0x0e, 0x8d, 0x22, 0x5c, 0xd6, 0xf1,
	0x71, 0x12, 0x7c, 0xe0, 0xc7, 0x00, 0x06, 0x25, 0x31, 0x83, 0x5e, 0x79, 0x5c, 0xdc, 0x37, 0x19,
	0xc8 };

char GOOGLE_EARTH_GEAUTH_KEY[] = { 0x00, 0x00, 0x02, 0x58, 0x00, 0x00, 0x00 };

const unsigned long CRYPTED_JPEG_MAGIC = 0xA6EF9107;
const unsigned long DECRYPTED_JPEG_MAGIC = 0xE0FFD8FF;
const unsigned long CRYPTED_DXT1_MAGIC = 0x77B3CBB7;
const unsigned long DECRYPTED_DXT1_MAGIC = 0x31A3824F;
const unsigned short CRYPTED_MODEL_LAYER_MAGIC = 0x4832;
const unsigned short DECRYPTED_MODEL_LAYER_MAGIC = 0x01CA;
const unsigned short CRYPTED_MODEL_DATA_MAGIC = 0x487B;
const unsigned short DECRYPTED_MODEL_DATA_MAGIC = 0x0183;
const unsigned long CRYPTED_ZLIB_MAGIC = 0x32789755;
const unsigned long DECRYPTED_ZLIB_MAGIC = 0x7468DEAD;

const unsigned short BOF_JPEG = 0xD8FF;
const unsigned short EOF_JPEG = 0xD9FF;

const unsigned short BOF_MODEL = 0x0183;
const unsigned short EOF_MODEL = 0x0184;
//////////////////////////////////////////////////////////////////////////
//
void convertEndian(LPVOID lpSrc, int size, LPVOID lpDst, bool littleEndian = false)
{
	if (lpSrc == nullptr || lpDst == nullptr || size < 1)
		return;

	BYTE* pIn = (BYTE*)lpSrc;
	BYTE* pOut = (BYTE*)lpDst;
	memset(pOut, 0, size);
	if (!littleEndian)
	{
		for (int i = 0; i < size; i++)
			pOut[i] = pIn[size - 1 - i];
	}
	else
	{
		for (int i = 0; i < size; i++)
			pOut[i] = pIn[i];
	}
}

void replaceString(std::string& str, const std::string& spat, const std::string& rpat)
{
	std::string::size_type pos = 0;
	while ((pos = str.find(spat, pos)) != std::string::npos)
	{
		str.replace(pos, spat.length(), rpat);
		pos += rpat.length();
	}
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// CLibGEHelper
CLibGEHelper::CLibGEHelper()
{
	_version = 0U;

	char szPath[_MAX_PATH];
	GetModuleFileNameA(nullptr, szPath, _MAX_PATH);
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
#if _MSC_VER < 1600
	_splitpath(szPath, szDrive, szDir, NULL, NULL);
#else
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
#endif
	std::stringstream sstr;
	sstr << szDrive << szDir << "\\Cache\\";
	_mkdir(sstr.str().c_str());
	_cachePath = sstr.str();

	std::string cacheDBPath = _cachePath + "\\libge_cache.db";
	CacheManager::GetInstance().Open(cacheDBPath.c_str());

	memset(_crypt_key, 0, sizeof(_crypt_key));
	memcpy(_crypt_key, GOOGLE_EARTH_CRYPT_KEY, sizeof(_crypt_key));

	_serverURLs.push_back("kh.google.com");

	{
		std::string geAuth;
		geAuth.resize(sizeof(GOOGLE_EARTH_GEAUTH2));
		memcpy(geAuth._Myptr(), GOOGLE_EARTH_GEAUTH2, sizeof(GOOGLE_EARTH_GEAUTH2));
		_geAuths.push_back(geAuth);
	}
	{
		std::string geAuth;
		geAuth.resize(sizeof(GOOGLE_EARTH_GEAUTH3));
		memcpy(geAuth._Myptr(), GOOGLE_EARTH_GEAUTH3, sizeof(GOOGLE_EARTH_GEAUTH3));
		_geAuths.push_back(geAuth);
	}	
}

CLibGEHelper::~CLibGEHelper()
{
}

void CLibGEHelper::Initialize()
{
	char szPath[_MAX_PATH];
	GetModuleFileNameA(nullptr, szPath, _MAX_PATH);
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
#if _MSC_VER < 1600
	_splitpath(szPath, szDrive, szDir, NULL, NULL);
#else
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
#endif
	std::stringstream sstr;
	sstr << szDrive << szDir << "\\gdal-data\\";
	CPLSetConfigOption("GDAL_DATA", sstr.str().c_str());
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	CPLSetConfigOption("SHAPE_ENCODING", "");
	GDALAllRegister();
	curl_global_init(CURL_GLOBAL_ALL);
}

void CLibGEHelper::UnInitialize()
{
	curl_global_cleanup();

}

static int OnDebug(CURL *, curl_infotype itype, char * pData, std::size_t size, void *)
{
	return 0;
	if (itype == CURLINFO_TEXT)
	{
		printf("[TEXT]%s\n", pData);  
	}
	else if (itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static std::size_t OnWriteData(void* buffer, std::size_t size, std::size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

void decryptData(char* data, int len, const char* key)
{
	int i = 0;
	int j = 16;
	while (i < len)
	{
		data[i] ^= key[j + 8];
		i += 1;
		j += 1;

		if (j % 8 == 0)
			j += 16;
		if (j >= 1016)
		{
			j += 8;
			j %= 24;
		}
	}
}

std::string CLibGEHelper::UnPackGEZlib(char* srcData, unsigned long srcSize)
{
	std::string outData;	
	if (srcData == nullptr || srcSize <= 0)
		return outData;

	unsigned long magic;
	convertEndian((LPVOID)srcData, 4, &magic, true);
	if (magic == CRYPTED_JPEG_MAGIC
		|| magic == CRYPTED_DXT1_MAGIC
		|| magic == CRYPTED_ZLIB_MAGIC
		|| magic == CRYPTED_MODEL_LAYER_MAGIC
		|| magic == CRYPTED_MODEL_DATA_MAGIC)
	{
		decryptData(srcData, srcSize, _crypt_key);
		convertEndian((LPVOID)srcData, 4, &magic, true);
	}

	if (magic == DECRYPTED_ZLIB_MAGIC)
	{
		unsigned long uncompress_size;
		convertEndian((LPVOID)(srcData + 4), 4, &uncompress_size, true);
		outData.resize(uncompress_size);		
		memset(outData._Myptr(), 0, outData.size());
		unsigned long outLen;
		uncompress((BYTE*)(outData._Myptr()), &outLen, (const BYTE*)(srcData + 8), srcSize - 8);		
	}
	else
	{
		outData.resize(srcSize);
		memset(outData._Myptr(), 0, outData.size());		
		memcpy(outData._Myptr(), srcData, srcSize);
	}
	return outData;
}

bool CLibGEHelper::DecodeImage(const char* srcData, unsigned long srcSize, const char* filePath)
{
	if (srcData == nullptr || srcSize <= 0 || filePath==nullptr)
		return false;

	unsigned short bof, eof;
	convertEndian((LPVOID)srcData, 2, &bof, true);
	convertEndian((LPVOID)(srcData + srcSize - 2), 2, &eof, true);
	if (bof == BOF_JPEG && eof == EOF_JPEG)
	{
		FILE* fp = nullptr;
		fopen_s(&fp, filePath, "wb+");
		if (fp != nullptr)
		{
			fwrite(srcData, srcSize, 1, fp);
			fclose(fp);
			return true;
		}		
	}

	return false;
}

Terrain* CLibGEHelper::DecodeTerrain(const char* name, const char* srcData, unsigned long srcSize)
{	
	Terrain* terrain = new Terrain(name);
	if (!terrain->decode(srcData, srcSize))
	{
		delete terrain;
		return nullptr;
	}
	return terrain;
}

QuadTreePacket16* CLibGEHelper::DecodeQuadtree(const char* srcData, unsigned long srcSize)
{ 
	if (srcData == nullptr || srcSize <= 0)
		return nullptr;

	QuadTreePacket16* packet = new QuadTreePacket16();
	if (!packet->decode(srcData, srcSize))
	{
		delete packet;
		return nullptr;
	}
	return packet;
}

int CLibGEHelper::Get(const std::string & strUrl, std::string & strResponse, bool useSession)
{
	CURL* curl = curl_easy_init();
	if (NULL == curl)
		return CURLE_FAILED_INIT;

#ifdef _DEBUG
	if (curl != nullptr)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
#endif	

	//http请求头
	struct curl_slist *headers = NULL;
	//headers = curl_slist_append(headers, "User-Agent: GoogleEarth/7.1.1.1580(Windows; Microsoft Windows(6.1.7601.1);zh-Hans;kml:2.2;client:Free;type:default)");
	headers = curl_slist_append(headers, "Accept: text/plain, text/html, text/xml, text/xml-external-parsed-entity, application/octet-stream, application/vnd.google-earth.kml+xml, application/vnd.google-earth.kmz, image/*");
	headers = curl_slist_append(headers, "Cache-Control: no-store");
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
	headers = curl_slist_append(headers, "Connection: Keep-Alive");
	if (useSession && _SessionIds.size() > 0)
	{
		srand((unsigned)time(NULL));
		int index = rand() % _SessionIds.size();
		std::stringstream sstr;
		sstr << "Cookie: $Version=\"0\"; SessionId=\"" << _SessionIds.at(index) << "\"; State=\"1\"";
		headers = curl_slist_append(headers, sstr.str().c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	int res = curl_easy_perform(curl);
	if (res == CURLE_OK)
	{
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		if (responseCode!=200)
		{
			strResponse.clear();
			res = responseCode;
		}
	}
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	return res;
}

int CLibGEHelper::Post(const std::string & strUrl, const char* szPost, int postSize, std::string & strResponse, bool useSession)
{	
	CURL* curl = curl_easy_init();
	if (NULL == curl)
		return CURLE_FAILED_INIT;

#ifdef _DEBUG
	if (curl != nullptr)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
#endif	

	//http请求头
	struct curl_slist *headers = NULL;
	//headers = curl_slist_append(headers, "User-Agent: GoogleEarth/7.1.1.1580(Windows; Microsoft Windows(6.1.7601.1);zh-Hans;kml:2.2;client:Free;type:default)");
	headers = curl_slist_append(headers, "Accept: text/plain, text/html, text/xml, text/xml-external-parsed-entity, application/octet-stream, application/vnd.google-earth.kml+xml, application/vnd.google-earth.kmz, image/*");
	headers = curl_slist_append(headers, "Cache-Control: no-store");
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
	headers = curl_slist_append(headers, "Connection: Keep-Alive");
	if (useSession && _SessionIds.size() > 0)
	{
		srand((unsigned)time(NULL));
		int index = rand() % _SessionIds.size();
		std::stringstream sstr;
		sstr << "Cookie: $Version=\"0\"; SessionId=\"" << _SessionIds.at(index) << "\"; State=\"1\"";
		headers = curl_slist_append(headers, sstr.str().c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	std::string strCookie;
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, &strCookie);
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szPost);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postSize);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	int res = curl_easy_perform(curl);
	if (res == CURLE_OK)
	{
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		if (responseCode != 200)
		{
			strResponse.clear();
			res = responseCode;
		}
	}
	curl_slist_free_all(headers);	
	curl_easy_cleanup(curl);
	return res;
}

std::string CLibGEHelper::randomServerURL()
{
	if (_serverURLs.size() <= 0)
		return "kh.google.com";

	srand((unsigned)time(NULL));
	int index = rand() % _serverURLs.size();
	return _serverURLs.at(index);
}

int  CLibGEHelper::getDBRoot()
{
	std::stringstream ssUrl;
	ssUrl << "http://" << randomServerURL() << "/dbRoot.v5";	
	std::string strResponse;
	int res = Get(ssUrl.str(), strResponse, false);
	if (res != CURLE_OK)
		return false;

	if (strResponse.empty() || strResponse.size()<=1024)
		return false;

	unsigned long len = strResponse.size();
	char* data = strResponse._Myptr();
	unsigned int _magic;
	convertEndian((LPVOID)data, 4, &_magic, true);
	unsigned short _unk;
	convertEndian((LPVOID)(data + 4), 2, &_unk, true);
	convertEndian((LPVOID)(data + 6), 2, &_version, true);	
	data += 8;
	memset(_crypt_key, 0, sizeof(_crypt_key));
	memcpy(_crypt_key + 8, data, 1016);
	data += 1016;
	
	if (memcmp(_crypt_key, GOOGLE_EARTH_CRYPT_KEY, sizeof(GOOGLE_EARTH_CRYPT_KEY)) != 0)
		return false;

	_version = _version ^ 0x4200;
	printf("magic:%X  unk:%X  version: %X\r\n", _magic, _unk, _version);

	std::string strXML = UnPackGEZlib(data, len - 1024);
	if (!strXML.empty())
	{
#ifdef _DEBUG
		{
			std::stringstream ssEncodePath;
			ssEncodePath << _cachePath << "\\dbroot5.xml";

			FILE* fp = nullptr;
			fopen_s(&fp, ssEncodePath.str().c_str(), "wb+");
			if (fp != nullptr)
			{
				fwrite(strXML.c_str(), strXML.size(), 1, fp);
				fclose(fp);
			}
		}
#endif

		CacheManager::GetInstance().BeginTransaction();
		std::string lowerXML = strXML;
		std::transform(lowerXML.begin(), lowerXML.end(), lowerXML.begin(), ::tolower);
		std::string key = "<etproviderinfo> ";
		const char* data = strstr(lowerXML.c_str(), key.c_str());
		while ( data!=nullptr )
		{
			data += key.size();
			char type = 0;
			int id = 0;	
			std::string name;
			if (sscanf_s(data, "[%ld]", &id) == 0)
			{
				sscanf_s(data, "[%c]", &type);
				const char* data1 = strchr(data, '{');
				if (data1 != nullptr)
					sscanf_s(data1 + 1, "%ld", &id);
			}
			
			const char* data1 = strchr(data, '"');
			if (data1 != nullptr)
			{
				data1 += 1;
				const char* data2 = strchr(data1, '"');
				if (data2 != nullptr && (data2 - data1)>0)
				{
					name.resize(data2 - data1);
					memcpy(name._Myptr(), data1, data2 - data1);					
					//replaceString(name, "\\251", "@");
				}
			}

			CacheManager::GetInstance().AddProvider(id, name, type);

			data = strstr(data, key.c_str());
		}
		CacheManager::GetInstance().Commit();
	}

	geauth();
	return true;
}

std::string CLibGEHelper::randomGEAuth()
{
	if (_geAuths.size() <= 0)
		return "";

	srand((unsigned)time(NULL));
	int index = rand() % _geAuths.size();
	return _geAuths.at(index);
}

bool CLibGEHelper::geauth()
{
	if (_geAuths.size() <= 0)
		return false;

	_SessionIds.clear();
	for (int j = 0; j < _geAuths.size(); j++)
	{
		std::stringstream ssUrl;
		ssUrl << "http://" << randomServerURL() << "/geauth";
		std::string geAuth = _geAuths.at(j);
		if (geAuth.size() != 49)
			continue;

		std::string strResponse;
		int res = Post(ssUrl.str(), geAuth.c_str(), 49, strResponse, false);
		if (res != CURLE_OK || strResponse.size() <= 8)
			continue;

		if (memcmp(strResponse.c_str(), GOOGLE_EARTH_GEAUTH_KEY, sizeof(GOOGLE_EARTH_GEAUTH_KEY)) != 0)
			continue;

		char szSession[1024];
		memset(szSession, 0, sizeof(szSession));
		for (int i = 8; i < strResponse.size(); i++)
		{
			if (strResponse.at(i) == 0)
				break;

			szSession[i - 8] = strResponse.at(i);
		}

		_SessionIds.push_back(szSession);
	}
	
	return _SessionIds.size()>0;
}

int CLibGEHelper::getVersion(const char* name, CacheManager::ETableType type)
{
	if (_version <= 0)
		getDBRoot();

	int version = 0;
	if (version <= 0)
	{
		version = CacheManager::GetInstance().GetVersion(name, type);
		if (version <= 0)
		{
			std::string qtName = "0";
			int len = ((strlen(name) - 1) / 4) * 4;
			if (len > 0)
			{
				qtName.resize(len);
				memcpy(qtName._Myptr(), name, len);
			}
			QuadTreePacket16* packet = getQuadtree(qtName.c_str(), 0);
			if (packet == nullptr)
				return 0U;

			delete packet;
			version = CacheManager::GetInstance().GetVersion(name, type);
			if (version <= 0)
				return 0U;
		}
	}

	return version;
}

QuadTreePacket16* CLibGEHelper::getQuadtree(unsigned int x, unsigned int y, unsigned level, int version)
{
	std::string baseGEName = ConvertToQtNode(x, y, level);
	return getQuadtree(baseGEName.c_str(), version);
}

QuadTreePacket16* CLibGEHelper::getQuadtree(const char* baseGEName, int version)
{
	if (version <= 0 && _version <= 0)
	{
		if (!getDBRoot() || _version<=0)
			return nullptr;
	}
	std::stringstream ssUrl;
	std::stringstream ssKey;
	ssKey << "q2-" << baseGEName << "-q." << (version <= 0 ? _version : version);
	ssUrl << "http://" << randomServerURL() << "/flatfile?" << ssKey.str();
	std::string strResponse = getFlatfile(ssUrl.str(), ssKey.str().c_str(), CacheManager::TYPE_QUADTREE);
	if (strResponse.empty())
		return nullptr;

	QuadTreePacket16* packet = DecodeQuadtree(strResponse.c_str(), strResponse.size());
	if (packet == nullptr)
		return nullptr;
	
	//printf("root_numbering\r\n%s\r\n", (packet->ToString(true, true)).c_str());
	//printf("tree_numbering\r\n%s\r\n", (packet->ToString(false, true)).c_str());

	std::vector<QuadtreeDataReference> qtp_refs;
	std::vector<QuadtreeDataReference> qtp2_refs;
	std::vector<QuadtreeDataReference> img_refs;
	std::vector<QuadtreeDataReference> ter_refs;
	std::vector<QuadtreeDataReference> vec_refs;
	QuadtreeDataReferenceGroup ref_group(
		&qtp_refs, &qtp2_refs, &img_refs, &ter_refs, &vec_refs);
	QuadtreePath path_prefix = "";
	packet->GetDataReferences(&ref_group, path_prefix,
		// unspecified date
		kUnknownJpegCommentDate, false);

	CacheManager::GetInstance().BeginTransaction();
	{
		for (std::vector<QuadtreeDataReference>::iterator it = qtp_refs.begin(); it != qtp_refs.end(); it++)
		{
			unsigned int level, row, col;
			std::string name = baseGEName + it->qt_path().AsString();
			ConvertFromQtNode(name, &col, &row, &level);
			//printf("qtp_refs: %s %ld\r\n", name.c_str(), it->version());
			CacheManager::GetInstance().AddVersion(name, col, row, level, it->version(), it->channel(), it->provider(), it->jpeg_date().GetHexString(), CacheManager::TYPE_QUADTREE);
		}
	}

	{
		for (std::vector<QuadtreeDataReference>::iterator it = qtp2_refs.begin(); it != qtp2_refs.end(); it++)
		{
			unsigned int level, row, col;
			it->qt_path().GetLevelRowCol(&level, &row, &col);
		}
	}

	{
		for (std::vector<QuadtreeDataReference>::iterator it = img_refs.begin(); it != img_refs.end(); it++)
		{
			unsigned int level, row, col;
			std::string name = baseGEName + it->qt_path().AsString();
			ConvertFromQtNode(name, &col, &row, &level);
			CacheManager::GetInstance().AddVersion(name, col, row, level, it->version(), it->channel(), it->provider(), it->jpeg_date().GetHexString(), CacheManager::TYPE_IMAGE);
		}
	}

	{
		for (std::vector<QuadtreeDataReference>::iterator it = ter_refs.begin(); it != ter_refs.end(); it++)
		{
			unsigned int level, row, col;
			std::string name = baseGEName + it->qt_path().AsString();
			ConvertFromQtNode(name, &col, &row, &level);
			CacheManager::GetInstance().AddVersion(name, col, row, level, it->version(), it->channel(), it->provider(), it->jpeg_date().GetHexString(), CacheManager::TYPE_TERRAIN);
		}
	}

	{
		for (std::vector<QuadtreeDataReference>::iterator it = vec_refs.begin(); it != vec_refs.end(); it++)
		{
			unsigned int level, row, col;
			std::string name = baseGEName + it->qt_path().AsString();
			ConvertFromQtNode(name, &col, &row, &level);
			CacheManager::GetInstance().AddVersion(name, col, row, level, it->version(), it->channel(), it->provider(), it->jpeg_date().GetHexString(), CacheManager::TYPE_VECTOR);
		}
	}
	CacheManager::GetInstance().Commit();
	return packet;
}

std::string CLibGEHelper::getImage(unsigned int x, unsigned int y, unsigned level, int version, bool is_mercator)
{
	std::string baseGEName = ConvertToQtNode(x, y, level);
	return getImage(baseGEName.c_str(), version, is_mercator);
}

std::string CLibGEHelper::getImage(const char* name, int version, bool is_mercator)
{
	if (version <= 0)
	{
		version = getVersion(name, CacheManager::TYPE_IMAGE);
		if (version <= 0)
			return "";
	}

	std::stringstream ssUrl;
	std::stringstream ssKey;
	ssKey << "f1-" << name << "-i." << version;
	ssUrl << "http://" << randomServerURL() << "/flatfile?" << ssKey.str();
	std::string strResponse = getFlatfile(ssUrl.str(), ssKey.str().c_str(), CacheManager::TYPE_IMAGE);
	if (strResponse.empty())
		return "";

	const char* data = strResponse.c_str();
	size_t srcSize = strResponse.size();
	unsigned short bof, eof;
	convertEndian((LPVOID)data, 2, &bof, true);
	convertEndian((LPVOID)(data + srcSize - 2), 2, &eof, true);
	if (bof == BOF_JPEG && eof == EOF_JPEG)
	{
#ifdef _DEBUG
		std::string imgFilePath;
		if (imgFilePath.empty())
		{
			unsigned int x, y, level;
			ConvertFromQtNode(name, &x, &y, &level);

			std::stringstream ssEncodePath;
			//ssEncodePath << _cachePath << "\\" << name << ".jpg";
			ssEncodePath << _cachePath << "\\" << level;
			_mkdir(ssEncodePath.str().c_str());

			ssEncodePath << "\\" << x;
			_mkdir(ssEncodePath.str().c_str());

			ssEncodePath << "\\" << y << ".jpg";
			imgFilePath = ssEncodePath.str();
		}

		std::string strPrj;
		if (is_mercator)
			strPrj = "PROJCS[\"WGS 84 / Pseudo - Mercator\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],EXTENSION[\"PROJ4\",\" + proj = merc + a = 6378137 + b = 6378137 + lat_ts = 0.0 + lon_0 = 0.0 + x_0 = 0.0 + y_0 = 0 + k = 1.0 + units = m + nadgrids = @null + wktext + no_defs\"],AUTHORITY[\"EPSG\",\"3857\"]]";
		else
			strPrj = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";
		std::stringstream ssMemFile;
		ssMemFile << "/vsimem/" << name;
		VSIFCloseL(VSIFileFromMemBuffer(ssMemFile.str().c_str(), (GByte*)data, srcSize, false));
		GDALDataset* readDS = (GDALDataset*)GDALOpen(ssMemFile.str().c_str(), GA_ReadOnly);
		VSIUnlink(ssMemFile.str().c_str());
		if (readDS != nullptr)
		{
			readDS->SetProjection(strPrj.c_str());

			double minX, minY, maxX, maxY;
			unsigned int tmplevel;
			QtNodeBounds(name, is_mercator, &minY, &minX, &maxY, &maxX, &tmplevel);
			double cellSizeX = (maxX - minX) / 256.0;
			double cellSizeY = (minY - maxY) / 256.0;
			double adfGeoTransform[6];
			adfGeoTransform[0] = minX;
			adfGeoTransform[1] = cellSizeX;
			adfGeoTransform[2] = 0;
			adfGeoTransform[3] = maxY;
			adfGeoTransform[4] = 0;
			adfGeoTransform[5] = cellSizeY;
			readDS->SetGeoTransform(adfGeoTransform);

			const char *pszFormat = "JPEG";
			GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
			if (poDriver != nullptr)
			{
				GDALDatasetH poTmpDS = GDALCreateCopy(poDriver, imgFilePath.c_str(), readDS, false, nullptr, nullptr, nullptr);
				if (poTmpDS != nullptr)
					GDALClose(poTmpDS);
			}
			GDALClose(readDS);
		}
#endif
	}

	return strResponse;
}

std::string CLibGEHelper::getImage(double minX, double minY, double maxX, double maxY, unsigned int level, unsigned int rasterXSize, unsigned int rasterYSize, bool is_mercator)
{
	std::vector<std::string> names = ConvertToQtNode(minY, minX, maxY, maxX, level, is_mercator);
	if (names.size() <= 0)
		return "";

	std::string strPrj;
	if (is_mercator)
		strPrj = "PROJCS[\"WGS 84 / Pseudo - Mercator\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],EXTENSION[\"PROJ4\",\" + proj = merc + a = 6378137 + b = 6378137 + lat_ts = 0.0 + lon_0 = 0.0 + x_0 = 0.0 + y_0 = 0 + k = 1.0 + units = m + nadgrids = @null + wktext + no_defs\"],AUTHORITY[\"EPSG\",\"3857\"]]";
	else
		strPrj = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";

	GDALDriver *poMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	if (poMemDriver == nullptr)
		return "";

	double resX = (maxX - minX) / rasterXSize;
	double resY = (minY - maxY) / rasterYSize;

	VRTDatasetH hVRTDS = VRTCreate(rasterXSize, rasterYSize);
	if (hVRTDS == nullptr)
		return "";

	GDALSetProjection(hVRTDS, strPrj.c_str());
	double adfGeoTransform[6];
	adfGeoTransform[0] = minX;
	adfGeoTransform[1] = resX;
	adfGeoTransform[2] = 0;
	adfGeoTransform[3] = maxY;
	adfGeoTransform[4] = 0;
	adfGeoTransform[5] = resY;
	GDALSetGeoTransform(hVRTDS, adfGeoTransform);

	std::string allName;
	bool firstDS = true;
	for (int i = 0; i < names.size(); i++)
	{
		std::string name = names.at(i);
		if (allName.empty())
			allName = name;
		else
			allName += "_" + name;
		std::string imgData = getImage(name.c_str(), 0, is_mercator);
		if (imgData.size() <= 0)
			continue;

		double tmpMinX, tmpMinY, tmpMaxX, tmpMaxY;
		unsigned int tmplevel;
		QtNodeBounds(name, is_mercator, &tmpMinY, &tmpMinX, &tmpMaxY, &tmpMaxX, &tmplevel);
		double cellSizeX = (tmpMaxX - tmpMinX) / 256.0;
		double cellSizeY = (tmpMinY - tmpMaxY) / 256.0;
		double adfGeoTransform[6];
		adfGeoTransform[0] = tmpMinX;
		adfGeoTransform[1] = cellSizeX;
		adfGeoTransform[2] = 0;
		adfGeoTransform[3] = tmpMaxY;
		adfGeoTransform[4] = 0;
		adfGeoTransform[5] = cellSizeY;

		std::stringstream ssMemFile;
		ssMemFile << "/vsimem/" << name;
		VSIFCloseL(VSIFileFromMemBuffer(ssMemFile.str().c_str(), (GByte*)(imgData._Myptr()), imgData.size(), false));
		GDALDataset* readDS = (GDALDataset*)GDALOpen(ssMemFile.str().c_str(), GA_ReadOnly);				
		if (readDS == nullptr)
		{
			VSIUnlink(ssMemFile.str().c_str());
			continue;
		}

		readDS->SetProjection(strPrj.c_str());
		readDS->SetGeoTransform(adfGeoTransform);

		GDALDataset *poMemDS = poMemDriver->CreateCopy("", readDS, false, nullptr, nullptr, nullptr);
		GDALClose(readDS);
		VSIUnlink(ssMemFile.str().c_str());
		if (poMemDS == nullptr)
			continue;

		if (firstDS)
		{
			for (int j = 0; j < poMemDS->GetRasterCount(); j++)
			{
				GDALRasterBandH hBand;
				GDALAddBand(hVRTDS, poMemDS->GetRasterBand(j + 1)->GetRasterDataType(), nullptr);
				hBand = GDALGetRasterBand(hVRTDS, j + 1);
				GDALSetRasterColorInterpretation(hBand, poMemDS->GetRasterBand(j + 1)->GetColorInterpretation());
				if (poMemDS->GetRasterBand(j + 1)->GetColorInterpretation() == GCI_PaletteIndex)				
					GDALSetRasterColorTable(hBand, poMemDS->GetRasterBand(j + 1)->GetColorTable());

				int hasNoDataValue = 0;
				double noDataValue = poMemDS->GetRasterBand(j + 1)->GetNoDataValue(&hasNoDataValue);
				if (hasNoDataValue)
					GDALSetRasterNoDataValue(hBand, noDataValue);
			}
			firstDS = false;
		}

		int left = 0;
		int top = 0;
		int right = poMemDS->GetRasterXSize();
		int bottom = poMemDS->GetRasterYSize();
		if (tmpMinX < minX)
			left = (int)(0.5 + (minX - tmpMinX) / cellSizeX);
		if ( tmpMaxX>maxX )
			right = (int)(0.5 + (maxX - tmpMinX) / cellSizeX);
		if (tmpMinY<minY)
			bottom = (int)(0.5 + (tmpMaxY - minY) / -cellSizeY);
		if (tmpMaxY > maxY)
			top = (int)(0.5 + (tmpMaxY - maxY) / -cellSizeY);
		left = __min(__max(0, left), poMemDS->GetRasterXSize()-1);
		right = __min(__max(0, right), poMemDS->GetRasterXSize());
		top = __min(__max(0, top), poMemDS->GetRasterYSize()-1);
		bottom = __min(__max(0, bottom), poMemDS->GetRasterYSize());
		if (left == right || top == bottom)
		{
			GDALClose(poMemDS);
			continue;
		}

		int xoffset = (int)(0.5 + (__max(tmpMinX, minX) - minX) / resX);
		int yoffset = (int)(0.5 + (maxY - __min(tmpMaxY, maxY)) / -resY);
		int xoffset2 = (int)(0.5 + (__min(tmpMaxX, maxX) - minX) / resX);
		int yoffset2 = (int)(0.5 + (maxY - __max(tmpMinY, minY)) / -resY);
		int dest_width = xoffset2 - xoffset;// (int)(0.5 + (right - left) * cellSizeX / resX);
		int dest_height = yoffset2 - yoffset;// (int)(0.5 + (bottom - top) * cellSizeY / resY);
		xoffset = __min(__max(0, xoffset), rasterXSize-1);
		dest_width = __min(__max(0, dest_width), rasterXSize);
		yoffset = __min(__max(0, yoffset), rasterYSize-1);
		dest_height = __min(__max(0, dest_height), rasterYSize);

		//printf("%03ld-%03ld  %03ld-%03ld", xoffset, xoffset + dest_width - 1, yoffset, yoffset + dest_height - 1);
		//printf("\t%03ld-%03ld  %03ld-%03ld", left, right, top, bottom);
		//printf("\t%lf-%lf  %lf-%lf\r\n", tmpMinX, tmpMaxX, tmpMinY, tmpMaxY);

		for (int j = 0; j < poMemDS->GetRasterCount(); j++)
		{
			VRTSourcedRasterBandH hVRTBand = (VRTSourcedRasterBandH)GDALGetRasterBand(hVRTDS, j + 1);

			/* Place the raster band at the right position in the VRT */
			VRTAddSimpleSource(hVRTBand, GDALGetRasterBand(poMemDS, j + 1),
				left, top,
				right - left,
				bottom - top,
				xoffset, yoffset,
				dest_width, dest_height, "near",
				VRT_NODATA_UNSET);
		}
	}

	std::string imgData;
	if (GDALGetRasterCount(hVRTDS) > 0)
	{
		const char *pszFormat = "JPEG";
		GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		std::string outName = "/vsimem/" + base64_encode((const unsigned char*)allName.c_str(), allName.size()) + ".jpg";
		GDALDatasetH hOutDS = GDALCreateCopy(poDriver, outName.c_str(), hVRTDS, false, nullptr, nullptr, nullptr);
		GDALClose(hOutDS);
		vsi_l_offset outDataLength = 0;
		int bUnlinkAndSeize = true;
		GByte * binData = VSIGetMemFileBuffer(outName.c_str(), &outDataLength, bUnlinkAndSeize);
		if (outDataLength > 0 && binData != nullptr)
		{
			imgData.resize(outDataLength);
			memcpy(imgData._Myptr(), binData, outDataLength);
			CPLFree(binData);
		}
	}
	GDALClose(hVRTDS);
	return imgData;
}

std::string CLibGEHelper::getTerrain(unsigned int x, unsigned int y, unsigned level, int version, int* pCols, int* pRows, bool is_mercator)
{
	std::string baseGEName = ConvertToQtNode(x, y, level);
	return getTerrain(baseGEName.c_str(), version, pCols, pRows, is_mercator);
}

std::string CLibGEHelper::getTerrain(const char* name, int version, int* pCols, int* pRows, bool is_mercator)
{
	std::string newName = name;
	if (newName.size() % 2 == 0)
		newName.resize(newName.size() - 1);
	
	if (version <= 0)
	{
		version = getVersion(newName.c_str(), CacheManager::TYPE_TERRAIN);
		if (version <= 0)
			return "";
	}

	std::stringstream ssUrl;
	std::stringstream ssKey;
	ssKey << "f1c-" << newName << "-t." << version;
	ssUrl << "http://" << randomServerURL() << "/flatfile?" << ssKey.str();
	std::string strResponse = getFlatfile(ssUrl.str(), ssKey.str().c_str(), CacheManager::TYPE_TERRAIN);
	if (strResponse.empty())
		return "";

	Terrain terrain;
	if (!terrain.decode(strResponse.c_str(), strResponse.size()))
		return "";

	{		
		for (Terrain::iterator it = terrain.begin(); it!=terrain.end(); it++)
		{
			if ( _stricmp(name, it->name().c_str())!=0 )
				continue;

			int nCols = 0;
			int nRows = 0;
			std::string imgData = terrain.toDEM(*it, nCols, nRows, is_mercator);
			if (imgData.size() <= 0)
				return "";

			if (pCols)
				*pCols = nCols;

			if (pRows)
				*pRows = nRows;

#ifdef _DEBUG
			unsigned int x, y, level;
			ConvertFromQtNode(it->name(), &x, &y, &level);
			std::stringstream ssEncodePath;			
			ssEncodePath << _cachePath << "\\" << level;
			_mkdir(ssEncodePath.str().c_str());

			ssEncodePath << "\\" << x;
			_mkdir(ssEncodePath.str().c_str());

			ssEncodePath << "\\" << y << ".tif";
			
			std::string strPrj;
			if (is_mercator)
				strPrj = "PROJCS[\"WGS 84 / Pseudo - Mercator\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],EXTENSION[\"PROJ4\",\" + proj = merc + a = 6378137 + b = 6378137 + lat_ts = 0.0 + lon_0 = 0.0 + x_0 = 0.0 + y_0 = 0 + k = 1.0 + units = m + nadgrids = @null + wktext + no_defs\"],AUTHORITY[\"EPSG\",\"3857\"]]";
			else
				strPrj = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";

			GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");			
			char **papszOptions = nullptr;
			papszOptions = CSLSetNameValue(papszOptions, "TFW", "ON");
			GDALDataset* poOutDS = poDriver->Create(ssEncodePath.str().c_str(), nCols, nRows, 1, GDT_Float32, papszOptions);
			if (poOutDS != nullptr)
			{
				double tmpMinX, tmpMinY, tmpMaxX, tmpMaxY;
				unsigned int tmplevel;
				QtNodeBounds(it->name(), is_mercator, &tmpMinY, &tmpMinX, &tmpMaxY, &tmpMaxX, &tmplevel);
				double cellSizeX = (tmpMaxX - tmpMinX) / nCols;
				double cellSizeY = (tmpMinY - tmpMaxY) / nRows;
				double adfGeoTransform[6];
				adfGeoTransform[0] = tmpMinX;
				adfGeoTransform[1] = cellSizeX;
				adfGeoTransform[2] = 0;
				adfGeoTransform[3] = tmpMaxY;
				adfGeoTransform[4] = 0;
				adfGeoTransform[5] = cellSizeY;
				poOutDS->SetGeoTransform(adfGeoTransform);
				poOutDS->SetProjection(strPrj.c_str());
				poOutDS->GetRasterBand(1)->SetNoDataValue(-FLT_MAX);
				poOutDS->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, nCols, nRows, imgData._Myptr(), nCols, nRows, GDT_Float32, 0, 0);
				poOutDS->FlushCache();				
				GDALClose(poOutDS);
			}
			if (papszOptions != nullptr)
				CSLDestroy(papszOptions);
#endif
			return imgData;
		}		
	}

	return "";
}

std::string CLibGEHelper::getTerrain(double minX, double minY, double maxX, double maxY, unsigned int level, unsigned int rasterXSize, unsigned int rasterYSize, bool is_mercator)
{
	std::vector<std::string> names = ConvertToQtNode(minY, minX, maxY, maxX, level, is_mercator);
	if (names.size() <= 0)
		return "";

	std::string strPrj;
	if (is_mercator)
		strPrj = "PROJCS[\"WGS 84 / Pseudo - Mercator\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],EXTENSION[\"PROJ4\",\" + proj = merc + a = 6378137 + b = 6378137 + lat_ts = 0.0 + lon_0 = 0.0 + x_0 = 0.0 + y_0 = 0 + k = 1.0 + units = m + nadgrids = @null + wktext + no_defs\"],AUTHORITY[\"EPSG\",\"3857\"]]";
	else
		strPrj = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";

	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == nullptr)
		return "";

	double resX = (maxX - minX) / rasterXSize;
	double resY = (minY - maxY) / rasterYSize;

	VRTDatasetH hVRTDS = VRTCreate(rasterXSize, rasterYSize);
	if (hVRTDS == nullptr)
		return "";

	GDALSetProjection(hVRTDS, strPrj.c_str());
	double adfGeoTransform[6];
	adfGeoTransform[0] = minX;
	adfGeoTransform[1] = resX;
	adfGeoTransform[2] = 0;
	adfGeoTransform[3] = maxY;
	adfGeoTransform[4] = 0;
	adfGeoTransform[5] = resY;
	GDALSetGeoTransform(hVRTDS, adfGeoTransform);

	std::vector<std::string> vtMemFiles;
	std::string allName;
	bool firstDS = true;
	for (int i = 0; i < names.size(); i++)
	{
		std::string name = names.at(i);
		if (allName.empty())
			allName = name;
		else
			allName += "_" + name;

		int nCols = 0;
		int nRows = 0;
		std::string imgData = getTerrain(name.c_str(), 0, &nCols, &nRows, is_mercator);
		if (imgData.size() <= 0)
			continue;

		double tmpMinX, tmpMinY, tmpMaxX, tmpMaxY;
		unsigned int tmplevel;
		QtNodeBounds(name, is_mercator, &tmpMinY, &tmpMinX, &tmpMaxY, &tmpMaxX, &tmplevel);
		double cellSizeX = (tmpMaxX - tmpMinX) / nCols;
		double cellSizeY = (tmpMinY - tmpMaxY) / nRows;
		double adfGeoTransform[6];
		adfGeoTransform[0] = tmpMinX;
		adfGeoTransform[1] = cellSizeX;
		adfGeoTransform[2] = 0;
		adfGeoTransform[3] = tmpMaxY;
		adfGeoTransform[4] = 0;
		adfGeoTransform[5] = cellSizeY;

		std::stringstream ssMemFile;
		ssMemFile << "/vsimem/" << name << ".tif";
		GDALDataset* poMemDS = poDriver->Create(ssMemFile.str().c_str(), nCols, nRows, 1, GDT_Float32, nullptr);
		if (poMemDS == nullptr)
			continue;

		poMemDS->SetGeoTransform(adfGeoTransform);
		poMemDS->SetProjection(strPrj.c_str());
		poMemDS->GetRasterBand(1)->SetNoDataValue(-FLT_MAX);
		poMemDS->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, nCols, nRows, imgData._Myptr(), nCols, nRows, GDT_Float32, 0, 0);
		poMemDS->FlushCache();
		vtMemFiles.push_back(ssMemFile.str());

		if (firstDS)
		{
			for (int j = 0; j < poMemDS->GetRasterCount(); j++)
			{
				GDALRasterBandH hBand;
				GDALAddBand(hVRTDS, poMemDS->GetRasterBand(j + 1)->GetRasterDataType(), nullptr);
				hBand = GDALGetRasterBand(hVRTDS, j + 1);
				GDALSetRasterColorInterpretation(hBand, poMemDS->GetRasterBand(j + 1)->GetColorInterpretation());				
				int hasNoDataValue = 0;
				double noDataValue = poMemDS->GetRasterBand(j + 1)->GetNoDataValue(&hasNoDataValue);
				if (hasNoDataValue)
					GDALSetRasterNoDataValue(hBand, noDataValue);
			}
			firstDS = false;
		}

		int left = 0;
		int top = 0;
		int right = poMemDS->GetRasterXSize();
		int bottom = poMemDS->GetRasterYSize();
		if (tmpMinX < minX)
			left = (int)(0.5 + (minX - tmpMinX) / cellSizeX);
		if (tmpMaxX > maxX)
			right = (int)(0.5 + (maxX - tmpMinX) / cellSizeX) + 1;
		if (tmpMinY < minY)
			bottom = (int)(0.5 + (tmpMaxY - minY) / -cellSizeY) + 1;
		if (tmpMaxY > maxY)
			top = (int)(0.5 + (tmpMaxY - maxY) / -cellSizeY);
		left = __min(__max(0, left), poMemDS->GetRasterXSize() - 1);
		right = __min(__max(0, right), poMemDS->GetRasterXSize());
		top = __min(__max(0, top), poMemDS->GetRasterYSize() - 1);
		bottom = __min(__max(0, bottom), poMemDS->GetRasterYSize());
		if (left == right || top == bottom)
		{
			GDALClose(poMemDS);
			continue;
		}

		int xoffset = (int)(0.5 + (__max(tmpMinX, minX) - minX) / resX);
		int yoffset = (int)(0.5 + (maxY - __min(tmpMaxY, maxY)) / -resY);
		int xoffset2 = (int)(0.5 + (__min(tmpMaxX, maxX) - minX) / resX);
		int yoffset2 = (int)(0.5 + (maxY - __max(tmpMinY, minY)) / -resY);
		int dest_width = xoffset2 - xoffset;// (int)(0.5 + (right - left) * cellSizeX / resX);
		int dest_height = yoffset2 - yoffset;// (int)(0.5 + (bottom - top) * cellSizeY / resY);
		xoffset = __min(__max(0, xoffset), rasterXSize-1);
		dest_width = __min(__max(0, dest_width), rasterXSize);
		yoffset = __min(__max(0, yoffset), rasterYSize-1);
		dest_height = __min(__max(0, dest_height), rasterYSize);


		for (int j = 0; j < poMemDS->GetRasterCount(); j++)
		{
			VRTSourcedRasterBandH hVRTBand = (VRTSourcedRasterBandH)GDALGetRasterBand(hVRTDS, j + 1);

			/* Place the raster band at the right position in the VRT */
			VRTAddSimpleSource(hVRTBand, GDALGetRasterBand(poMemDS, j + 1),
				left, top,
				right - left,
				bottom - top,
				xoffset, yoffset,
				dest_width, dest_height, "near",
				VRT_NODATA_UNSET);
		}
	}

	std::string imgData;
	if (GDALGetRasterCount(hVRTDS)>0)
	{
		imgData.resize(rasterXSize*rasterYSize*sizeof(float));
		GDALRasterIO(GDALGetRasterBand(hVRTDS, 1), GF_Read, 0, 0, rasterXSize, rasterYSize, imgData._Myptr(), rasterXSize, rasterYSize, GDT_Float32, 0, 0);
	}	
	GDALClose(hVRTDS);

	for (int i = 0; i < vtMemFiles.size(); i++)
		VSIUnlink(vtMemFiles[i].c_str());
	return imgData;
}


std::string CLibGEHelper::getGEName(const std::string& strUrl)
{
	std::string name;
	std::string lowerUrl = strUrl;
	std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);
	std::string key = "flatfile?";
	const char* nameUrl = strstr(lowerUrl.c_str(), key.c_str());
	if (nameUrl == nullptr)
		return name;
	nameUrl += key.size();

	const char* pos = strstr(nameUrl, "-");
	if (pos == nullptr)
		return name;
	pos += 1;

	const char* pos2 = strstr(pos, "-");
	if (pos2 == nullptr)
		return name;

	unsigned int size = pos2 - pos;
	name.resize(size);
	memset(name._Myptr(), 0, name.size());	
	memcpy(name._Myptr(), pos, size);
	return name;
}

CLibGEHelper::EFaltfileType CLibGEHelper::faltFileType(const std::string& strUrl)
{
	EFaltfileType type = FALTFILE_UNKNOWN;
	std::string lowerUrl = strUrl;
	std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);
	std::string key = "flatfile?";
	const char* nameUrl = strstr(lowerUrl.c_str(), key.c_str());
	if ( nameUrl==nullptr )
		return type;
	nameUrl += key.size();
	
	const char* posF1 = strstr(nameUrl, "f1-");
	const char* posF1C = strstr(nameUrl, "f1c-");
	const char* posQ2 = strstr(nameUrl, "q2-");
	const char* posQP = strstr(nameUrl, "qp-");
	const char* posI = strstr(nameUrl, "-i.");
	const char* posD = strstr(nameUrl, "-d.");
	const char* posT = strstr(nameUrl, "-t.");
	const char* posQ = strstr(nameUrl, "-q.");
	if (posF1 != nullptr && posI != nullptr)
		type = FALTFILE_IMAGE;
	else if (posF1 != nullptr && posD != nullptr)
		type = FALTFILE_TEXTURE;
	else if (posF1C != nullptr && posD != nullptr)
		type = FALTFILE_LAYER;
	else if (posF1 != nullptr && posD != nullptr)
		type = FALTFILE_LAYER_3D;
	else if (posF1C != nullptr && posT != nullptr)
		type = FALTFILE_TERRAIN;
	else if (posQ2 != nullptr && posQ != nullptr)
		type = FALTFILE_Q2TREE;
	else if (posQP != nullptr && posQ != nullptr)
		type = FALTFILE_QPTREE;
	return type;
}

std::string CLibGEHelper::getFlatfile(const std::string& url, const std::string& key, CacheManager::ETableType type)
{
	if (url.empty())
		return "";	

	std::string strResponse = CacheManager::GetInstance().GetFaltfile(key, type);
	if (strResponse.empty())
	{
		int res = Get(url, strResponse, true);
#ifdef _DEBUG
		printf("getFlatfile %s: %s\r\n", ((res == CURLE_OK && !strResponse.empty()) ? "Success" : "Failed"), url.c_str());
#endif
		if (res != CURLE_OK)
			return "";

		CacheManager::GetInstance().AddFaltfile(key, strResponse, type);
	}
	else
	{
#ifdef _DEBUG
		printf("getFlatfile %s From Cache: %s\r\n", "Success", url.c_str());
#endif
	}

	if (strResponse.empty())
		return "";

	return UnPackGEZlib(strResponse._Myptr(), strResponse.size());
}
LIBGE_NAMESPACE_END