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

//sftpとともにcacheのロードもここに加えるか？
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

	std::cout << "uploading " << data << std::endl;
	std::cout << "size of block is " << strlen(data) << std::endl;
	nwritten=p_sftp->upload(*p_path,data,offset,strlen(data));
	if(nwritten<=0){
		std::cerr << "sftp->upload fail" << std::endl;
		return -1;
	}else{
		return nwritten;
	}
}

int block::bread(char* buf,int offset,int size){
	int dsize,nread;
	if(data==NULL){
		dsize = this->download();
	}
	if(offset>BLOCK_SIZE){
		std::cerr << "offset over BLOCK_SIZE" << std::endl;
		return -1;
	}
	if(size > (BLOCK_SIZE-offset)){
		memcpy(buf,data+offset,BLOCK_SIZE-offset);
		nread= BLOCK_SIZE-offset;
	}else if(size >= 0){
		memcpy(buf,data+offset,size);
		nread= size;
	}else{
		nread= -1;
	}
	std::cout << "block [" << index << "] is read " << nread << " bytes" << std::endl;
	return nread;
}

int block::bwrite(const char* buf,int offset,int size){
	int nwritten;
	if(data==NULL){
		this->download();
	}
	if(offset>BLOCK_SIZE){
		std::cerr << "offset over BLOCK_SIZE" << std::endl;
		return -1;
	}
	if(size > (BLOCK_SIZE-offset)){
		memcpy(data+offset,buf,BLOCK_SIZE-offset);
		nwritten = BLOCK_SIZE-offset;
	}else if(size >= 0){
		memcpy(data+offset,buf,size);
		nwritten = size;
	}else{
		nwritten = -1;
	}
	this->upload();
	std::cout << "block [" << index << "] is writeen " << nwritten << " bytes" << std::endl;
	return nwritten;
}

//ブロックを所定のファイルから読み込む。
int block::lload(){
	int offset,nread;
	if(data==NULL){
		data=new char[BLOCK_SIZE];
		memset(data,0,BLOCK_SIZE);
	}
	offset=BLOCK_SIZE*index;
	lseek(cache_fd,offset,SEEK_SET);
	nread=read(cache_fd,data,BLOCK_SIZE);
	if(nread<=0){
		std::cerr << "read cache fail" << std::endl;
		return -1;
	}else{
		isfull=true;
		return nread;
	}
}
//ブロックを所定のファイルに書き込む。
int block::ldown(){
	int offset,nwritten;
	offset=BLOCK_SIZE*index;
	lseek(cache_fd,offset,SEEK_SET);
	nwritten=write(cache_fd,data,BLOCK_SIZE);
	if(nwritten<=0){
		std::cerr << "write to cache fail" << std::endl;
		return -1;
	}else{
		isfull=true;
		return nwritten;
	}
}