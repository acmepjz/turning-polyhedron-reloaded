#pragma once

#include <string>
#include <vector>

namespace util {

	struct FileInfo {
		std::string name; /**< the file name */
		std::string ext; /**< the file extension */
		std::string mtime; /**< the modify time */
		long long size; /**< the file size */
		bool isFolder; /**< is it a folder */
	};

	/** Method that returns a list of all the files in a given directory.
	\param ret Return a vector containing the names of the files.
	\param path The path to list the files of.
	\param extension The extension the files must have, separated by " ".
	\param enumFile List all files.
	\param enumDir List all directory.
	\param containsPath Specifies if the return file name should contains path.
	*/
	void enumAllFiles(std::vector<FileInfo> ret, std::string path, const char* extension = NULL, bool enumFile = true, bool enumDir = false, bool containsPath = false);

}
