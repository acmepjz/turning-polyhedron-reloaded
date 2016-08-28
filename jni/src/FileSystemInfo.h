/*!
	@file
	@author		Albert Semenov
	@date		09/2009
	@module
*/
#ifndef FILE_SYSTEM_INFO_H_
#define FILE_SYSTEM_INFO_H_

#include <MyGUI/MyGUI.h>

namespace common
{

	struct FileInfo
	{
		FileInfo(const std::wstring& _name, bool _folder) : name(_name), folder(_folder) { }
		std::wstring name;
		bool folder;
	};
	typedef std::vector<FileInfo> VectorFileInfo;

	std::wstring toLower(const std::wstring& _input);

	bool sortFiles(const common::FileInfo& left, const common::FileInfo& right);

	bool isAbsolutePath(const wchar_t* path);

	bool endWith(const std::wstring& _source, const std::wstring& _value);

	std::wstring concatenatePath(const std::wstring& _base, const std::wstring& _name);

	bool isReservedDir(const wchar_t* _fn);

	bool isParentDir(const wchar_t* _fn);

	void getSystemFileList(VectorFileInfo& _result, const std::wstring& _folder, const std::wstring& _mask, bool _sorted = true);

	std::wstring getSystemCurrentFolder();

	typedef std::vector<std::wstring> VectorWString;
	void scanFolder(VectorWString& _result, const std::wstring& _folder, bool _recursive, const std::wstring& _mask, bool _fullpath);

}

#endif // FILE_SYSTEM_INFO_H_
