#include "u8file.h"
#include <string.h>

IFile::~IFile(){
}

Sint64 IFile::fseek(Sint64 offset,int whence){
	return -1;
}

Sint64 IFile::ftell(){
	return -1;
}

Sint64 IFile::fsize(){
	if(m_size==-1){
		Sint64 oldPos=this->ftell();
		m_size=this->fseek(0,SEEK_END);
		if(oldPos!=-1) this->fseek(oldPos,SEEK_SET);
	}

	return m_size;
}

void IFile::fclose(){
}

size_t IFile::fread(void *ptr,size_t size){
	return 0;
}

size_t IFile::fwrite(const void *ptr,size_t size){
	return 0;
}

Sint64 u8file::fseek(Sint64 offset,int whence){
	return SDL_RWseek(m_file,offset,whence);
}

Sint64 u8file::ftell(){
	return SDL_RWtell(m_file);
}

Sint64 u8file::fsize(){
	if(m_size==-1){
		m_size=SDL_RWsize(m_file);
	}

	return m_size;
}

void u8file::fclose(){
	if(m_file){
		SDL_RWclose(m_file);
		m_file=NULL;
	}
}

size_t u8file::fread(void *ptr,size_t size){
	return SDL_RWread(m_file,ptr,1,size);
}

size_t u8file::fwrite(const void *ptr,size_t size){
	return SDL_RWwrite(m_file,ptr,1,size);
}

Sint64 MemoryFile::fseek(Sint64 offset,int whence){
	switch(whence){
	case SEEK_CUR: offset+=m_pos; break;
	case SEEK_END: offset+=m_data.size(); break;
	}

	if(offset<0) return -1;
	m_pos=offset;
	return offset;
}

Sint64 MemoryFile::ftell(){
	return m_pos;
}

Sint64 MemoryFile::fsize(){
	return m_data.size();
}

size_t MemoryFile::fread(void *ptr,size_t size){
	Sint64 sz=(Sint64)m_data.size()-m_pos;
	if(sz>(Sint64)size) sz=size;

	if(sz<=0) return 0;
	memcpy(ptr,&(m_data[m_pos]),(size_t)sz);
	m_pos+=sz;
	return (size_t)sz;
}

size_t MemoryFile::fwrite(const void *ptr,size_t size){
	Sint64 oldSize=(Sint64)m_data.size();

	if(m_pos+(Sint64)size>oldSize){
		m_data.resize(m_pos+(Sint64)size);
		if(m_pos>oldSize){
			memset(&(m_data[oldSize]),0,(size_t)(m_pos-oldSize));
		}
	}

	memcpy(&(m_data[m_pos]),ptr,size);
	m_pos+=size;
	return size;
}

char* u8fgets(char* buf, int count, IFile* file){
	for(int i=0;i<count-1;i++){
		if(u8fread(buf+i,1,file)==0){
			buf[i]=0;
			return (i>0)?buf:NULL;
		}
		if(buf[i]=='\n'){
			buf[i+1]=0;
			return buf;
		}
	}

	buf[count-1]=0;
	return buf;
}

const char* u8fgets(u8string& s,IFile* file){
	s.clear();

	if(file){
		char c;
		for(int i=0;;i++){
			if(u8fread(&c,1,file)==0) return (i>0)?s.c_str():NULL;
			s.push_back(c);
			if(c=='\n') return s.c_str();
		}
	}

	return NULL;
}

