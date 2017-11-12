#pragma once

/** \file Various file types and operations.
*/

#include <stdio.h>
#include <iostream>

/** Obtains the current value of the file position.
*/
inline std::istream::off_type u8ftellg(std::istream* file){
	if (file == NULL) return 0;
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	return file->tellg();
}

/** Reposition a stream.
*/
inline std::istream::off_type u8fseekg(std::istream* file, std::istream::off_type offset, int whence = SEEK_SET){
	if (file == NULL) return 0;
	file->seekg(offset, whence);
	return u8ftellg(file);
}

/** Obtains the current value of the file position.
*/
inline std::ostream::off_type u8ftellp(std::ostream* file){
	if (file == NULL) return 0;
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	return file->tellp();
}

/** Reposition a stream.
*/
inline std::ostream::off_type u8fseekp(std::ostream* file, std::ostream::off_type offset, int whence = SEEK_SET){
	if (file == NULL) return 0;
	file->seekp(offset, whence);
	return u8ftellp(file);
}

/** Get file size.
*/
inline std::ostream::off_type u8fsize(std::istream* file){
	if (file == NULL) return 0;
	if (file->eof() && file->fail() && !file->bad()) file->clear();
	std::ostream::pos_type p = file->tellg();

	file->seekg(0, std::ios_base::end);
	std::ostream::off_type size = file->tellg();
	file->seekg(p);

	return size;
}

/** Read file.
*/
inline std::streamsize u8fread(void* ptr, std::streamsize size, std::istream* file){
	if (file == NULL) return 0;
	file->read((char*)ptr, size);
	return file->gcount();
}

/** Write file.
*/
inline std::streamsize u8fwrite(const void* ptr, std::streamsize size, std::ostream* file){
	if (file == NULL) return 0;
	file->write((const char*)ptr, size);
	return size; //???
}

template <class T,bool BE>
inline T u8freadInteger(std::istream* file) {
	const int SIZE = sizeof(T);
	if (file == NULL) return 0;
	unsigned char d[SIZE] = {};
	file->read((char*)d, SIZE);
	T result = 0;
	for (int i = 0; i < SIZE; i++) {
		result |= ((T)d[i]) << ((BE ? (SIZE - 1 - i) : i) * 8);
	}
	return result;
}

/** read 16-bit little endian number */
inline unsigned short u8freadLE16(std::istream* file) {
	return u8freadInteger<unsigned short, false>(file);
}

/** read 16-bit big endian number */
inline unsigned short u8freadBE16(std::istream* file) {
	return u8freadInteger<unsigned short, true>(file);
}

/** read 32-bit little endian number */
inline unsigned int u8freadLE32(std::istream* file) {
	return u8freadInteger<unsigned int, false>(file);
}

/** read 32-bit big endian number */
inline unsigned int u8freadBE32(std::istream* file) {
	return u8freadInteger<unsigned int, true>(file);
}

/** read 64-bit little endian number */
inline unsigned long long u8freadLE64(std::istream* file) {
	return u8freadInteger<unsigned long long, false>(file);
}

/** read 64-bit big endian number */
inline unsigned long long u8freadBE64(std::istream* file) {
	return u8freadInteger<unsigned long long, true>(file);
}

template <class T, bool BE>
inline void u8fwriteInteger(std::ostream* file, T number) {
	const int SIZE = sizeof(T);
	if (file == NULL) return;
	unsigned char d[SIZE];
	for (int i = 0; i < SIZE; i++) {
		d[i] = (unsigned char)(number >> ((BE ? (SIZE - 1 - i) : i) * 8));
	}
	file->write((char*)d, SIZE);
}

/** write 16-bit little endian number */
inline void u8fwriteLE16(std::ostream* file, unsigned short number) {
	u8fwriteInteger<unsigned short, false>(file, number);
}

/** write 16-bit big endian number */
inline void u8fwriteBE16(std::ostream* file, unsigned short number) {
	u8fwriteInteger<unsigned short, true>(file, number);
}

/** write 32-bit little endian number */
inline void u8fwriteLE32(std::ostream* file, unsigned int number) {
	u8fwriteInteger<unsigned int, false>(file, number);
}

/** write 32-bit big endian number */
inline void u8fwriteBE32(std::ostream* file, unsigned int number) {
	u8fwriteInteger<unsigned int, true>(file, number);
}

/** write 64-bit little endian number */
inline void u8fwriteLE64(std::ostream* file, unsigned long long number) {
	u8fwriteInteger<unsigned long long, false>(file, number);
}

/** write 64-bit big endian number */
inline void u8fwriteBE64(std::ostream* file, unsigned long long number) {
	u8fwriteInteger<unsigned long long, true>(file, number);
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
inline std::streamsize u8fputs(const std::string& s, std::ostream* file){
	if (file == NULL) return 0;
	return u8fwrite(s.c_str(), s.size(), file);
}

/** Read a character from file.
\param file The file.
\return The character read or EOF.
*/
inline int u8fgetc(std::istream* file){
	unsigned char c;
	if (u8fread(&c, 1, file) != 1) return EOF;
	return c;
}

/** Write a character to file.
\param ch The character.
\param file The file.
\return The character written or EOF.
*/
inline int u8fputc(int ch,std::ostream* file){
	return u8fwrite(&ch, 1, file) == 1 ? (int)(unsigned char)ch : EOF;
}
