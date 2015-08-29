#pragma once

/** \file Various file types and operations.
*/

#include <stdio.h>
#include <iostream>
#include <vector>

/** Obtains the current value of the file position.
*/
inline std::istream::off_type u8ftellg(std::istream* file){
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	return file->tellg();
}

/** Reposition a stream.
*/
inline std::istream::off_type u8fseekg(std::istream* file, std::istream::off_type offset, int whence = SEEK_SET){
	file->seekg(offset, whence);
	return u8ftellg(file);
}

/** Obtains the current value of the file position.
*/
inline std::ostream::off_type u8ftellp(std::ostream* file){
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	return file->tellp();
}

/** Reposition a stream.
*/
inline std::ostream::off_type u8fseekp(std::ostream* file, std::ostream::off_type offset, int whence = SEEK_SET){
	file->seekp(offset, whence);
	return u8ftellp(file);
}

/** Get file size.
*/
inline std::ostream::off_type u8fsize(std::istream* file){
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	std::ostream::pos_type p = file->tellg();

	file->seekg(0, std::ios_base::end);
	std::ostream::off_type size = file->tellg();
	file->seekg(p);

	return size;
}

/** Read file.
*/
inline size_t u8fread(void* ptr,size_t size,std::istream* file){
	file->read((char*)ptr, size);
	return file->gcount();
}

/** Write file.
*/
inline size_t u8fwrite(const void* ptr,size_t size,std::ostream* file){
	file->write((const char*)ptr, size);
	return size; //???
}

/** Read a line of string from file, assuming the file is UTF-8 encoded.
\param buf String buffer.
\param count Size of buffer, including trailing zero.
\param file The file.
\return The string buffer or <em>NULL</em> (EOF or failed).
*/
char* u8fgets(char* buf, int count, std::istream* file);

/** Read a line of string from file, assuming the file is UTF-8 encoded.
\param[out] s The string.
\param file The file.
\return The string buffer or <em>NULL</em> (EOF or failed).
*/
const char* u8fgets(std::string& s,std::istream* file);

/** Write a UTF-8 string to file.
\return Number of bytes written.
*/
inline size_t u8fputs(const std::string& s,std::ostream* file){
	return u8fwrite(s.c_str(),s.size(),file);
}

/** Read a character from file.
\param file The file.
\return The character read or EOF.
*/
inline int u8fgetc(std::istream* file){
	unsigned char c;
	if(u8fread(&c,1,file)!=1) return EOF;
	return c;
}

/** Write a character to file.
\param ch The character.
\param file The file.
\return The character written or EOF.
*/
inline int u8fputc(int ch,std::ostream* file){
	return u8fwrite(&ch,1,file)==1?(int)(unsigned char)ch:EOF;
}
