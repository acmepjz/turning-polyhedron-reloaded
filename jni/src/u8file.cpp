#include "u8file.h"
#include <string.h>

char* u8fgets(char* buf, int count, std::istream* file){
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

const char* u8fgets(std::string& s,std::istream* file){
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

