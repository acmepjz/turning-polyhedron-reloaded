#pragma once

#include <osg/Referenced>
#include <vector>
#include <string>
#include <map>
#include <istream>
#include <ostream>

class CompressionManager;

typedef bool(*CompressedStreamRoutine)(std::istream& fin, std::ostream& fout);

namespace util {
	class ostream {
	public:
		friend class CompressionManager;

		ostream();
		~ostream();

		std::ostream& get() { return *uncompressed; }

	private:
		ostream(const ostream& other); // no copy constructor

		std::ostream *uncompressed;
		std::ostream *compressed;
		CompressedStreamRoutine compressRoutine;
	};
}

class CompressionManager : public osg::Referenced {
protected:
	~CompressionManager();

public:
	CompressionManager();

	/** register compress/decompress routine for a type of compressed stream
	\param _extension The extension, separated by " " and omit the first dot, e.g. "lzma xz".
	\param _compressRoutine The compress routine. can be NULL.
	\param _decompressRoutine The decompress routine. can be NULL.
	*/
	void registerCompressedStreamRoutine(const std::string& _extension, CompressedStreamRoutine _compressRoutine,
		CompressedStreamRoutine _decompressRoutine);

	/** get the compress routine for compressed stream */
	CompressedStreamRoutine getCompressRoutine(const std::string& _fileName) const;

	/** get the decompress routine for compressed stream */
	CompressedStreamRoutine getDecompressRoutine(const std::string& _fileName) const;

	/** open a file for read,
	can be uncompressed file, compressed stream,
	or a file in (compressed) archive (to be implemented). */
	std::istream* openFileForRead(const std::string& _fileName) const;

	/** open a file for write,
	can be uncompressed file, or compressed stream. */
	util::ostream* openFileForWrite(const std::string& _fileName) const;

	static CompressionManager *instance; //!< the global compression (and file) manager

private:
	std::map<std::string, CompressedStreamRoutine> compressRoutine;
	std::map<std::string, CompressedStreamRoutine> decompressRoutine;
};
