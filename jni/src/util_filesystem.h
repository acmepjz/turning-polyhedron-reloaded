#pragma once

#include <string>
#include <vector>

namespace util {

	/** Contains basic information for a file.
	*/
	struct FileInfo {
		std::string name; /**< the file name */
		std::string ext; /**< the file extension */
		std::string mtime; /**< the modify time */
		long long size; /**< the file size */
		bool isFolder; /**< is it a folder */
	};

	/** A helper to compare FileInfo using std::sort.
	*/
	struct FileInfoComparer {
		static const char SortByName = 0;
		static const char SortByExtension = 1;
		static const char SortByTime = 2;
		static const char SortBySize = 3;
		FileInfoComparer(char sortKey_ = SortByName, bool descending_ = false) : sortKey(sortKey_), descending(descending_) {}

		/** Used to std::sort FileInfo array */
		bool operator() (const FileInfo& f1, const FileInfo& f2) const {
			return compare(f1, f2) < 0;
		}

		/** Used to std::sort FileInfo* array */
		bool operator() (const FileInfo* f1, const FileInfo* f2) const {
			return compare(*f1, *f2) < 0;
		}

		/** Used to qsort FileInfo* array */
		static FileInfoComparer staticComparer;

		/** Used to qsort FileInfo* array */
		static int staticCompare(const void* f1, const void* f2) {
			return staticComparer.compare(**(FileInfo**)f1, **(FileInfo**)f2);
		}

		//! generic compare function
		int compare(const FileInfo& f1, const FileInfo& f2) const {
			return compare(sortKey, descending, f1, f2);
		}

		//! generic compare function (static version)
		static int compare(char sortKey_, bool descending_, const FileInfo& f1, const FileInfo& f2);
	private:
		char sortKey;
		bool descending;
	};

	/** Method that returns a list of all the files in a given directory.
	\param ret Return a vector containing the names of the files.
	\param path The path to list the files of.
	\param extension The extension the files must have, separated by " ". NOTE: this does not apply to directories.
	\param enumFile List all files.
	\param enumDir List all directory.
	\param containsPath Specifies if the return file name should contains path.
	*/
	void enumAllFiles(std::vector<FileInfo>& ret, std::string path, const char* extension = NULL, bool enumFile = true, bool enumDir = false, bool containsPath = false);

}
