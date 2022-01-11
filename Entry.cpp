#include "Entry.h"

attribute::attribute(sftp* _p_sftp,std::string* _path){
	p_sftp = _p_sftp;
	path = _path;
	st = new struct stat;
}

attribute::attribute(sftp* _p_sftp,std::string* _path,struct stat &_st){
	p_sftp = _p_sftp;
	path = _path;
	st = new struct stat;
	*st = _st;
}

attribute::~attribute(){
	delete st;
}

int attribute::download(){
	Stat stbuf;
	stbuf = p_sftp->getstat(*path);
	*st = stbuf.st;
	return 0;
}

struct stat* attribute::getattr(){
	return st;
}