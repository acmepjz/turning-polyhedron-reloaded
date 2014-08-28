#include "FileSystem.h"
#include "u8file.h"

#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <direct.h>
#pragma comment(lib,"shlwapi.lib")
#else
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <SDL.h>

using namespace std;

u8string externalStoragePath;

std::vector<u8string> enumAllFiles(u8string path,const char* extension,bool containsPath){
	vector<u8string> v;
#ifdef WIN32
	WIN32_FIND_DATAW f;

	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/' && c!='\\') path+="\\";
	}

	HANDLE h=NULL;
	{
		u8string s1=path;
		if(extension!=NULL && *extension){
			s1+="*.";
			s1+=extension;
		}else{
			s1+="*";
		}
		u16string s1b=toUTF16(s1);
		h=FindFirstFileW((LPCWSTR)s1b.c_str(),&f);
	}

	if(h==NULL || h==INVALID_HANDLE_VALUE) return v;

	do{
		if(!(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			u8string s2=toUTF8((const unsigned short*)f.cFileName);
			if(containsPath){
				v.push_back(path+s2);
			}else{
				v.push_back(s2);
			}
		}
	}while(FindNextFileW(h,&f));

	FindClose(h);

	return v;
#else
	int len=0;
	if(extension!=NULL && *extension) len=strlen(extension);
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="/";
	}
	DIR *pDir;
	struct dirent *pDirent;
	pDir=opendir(path.c_str());
	if(pDir==NULL){
#ifdef ANDROID
		//ad-hoc workaround
		u8file *f=u8fopen((path+"list.txt").c_str(),"rb");
		if(f){
			u8string s;
			while(u8fgets2(s,f)){
				u8string::size_type lps=s.find_first_of("\r\n");
				if(lps!=u8string::npos) s=s.substr(0,lps);
				if(s.empty()) continue;

				//trim
				lps=s.find_first_not_of(" \t");
				if(lps>0) s=s.substr(lps);
				if(s.empty()) continue;

				lps=s.find_last_not_of(" \t");
				if(lps+1<s.size()) s=s.substr(0,lps+1);

				if(s.empty() || s[s.size()-1]=='/') continue;

				if(len>0){
					if((int)s.size()<len+1) continue;
					if(s[s.size()-len-1]!='.') continue;
					if(strcasecmp(&s[s.size()-len],extension)) continue;
				}

				if(containsPath){
					v.push_back(path+s);
				}else{
					v.push_back(s);
				}
			}
			u8fclose(f);
		}
#endif
		return v;
	}
	while((pDirent=readdir(pDir))!=NULL){
		if(pDirent->d_name[0]=='.'){
			if(pDirent->d_name[1]==0||
				(pDirent->d_name[1]=='.'&&pDirent->d_name[2]==0)) continue;
		}
		string s1=path+pDirent->d_name;
		struct stat S_stat;
		lstat(s1.c_str(),&S_stat);
		if(!S_ISDIR(S_stat.st_mode)){
			if(len>0){
				if((int)s1.size()<len+1) continue;
				if(s1[s1.size()-len-1]!='.') continue;
				if(strcasecmp(&s1[s1.size()-len],extension)) continue;
			}

			if(containsPath){
				v.push_back(s1);
			}else{
				v.push_back(string(pDirent->d_name));
			}
		}
	}
	closedir(pDir);
	return v;
#endif
}

std::vector<u8string> enumAllDirs(u8string path,bool containsPath){
	vector<u8string> v;
#ifdef WIN32
	WIN32_FIND_DATAW f;

	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/' && c!='\\') path+="\\";
	}

	HANDLE h=NULL;
	{
		u16string s1b=toUTF16(path);
		s1b.push_back('*');
		h=FindFirstFileW((LPCWSTR)s1b.c_str(),&f);
	}

	if(h==NULL || h==INVALID_HANDLE_VALUE) return v;

	do{
		// skip '.' and '..' and hidden folders
		if(f.cFileName[0]=='.'){
			/*if(f.cFileName[1]==0||
				(f.cFileName[1]=='.'&&f.cFileName[2]==0))*/ continue;
		}
		if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			u8string s2=toUTF8((const unsigned short*)f.cFileName);
			if(containsPath){
				v.push_back(path+s2);
			}else{
				v.push_back(s2);
			}
		}
	}while(FindNextFileW(h,&f));

	FindClose(h);

	return v;
#else
	if(!path.empty()){
		char c=path[path.size()-1];
		if(c!='/'&&c!='\\') path+="/";
	}
	DIR *pDir;
	struct dirent *pDirent;
	pDir=opendir(path.c_str());
	if(pDir==NULL){
#ifdef ANDROID
		//ad-hoc workaround
		u8file *f=u8fopen((path+"list.txt").c_str(),"rb");
		if(f){
			u8string s;
			while(u8fgets2(s,f)){
				u8string::size_type lps=s.find_first_of("\r\n");
				if(lps!=u8string::npos) s=s.substr(0,lps);
				if(s.empty()) continue;

				//trim
				lps=s.find_first_not_of(" \t");
				if(lps>0) s=s.substr(lps);
				if(s.empty()) continue;

				lps=s.find_last_not_of(" \t");
				if(lps+1<s.size()) s=s.substr(0,lps+1);

				if(s.size()<2 || s[s.size()-1]!='/') continue;
				s=s.substr(0,s.size()-1);

				if(containsPath){
					v.push_back(path+s);
				}else{
					v.push_back(s);
				}
			}
			u8fclose(f);
		}
#endif
		return v;
	}
	while((pDirent=readdir(pDir))!=NULL){
		if(pDirent->d_name[0]=='.'){
			if(pDirent->d_name[1]==0||
				(pDirent->d_name[1]=='.'&&pDirent->d_name[2]==0)) continue;
		}
		string s1=path+pDirent->d_name;
		struct stat S_stat;
		lstat(s1.c_str(),&S_stat);
		if(S_ISDIR(S_stat.st_mode)){
			//Skip hidden folders.
			s1=string(pDirent->d_name);
			if(s1.find('.')==0) continue;
			
			//Add result to vector.
			if(containsPath){
				v.push_back(path+pDirent->d_name);
			}else{
				v.push_back(s1);
			}
		}
	}
	closedir(pDir);
	return v;
#endif
}

void initPaths(){
#if defined(ANDROID)
	externalStoragePath=SDL_AndroidGetExternalStoragePath();
#elif defined(WIN32)
	const int size=65536;
	wchar_t *s=new wchar_t[size];
	SHGetSpecialFolderPathW(NULL,s,CSIDL_PERSONAL,1);
	externalStoragePath=toUTF8((const unsigned short*)s)+"/My Games/Turning Polyhedron Reloaded";
	delete[] s;
#else
	const char *env=getenv("HOME");
	if(env==NULL) externalStoragePath="local";
	else externalStoragePath=u8string(env)+"/.TurningPolyhedronReloaded";
#endif
	if(externalStoragePath.empty()) return;

	//Create subfolders.
	createDirectory(externalStoragePath);
	createDirectory(externalStoragePath+"/levels");
}

bool createDirectory(const u8string& path){
#ifdef WIN32
	const int size=65536;
	wchar_t *s0=new wchar_t[size],*s=new wchar_t[size];

	GetCurrentDirectoryW(size,s0);
	PathCombineW(s,s0,(LPCWSTR)toUTF16(path).c_str());

	for(int i=0;i<size;i++){
		if(s[i]=='\0') break;
		else if(s[i]=='/') s[i]='\\';
	}

	bool ret=(SHCreateDirectoryExW(NULL,s,NULL)!=0);

	delete[] s0;
	delete[] s;

	return ret;
#else
	return mkdir(path.c_str(),0777)==0;
#endif
}
