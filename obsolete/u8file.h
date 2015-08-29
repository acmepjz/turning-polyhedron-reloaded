#pragma once

/** \file Various file types and operations.
*/

#include "UTF8-16.h"
#include <stdio.h>
#include <SDL_rwops.h>
#include <vector>

/** An interface for (possibly read only) file (smaller than 2GB) access.
\note Some implementations store their file data in corresponding file
manager, so after closing the file manager these files become invalid,
and contain invalid pointer.
\warning fclose() must be called before deleting file object.
*/

class IFile{
public:
	IFile():m_size(-1){}
	virtual ~IFile();

	/** Reposition a stream.
	\param offset Offset relative to <em>whence</em>
	\param whence <em>SEEK_SET</em> start of the file,
	<em>SEEK_CUR</em> the current position indicator,
	<em>SEEK_END</em> end of the file.
	\return The final offset in the data stream, or -1 if error or unimplemented.
	*/
	virtual Sint64 fseek(Sint64 offset,int whence=SEEK_SET);

	/** Obtains the current value of the file position.
	\return The offset in the data stream, or -1 if error or unimplemented.
	*/
	virtual Sint64 ftell();

	/** Get file size.
	\return The size of the file, or -1 if error or unimplemented.
	*/
	virtual Sint64 fsize();

	/** Close the file.
	\note In some implementations, this function does nothing.
	*/
	virtual void fclose();

	/** Read file.
	\return Size of data read (in bytes), or 0 if error or unimplemented.
	*/
	virtual size_t fread(void *ptr,size_t size);

	/** Write file.
	\return Size of data written (in bytes), or 0 if error or unimplemented.
	*/
	virtual size_t fwrite(const void *ptr,size_t size);
public:
	/** (Internal use only) The file size or -1 means file size still unknown. */
	Sint64 m_size;
};

/** An implementation of IFile to read/write disk file, memory or other file types supported by SDL.
*/

class u8file:public IFile{
public:
	u8file():m_file(NULL){}

	Sint64 fseek(Sint64 offset,int whence=SEEK_SET) override;
	Sint64 ftell() override;
	Sint64 fsize() override;
	void fclose() override;
	size_t fread(void *ptr,size_t size) override;
	size_t fwrite(const void *ptr,size_t size) override;
public:
	/** (Internal use only) The SDL_RWops. */
	SDL_RWops* m_file;
};

/** Yet another memory file.
*/

class MemoryFile:public IFile{
public:
	MemoryFile():m_pos(0){}

	Sint64 fseek(Sint64 offset,int whence=SEEK_SET) override;
	Sint64 ftell() override;
	Sint64 fsize() override;
	size_t fread(void *ptr,size_t size) override;
	size_t fwrite(const void *ptr,size_t size) override;
public:
	/** The data. */
	std::vector<unsigned char> m_data;
	/** The position. */
	Sint64 m_pos;
};

/** Create a file from SDL_RWops.
\param f The SDL_RWops.
*/
inline u8file *u8fopen(SDL_RWops *f){
	if(f){
		u8file *file=new u8file;
		file->m_file=f;
		return file;
	}
	return NULL;
}

/** Load a file from disk.
\param filename File name in UTF-8 encoded string.
\param mode Mode.
*/
inline u8file *u8fopen(const char* filename,const char* mode){
	return u8fopen(SDL_RWFromFile(filename,mode));
}

/** Load a read/write file from memory.
\param mem The memory.
\param size The size.
*/
inline u8file* u8fopenMem(void *mem, int size){
	return u8fopen(SDL_RWFromMem(mem,size));
}

/** Load a read-only file from memory.
\param mem The memory.
\param size The size.
*/
inline u8file* u8fopenConstMem(const void *mem, int size){
	return u8fopen(SDL_RWFromConstMem(mem,size));
}

/** Reposition a stream.
\sa IFile::fseek
*/
inline Sint64 u8fseek(IFile* file,Sint64 offset,int whence=SEEK_SET){
	return file->fseek(offset,whence);
}

/** Obtains the current value of the file position.
\sa IFile::ftell
*/
inline Sint64 u8ftell(IFile* file){
	return file->ftell();
}

/** Get file size.
\sa IFile::fsize
*/
inline Sint64 u8fsize(IFile* file){
	return file->fsize();
}

/** Read file.
\sa IFile::fread
*/
inline size_t u8fread(void* ptr,size_t size,IFile* file){
	return file->fread(ptr,size);
}

/** Write file.
\sa IFile::fwrite
*/
inline size_t u8fwrite(const void* ptr,size_t size,IFile* file){
	return file->fwrite(ptr,size);
}

/** Close the file and delete file object.
\sa IFile::fclose
*/
inline void u8fclose(IFile* file){
	file->fclose();
	delete file;
}

/** Read a line of string from file, assuming the file is UTF-8 encoded.
\param buf String buffer.
\param count Size of buffer, including trailing zero.
\param file The file.
\return The string buffer or <em>NULL</em> (EOF or failed).
*/
char* u8fgets(char* buf, int count, IFile* file);

/** Read a line of string from file, assuming the file is UTF-8 encoded.
\param[out] s The string.
\param file The file.
\return The string buffer or <em>NULL</em> (EOF or failed).
*/
const char* u8fgets(u8string& s,IFile* file);

/** Write a UTF-8 string to file.
\return Number of bytes written.
*/
inline size_t u8fputs(const u8string& s,IFile* file){
	return u8fwrite(s.c_str(),s.size(),file);
}

/** Read a character from file.
\param file The file.
\return The character read or EOF.
*/
inline int u8fgetc(IFile* file){
	unsigned char c;
	if(u8fread(&c,1,file)!=1) return EOF;
	return c;
}

/** Write a character to file.
\param ch The character.
\param file The file.
\return The character written or EOF.
*/
inline int u8fputc(int ch,IFile* file){
	return u8fwrite(&ch,1,file)==1?(int)(unsigned char)ch:EOF;
}
