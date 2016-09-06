#include "util_filesystem.h"

#include <string.h>
#ifdef WIN32
#include <windows.h>
#if !defined(_WIN32_IE) || _WIN32_IE<0x0600
#undef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#if !defined(_WIN32_WINNT) || _WIN32_WINNT<0x0500
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <shlobj.h>
#include <shlwapi.h>
#include <direct.h>
#ifdef _MSC_VER
#pragma comment(lib,"shlwapi.lib")
#endif
#else
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#ifdef ANDROID
#include <fstream>
#include "u8file.h"
#endif
#endif

#include <osgDB/ConvertUTF>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Serializer>

namespace util {

	void enumAllFiles(std::vector<FileInfo> ret, std::string path, const char* extension, bool enumFile, bool enumDir, bool containsPath) {
		// normalize path
		if (!path.empty()){
			char c = path[path.size() - 1];
			if (c != '/' && c != '\\') path += "\\";
			path = osgDB::convertFileNameToNativeStyle(path);
		}

		// check file extension
		osgDB::StringList extList;
		if (extension)
			osgDB::split(osgDB::convertToLowerCase(extension), extList);

		FileInfo fi;
		char mtime[128];

#ifdef WIN32
		WIN32_FIND_DATAW f;

		HANDLE h = NULL;
		{
			std::string s1 = path;
			if (extList.size() == 1) {
				s1 += "*.";
				s1 += extList[0];
			} else{
				s1 += "*";
			}
			std::wstring s1b = osgDB::convertUTF8toUTF16(s1);
			h = FindFirstFileW(s1b.c_str(), &f);
		}

		if (h == NULL || h == INVALID_HANDLE_VALUE) return;

		do {
			if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// skip if we don't want directories
				if (!enumDir) continue;

				// skip '.' and '..'
				if (f.cFileName[0] == '.' && (f.cFileName[1] == 0 || (f.cFileName[1] == '.' && f.cFileName[2] == 0))) continue;

				fi.name = osgDB::convertUTF16toUTF8(f.cFileName);
				fi.ext.clear();
				fi.size = 0;
				fi.isFolder = true;
			} else {
				// skip if we don't want files
				if (!enumFile) continue;

				fi.name = osgDB::convertUTF16toUTF8(f.cFileName);
				fi.ext = osgDB::getLowerCaseFileExtension(fi.name);

				// skip if we don't want this extension
				if (extList.size() >= 2) {
					bool b = true;
					for (size_t i = 0; i < extList.size(); i++) {
						if (fi.ext == extList[i]) {
							b = false;
							break;
						}
					}
					if (b) continue;
				}

				fi.size = (((long long)(f.nFileSizeHigh)) << 32) | (long long)(f.nFileSizeLow);
				fi.isFolder = false;
			}

			// get modify time
			FILETIME lft;
			SYSTEMTIME st;
			FileTimeToLocalFileTime(&f.ftLastWriteTime, &lft);
			FileTimeToSystemTime(&lft, &st);
			sprintf(mtime, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			fi.mtime = mtime;

			// add to list
			if (containsPath) fi.name = path + fi.name;
			ret.push_back(fi);
		} while (FindNextFileW(h, &f));

		FindClose(h);

		return;
#else
		DIR *pDir;
		struct dirent *pDirent;
		pDir = opendir(path.c_str());

		if (pDir == NULL){
#ifdef ANDROID
			//ad-hoc workaround
			std::ifstream f(path + "list.txt", std::ios::in | std::ios::binary);
			if (f) {
				std::string s;
				while (u8fgets(s, &f)){
					size_t lps = s.find_first_of("\r\n");
					if (lps != std::string::npos) s = s.substr(0, lps);
					if (s.empty()) continue;

					//trim
					lps = s.find_first_not_of(" \t");
					if (lps > 0) s = s.substr(lps);
					if (s.empty()) continue;

					lps = s.find_last_not_of(" \t");
					if (lps + 1 < s.size()) s = s.substr(0, lps + 1);

					if (s.empty()) continue;

					if (s[s.size() - 1] == '/' || s[s.size() - 1] == '\\') {
						// skip if we don't want directories
						if (!enumDir) continue;

						fi.name = s;
						fi.ext.clear();
						fi.size = 0;
						fi.isFolder = true;
					} else {
						// skip if we don't want files
						if (!enumFile) continue;

						fi.name = s;
						fi.ext = osgDB::getLowerCaseFileExtension(fi.name);

						// skip if we don't want this extension
						if (extList.size() >= 1) {
							bool b = true;
							for (size_t i = 0; i < extList.size(); i++) {
								if (fi.ext == extList[i]) {
									b = false;
									break;
								}
							}
							if (b) continue;
						}

						fi.size = 0; // it doesn't contain file size information
						fi.isFolder = false;
					}

					fi.mtime.clear(); // it doesn't contain modify time information

					// add to list
					if (containsPath) fi.name = path + fi.name;
					ret.push_back(fi);
				}
			}
#endif
			return;
		}

		while ((pDirent = readdir(pDir)) != NULL){
			// skip '.' and '..'
			if (pDirent->d_name[0] == '.' && (pDirent->d_name[1] == 0 || (pDirent->d_name[1] == '.' && pDirent->d_name[2] == 0))) continue;

			std::string s1 = path + pDirent->d_name;

			struct stat S_stat;
			lstat(s1.c_str(), &S_stat);

			if (S_ISDIR(S_stat.st_mode)) {
				// skip if we don't want directories
				if (!enumDir) continue;

				if (containsPath) fi.name = s1;
				else fi.name = pDirent->d_name;
				fi.ext.clear();
				fi.size = 0;
				fi.isFolder = true;
			} else {
				// skip if we don't want files
				if (!enumFile) continue;

				if (containsPath) fi.name = s1;
				else fi.name = pDirent->d_name;
				fi.ext = osgDB::getLowerCaseFileExtension(fi.name);

				// skip if we don't want this extension
				if (extList.size() >= 1) {
					bool b = true;
					for (size_t i = 0; i < extList.size(); i++) {
						if (fi.ext == extList[i]) {
							b = false;
							break;
						}
					}
					if (b) continue;
				}

				fi.size = S_stat.st_size;
				fi.isFolder = false;
			}

			// get modify time
			struct tm * timeinfo;
			timeinfo = localtime(&S_stat.st_mtime);
			strftime(mtime, sizeof(mtime), "%Y-%m-%d %H:%M:%S", timeinfo);
			fi.mtime = mtime;

			// add to list
			ret.push_back(fi);
		}

		closedir(pDir);

		return;
#endif
	}

}