#include "Block.h"

block::block(sftp* _p_sftp,std::string *_p_path,int _index){
	p_sftp=_p_sftp;
	p_path=_p_path;
	index=_index;
	data=NULL;
}

block::~block(){
	delete[] data;
}

int block::download(){
	int offset,nread;
	if(data==NULL){
		data=new char[BLOCK_SIZE];
		memset(data,0,BLOCK_SIZE);
	}
	offset=BLOCK_SIZE*index;
	nread=p_sftp->download(*p_path,data,offset,BLOCK_SIZE);
	if(nread<=0){
		std::cerr << "sftp->download fail" << std::endl;
		return -1;
	}else{
		isfull=true;
		return nread;
	}
}

int block::upload(){
	int offset,nwritten;
	offset=BLOCK_SIZE*index;
	nwritten=p_sftp->upload(*p_path,data,offset,sizeof(data));
	if(nwritten<=0){
		std::cerr << "sftp->upload fail" << std::endl;
		return -1;
	}else{
		return nwritten;
	}
}

int block::read(char* buf,int offset,int size){
	int dsize;
	if(data==NULL){
		dsize = this->download();
	}
	if(offset>BLOCK_SIZE){
		std::cerr << "offset over BLOCK_SIZE" << std::endl;
		return -1;
	}
	memcpy(buf,data+offset,size);
	return size;
}

int block::write(char* buf,int offset,int size){
	if(data==NULL){
		this->download();
	}
	if(offset>BLOCK_SIZE){
		std::cerr << "offset over BLOCK_SIZE" << std::endl;
		return -1;
	}
	memcpy(data+offset,buf,size);
	this->upload();
	return size;
}