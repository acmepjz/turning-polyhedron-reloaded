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
#endif

#include <osgDB/ConvertUTF>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Serializer>

namespace util {

	void enumAllFiles(std::vector<FileInfo> ret, std::string path, const char* extension, bool enumFile, bool enumDir, bool containsPath) {
#ifdef WIN32
		WIN32_FIND_DATAW f;

		// normalize path
		if (!path.empty()){
			char c = path[path.size() - 1];
			if (c != '/' && c != '\\') path += "\\";
		}

		// check file extension
		osgDB::StringList extList;
		if (extension)
			osgDB::split(osgDB::convertToLowerCase(extension), extList);

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

		FileInfo fi;
		char mtime[128];

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
		//TODO: this code is heavily outdated
		int len = 0;
		if (extension != NULL && *extension) len = strlen(extension);
		if (!path.empty()){
			char c = path[path.size() - 1];
			if (c != '/'&&c != '\\') path += "/";
		}
		DIR *pDir;
		struct dirent *pDirent;
		pDir = opendir(path.c_str());
		if (pDir == NULL){
#ifdef ANDROID
			//ad-hoc workaround
			u8file *f = u8fopen((path + "list.txt").c_str(), "rb");
			if (f){
				u8string s;
				while (u8fgets2(s, f)){
					u8string::size_type lps = s.find_first_of("\r\n");
					if (lps != u8string::npos) s = s.substr(0, lps);
					if (s.empty()) continue;

					//trim
					lps = s.find_first_not_of(" \t");
					if (lps>0) s = s.substr(lps);
					if (s.empty()) continue;

					lps = s.find_last_not_of(" \t");
					if (lps + 1<s.size()) s = s.substr(0, lps + 1);

					if (s.empty() || s[s.size() - 1] == '/') continue;

					if (len>0){
						if ((int)s.size()<len + 1) continue;
						if (s[s.size() - len - 1] != '.') continue;
						if (strcasecmp(&s[s.size() - len], extension)) continue;
					}

					if (containsPath){
						v.push_back(path + s);
					} else{
						v.push_back(s);
					}
				}
				u8fclose(f);
			}
#endif
			return v;
		}
		while ((pDirent = readdir(pDir)) != NULL){
			if (pDirent->d_name[0] == '.'){
				if (pDirent->d_name[1] == 0 ||
					(pDirent->d_name[1] == '.'&&pDirent->d_name[2] == 0)) continue;
			}
			u8string s1 = path + pDirent->d_name;
			struct stat S_stat;
			lstat(s1.c_str(), &S_stat);
			if (!S_ISDIR(S_stat.st_mode)){
				if (len>0){
					if ((int)s1.size()<len + 1) continue;
					if (s1[s1.size() - len - 1] != '.') continue;
					if (strcasecmp(&s1[s1.size() - len], extension)) continue;
				}

				if (containsPath){
					v.push_back(s1);
				} else{
					v.push_back(u8string(pDirent->d_name));
				}
			}
		}
		closedir(pDir);
		return v;
#endif
	}

}