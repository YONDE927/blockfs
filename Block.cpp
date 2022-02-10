#include "Block.hpp"
#include "Entry.hpp"
#include "Manager.hpp"

block::block(stdobj* parent,int _index):stdobj(parent){
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
	nread=((manager*)base)->p_sftp->download(((file*)parent)->path,data,offset,BLOCK_SIZE);
	//nread=p_sftp->download(*p_path,data,offset,BLOCK_SIZE);
	if(nread<=0){
		std::cerr << "sftp->download fail" << std::endl;
		return -1;
	}else{
		bsize = nread;
		return nread;
	}
}

int block::upload(){
	int offset,nwritten;
	offset=BLOCK_SIZE*index;
	//std::cout << "uploading " << data << std::endl;
	//std::cout << "size of block is " << strlen(data) << std::endl;
	nwritten=((manager*)base)->p_sftp->upload(((file*)parent)->path,data,offset,strlen(data));
	//nwritten=p_sftp->upload(*p_path,data,offset,strlen(data));
	if(nwritten<=0){
		std::cerr << "block::upload sftp->upload fail" << std::endl;
		return -1;
	}else{
		return nwritten;
	}
}

int block::bread(char* buf,int offset,int size,bool uptodate,int cachefd){
	int dsize,nread;
	BlockCache bc;
	//load
	if(data==NULL){
		if(uptodate & (((manager*)base)->p_cache->find_block(((file*)parent)->path,index,bc)==0)){
		//if(uptodate & (p_cache->find_block(*p_path,index,bc)==0)){
			this->lload(cachefd);
		}else{
			dsize = this->download();
			//cache
			//cache write
			if(this->ldown(cachefd)<0){
				std::cout << "block::bread block ldown failed" << std::endl;
			}
			//add cache
			((manager*)base)->p_cache->add_block(((file*)parent)->path,index);
			//p_cache->add_block(*p_path,index);
		}
	}
	
	//read
	if(offset>BLOCK_SIZE){
		std::cerr << "block::bread offset over BLOCK_SIZE" << std::endl;
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
	//std::cout << "block::bread block[" << index << "] is read " << nread << " bytes" << std::endl;
	return nread;
}

int block::bwrite(const char* buf,int offset,int size,bool uptodate,int cachefd){
	int nwritten,dsize;
	BlockCache bc;
	//load
	if(data==NULL){
		if(uptodate & (((manager*)base)->p_cache->find_block(((file*)parent)->path,index,bc)==0)){
		//if(uptodate & (p_cache->find_block(*p_path,index,bc)==0)){
			this->lload(cachefd);
		}else{
			dsize = this->download();
			//write cachefile later
			//add cache
			((manager*)base)->p_cache->add_block(((file*)parent)->path,index);
		}
	}
	//write
	if(offset>BLOCK_SIZE){
		std::cerr << "block::bwrite offset over BLOCK_SIZE" << std::endl;
		return -1;
	}
	if(size > (BLOCK_SIZE-offset)){
		memcpy(data+offset,buf,BLOCK_SIZE-offset);
		nwritten = BLOCK_SIZE-offset;
		bsize = BLOCK_SIZE;
	}else if(size >= 0){
		memcpy(data+offset,buf,size);
		nwritten = size;
		if((offset + size)>bsize){
			bsize = offset + size;
		}
	}else{
		nwritten = -1;
	}
	//cache write
	if(this->ldown(cachefd)<0){
		std::cout << "block::bwrite block ldown failed" << std::endl;
	}
	//sftp write
	this->upload();
	std::cout << "block::bwrite block[" << index << "] is writeen " << nwritten << " bytes" << std::endl;
	return nwritten;
}

//ブロックを所定のファイルから読み込む。
int block::lload(int fd){
	int offset,nread;
	if(data==NULL){
		data=new char[BLOCK_SIZE];
		memset(data,0,BLOCK_SIZE);
	}
	offset=BLOCK_SIZE*index;
	lseek(fd,offset,SEEK_SET);
	nread=read(fd,data,BLOCK_SIZE);
	if(nread<=0){
		std::cerr << "block::lload read cache fail" << std::endl;
		return -1;
	}else{
		bsize = nread;
		return nread;
	}
}
//ブロックを所定のファイルに書き込む。
int block::ldown(int fd){
	int offset,nwritten;
	std::cout << "block::ldown " << bsize << std::endl; 
	offset=BLOCK_SIZE*index;
	lseek(fd,offset,SEEK_SET);
	nwritten=write(fd,data,bsize);
	if(nwritten<=0){
		std::cerr << "block::ldown write to cache fail" << std::endl;
		return -1;
	}else{
		return nwritten;
	}
}
