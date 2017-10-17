#include "LZMA.h"
#include "CompressionManager.h"
#include "util_err.h"

#include <iostream>

#include <lzmasdk/C/Alloc.h>
#include <lzmasdk/C/LzmaEnc.h>
#include <lzmasdk/C/LzmaDec.h>
#include <lzmasdk/C/XzEnc.h>
#include <lzmasdk/C/Xz.h>
#include <lzmasdk/C/7zCrc.h>
#include <lzmasdk/C/XzCrc64.h>
#include <lzmasdk/C/7z.h>
#include <lzmasdk/C/7zBuf.h>

const int IN_BUF_SIZE = (1 << 16);
const int OUT_BUF_SIZE = (1 << 16);

struct LzmaIStream : public ISeqInStream {
	LzmaIStream(std::istream& file) : _file(file) {
		this->Read = LzmaIStream::_read;
	}
	std::istream& _file;
	static SRes _read(const ISeqInStream *p, void *buf, size_t *size) {
		size_t originalSize = *size;
		if (originalSize > 0) {
			((LzmaIStream*)p)->_file.read((char*)buf, originalSize);
			*size = ((LzmaIStream*)p)->_file.gcount();
		}
		return SZ_OK;
	}
};

struct LzmaIStream2 : public ISeekInStream {
	LzmaIStream2(std::istream& file) : _file(file) {
		this->Read = LzmaIStream2::_read;
		this->Seek = LzmaIStream2::_seek;
	}
	std::istream& _file;
	static SRes _read(const ISeekInStream *p, void *buf, size_t *size) {
		size_t originalSize = *size;
		if (originalSize > 0) {
			((LzmaIStream2*)p)->_file.read((char*)buf, originalSize);
			*size = ((LzmaIStream2*)p)->_file.gcount();
		}
		return SZ_OK;
	}
	static SRes _seek(const ISeekInStream *p, Int64 *pos, ESzSeek origin) {
		((LzmaIStream2*)p)->_file.seekg(*pos, origin);
		*pos = ((LzmaIStream2*)p)->_file.tellg();
		return SZ_OK;
	}
};

struct LzmaOStream : public ISeqOutStream {
	LzmaOStream(std::ostream& file) : _file(file) {
		this->Write = LzmaOStream::_write;
	}
	std::ostream& _file;
	static size_t _write(const ISeqOutStream *p, const void *buf, size_t size) {
		((LzmaOStream*)p)->_file.write((const char*)buf, size);
		return size;
	}
};

// used by 7z and xz
static void _Init(){
	static bool _init = false;
	if (!_init){
		CrcGenerateTable();
		Crc64GenerateTable();
		_init = true;
	}
}

static SRes Decode2_LZMA(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 unpackSize)
{
	bool thereIsSize = (unpackSize != (UInt64)(Int64)-1);
	Byte inBuf[IN_BUF_SIZE];
	Byte outBuf[OUT_BUF_SIZE];
	size_t inPos = 0, inSize = 0, outPos = 0;
	LzmaDec_Init(state);
	for (;;)
	{
		if (inPos == inSize)
		{
			inSize = IN_BUF_SIZE;
			RINOK(inStream->Read(inStream, inBuf, &inSize));
			inPos = 0;
		}

		SRes res;
		SizeT inProcessed = inSize - inPos;
		SizeT outProcessed = OUT_BUF_SIZE - outPos;
		ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
		ELzmaStatus status;
		if (thereIsSize && outProcessed > unpackSize)
		{
			outProcessed = (SizeT)unpackSize;
			finishMode = LZMA_FINISH_END;
		}

		res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
			inBuf + inPos, &inProcessed, finishMode, &status);
		inPos += inProcessed;
		outPos += outProcessed;
		unpackSize -= outProcessed;

		if (outStream)
			if (outStream->Write(outStream, outBuf, outPos) != outPos)
				return SZ_ERROR_WRITE;

		outPos = 0;

		if (res != SZ_OK || (thereIsSize && unpackSize == 0))
			return res;

		if (inProcessed == 0 && outProcessed == 0)
		{
			if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
				return SZ_ERROR_DATA;
			return res;
		}

	}
}

static SRes Decode2_XZ(CXzUnpacker *state, ISeqOutStream *outStream, ISeqInStream *inStream)
{
	Byte inBuf[IN_BUF_SIZE];
	Byte outBuf[OUT_BUF_SIZE];
	size_t inPos = 0, inSize = 0, outPos = 0;
	for (;;)
	{
		if (inPos == inSize)
		{
			inSize = IN_BUF_SIZE;
			RINOK(inStream->Read(inStream, inBuf, &inSize));
			inPos = 0;
		}

		SRes res;
		SizeT inProcessed = inSize - inPos;
		SizeT outProcessed = OUT_BUF_SIZE - outPos;
		ECoderStatus status;

		res = XzUnpacker_Code(state, outBuf + outPos, &outProcessed,
			inBuf + inPos, &inProcessed, CODER_FINISH_ANY, &status);
		inPos += inProcessed;
		outPos += outProcessed;

		if (outStream)
			if (outStream->Write(outStream, outBuf, outPos) != outPos)
				return SZ_ERROR_WRITE;

		outPos = 0;

		if (res != SZ_OK)
			return res;

		if (inProcessed == 0 && outProcessed == 0)
		{
			if (!XzUnpacker_IsStreamWasFinished(state))
				return SZ_ERROR_DATA;
			return res;
		}

	}
}

static bool decompressLZMA(std::istream& fin, std::ostream& fout) {
	bool ret = false;

	LzmaIStream ii(fin);
	LzmaOStream oo(fout);

	char header[LZMA_PROPS_SIZE];
	UInt64 size = 0;

	CLzmaDec lzmaState;

	fin.read(header, LZMA_PROPS_SIZE);
	for (int i = 0; i < 8; i++){
		char c;
		fin.read(&c, 1);
		size |= ((UInt64)(unsigned char)c) << (8 * i);
	}
	UTIL_DEBUG "size=" << size << std::endl;

	LzmaDec_Construct(&lzmaState);
	if (LzmaDec_Allocate(&lzmaState, (const Byte*)header, LZMA_PROPS_SIZE, &g_Alloc) == 0){
		UTIL_DEBUG "LzmaDec_Allocate OK" << std::endl;
		SRes res = Decode2_LZMA(&lzmaState, &oo, &ii, size);
		UTIL_DEBUG "Decode2_LZMA=" << res << std::endl;
		ret = res ? false : true;
		LzmaDec_Free(&lzmaState, &g_Alloc);
	}

	return ret;
}

static bool decompressXZ(std::istream& fin, std::ostream& fout) {
	bool ret = false;

	LzmaIStream ii(fin);
	LzmaOStream oo(fout);

	CXzUnpacker xzState;

	_Init();
	XzUnpacker_Construct(&xzState, &g_Alloc);

	SRes res = Decode2_XZ(&xzState, &oo, &ii);
	UTIL_DEBUG "Decode2_XZ=" << res << std::endl;
	ret = res ? false : true;

	XzUnpacker_Free(&xzState);

	return ret;
}

static bool compressLZMA(std::istream& fin, std::ostream& fout) {
	bool ret = false;

	LzmaIStream ii(fin);
	LzmaOStream oo(fout);

	CLzmaEncHandle lzmaEnc;
	CLzmaEncProps lzmaProps;

	UInt64 size = 0;

	fin.seekg(0, std::ios::end);
	size = fin.tellg();
	fin.seekg(0);

	UTIL_DEBUG "size=" << size << std::endl;

	if ((lzmaEnc = LzmaEnc_Create(&g_Alloc)) == NULL) return false;
	UTIL_DEBUG "LzmaEnc_Create OK" << std::endl;

	LzmaEncProps_Init(&lzmaProps);
	if (LzmaEnc_SetProps(lzmaEnc, &lzmaProps) == SZ_OK) {
		UTIL_DEBUG "LzmaEnc_SetProps OK" << std::endl;
		Byte header[LZMA_PROPS_SIZE + 8];
		size_t headerSize = LZMA_PROPS_SIZE;

		SRes res = LzmaEnc_WriteProperties(lzmaEnc, header, &headerSize);
		if (res == SZ_OK) {
			for (int i = 0; i < 8; i++)
				header[headerSize++] = (Byte)(size >> (8 * i));
			fout.write((const char*)header, headerSize);
			res = LzmaEnc_Encode(lzmaEnc, &oo, &ii, NULL, &g_Alloc, &g_Alloc);
		}
		UTIL_DEBUG "LzmaEnc_Encode=" << res << std::endl;

		if (res == SZ_OK) ret = true;
	}
	LzmaEnc_Destroy(lzmaEnc, &g_Alloc, &g_Alloc);

	return ret;
}

static bool compressXZ(std::istream& fin, std::ostream& fout) {
	bool ret = false;

	LzmaIStream ii(fin);
	LzmaOStream oo(fout);

	CXzProps xzProps;

	UInt64 size = 0;

	fin.seekg(0, std::ios::end);
	size = fin.tellg();
	fin.seekg(0);

	UTIL_DEBUG "size=" << size << std::endl;

	_Init();
	XzProps_Init(&xzProps);

	SRes res = Xz_Encode(&oo, &ii, &xzProps, NULL);
	UTIL_DEBUG "Xz_Encode=" << res << std::endl;

	if (res == SZ_OK) ret = true;

	return ret;
}

void registerLZMACompression(CompressionManager* mgr) {
	_Init();
	mgr->registerCompressedStreamRoutine("lzma", compressLZMA, decompressLZMA);
	mgr->registerCompressedStreamRoutine("xz", compressXZ, decompressXZ);
}
