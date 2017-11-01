#include "CompressionManager.h"
#include "LZMA.h"
#include "util_err.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

CompressionManager *CompressionManager::instance = NULL;

namespace util {

	ostream::ostream()
		: uncompressed(NULL)
		, compressed(NULL)
		, compressRoutine(NULL)
	{

	}

	ostream::~ostream() {
		if (compressed && compressRoutine) {
			std::stringstream *ss = dynamic_cast<std::stringstream*>(uncompressed);
			ss->seekg(0);
			if (!compressRoutine(*ss, *compressed)) {
				UTIL_WARN "Failed to compress a compressed stream" << std::endl;
			}
		}

		if (uncompressed) delete uncompressed;
		if (compressed) delete compressed;
	}

}

CompressionManager::CompressionManager() {
	instance = this;

	registerLZMACompression(this);
}

CompressionManager::~CompressionManager() {
	instance = NULL;
}

void CompressionManager::registerCompressedStreamRoutine(const std::string& _extension, CompressedStreamRoutine _compressRoutine,
	CompressedStreamRoutine _decompressRoutine) {
	if (!_extension.empty()) {
		osgDB::StringList extList;
		osgDB::split(osgDB::convertToLowerCase(_extension), extList);
		for (int i = 0, m = extList.size(); i < m; i++) {
			if (!extList[i].empty()) {
				extList[i] = "." + extList[i];
				if (_compressRoutine) compressRoutine[extList[i]] = _compressRoutine;
				if (_decompressRoutine) decompressRoutine[extList[i]] = _decompressRoutine;
			}
		}
	}
}

CompressedStreamRoutine CompressionManager::getCompressRoutine(const std::string& _fileName) const {
	for (std::map<std::string, CompressedStreamRoutine>::const_iterator
		it = compressRoutine.begin(); it != compressRoutine.end(); ++it) {
		const std::string& ext = it->first;
		if (_fileName.size() >= ext.size() &&
			osgDB::convertToLowerCase(_fileName.substr(_fileName.size() - ext.size())) == ext) {
			return it->second;
		}
	}
	return NULL;
}

CompressedStreamRoutine CompressionManager::getDecompressRoutine(const std::string& _fileName) const {
	for (std::map<std::string, CompressedStreamRoutine>::const_iterator
		it = decompressRoutine.begin(); it != decompressRoutine.end(); ++it) {
		const std::string& ext = it->first;
		if (_fileName.size() >= ext.size() &&
			osgDB::convertToLowerCase(_fileName.substr(_fileName.size() - ext.size())) == ext) {
			return it->second;
		}
	}
	return NULL;
}

std::istream* CompressionManager::openFileForRead(const std::string& _fileName) const {
	CompressedStreamRoutine routine = getDecompressRoutine(_fileName);

	std::istream *fin = new std::ifstream(_fileName.c_str(), std::ios_base::in | std::ios_base::binary);

	if (!(*fin)) {
		UTIL_WARN "Failed to open '" << _fileName << "'" << std::endl;
		delete fin;
		return NULL;
	}

	if (routine) {
		std::stringstream *ss = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

		if (!routine(*fin, *ss)) {
			UTIL_WARN "Failed to decompress '" << _fileName << "' as a compressed stream" << std::endl;
			delete fin;
			delete ss;
			return NULL;
		}

		delete fin;
		ss->seekg(0);
		return ss;
	} else {
		return fin;
	}
}

util::ostream* CompressionManager::openFileForWrite(const std::string& _fileName) const {
	CompressedStreamRoutine routine = getCompressRoutine(_fileName);

	std::ostream *fout = new std::ofstream(_fileName.c_str(), std::ios_base::out | std::ios_base::binary);

	if (!(*fout)) {
		UTIL_WARN "Failed to open '" << _fileName << "'" << std::endl;
		delete fout;
		return NULL;
	}

	util::ostream *ret = new util::ostream;

	if (routine) {
		ret->uncompressed = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
		ret->compressed = fout;
		ret->compressRoutine = routine;
	} else {
		ret->uncompressed = fout;
	}

	return ret;
}
