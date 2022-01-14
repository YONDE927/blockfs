#include "Sftp.h"
#include "Block.h"

#define DEST "/home/yonde/Documents/blockfs/build/sample"
#define PATH "hello/hello.c"

void printstat(Stat &st);

int test_sftp();
int test_block();
int test_entry();
int test_attribute();
int test_file();



int main(){
	if(test_sftp()==0){
		std::cout << "sftp clear" << std::endl;
	}
	if(test_block()==0){
		std::cout << "block clear" << std::endl;
	}
	return 0;
}

void printstat(Stat &st){
	std::cout << st.path << " " << st.name << " " <<  st.st.st_size << " " << st.st.st_atime << std::endl;
}

int test_sftp(){
	sftp* p_connector = new sftp;
	Stat attribute;
	attribute = p_connector->getstat(PATH);
	printstat(attribute);
	std::list<Stat> attrs = p_connector->getdir("hello");
	std::list<Stat>::iterator itr;
	for(itr=attrs.begin();itr!=attrs.end();itr++){
		printstat(*itr);
	}
	//fulldownload
	p_connector->fulldownload(PATH, DEST);
	char buffer[8];
	//download
	p_connector->download(PATH,buffer,2,8);
	std::cout << buffer << std::endl;
	//upload
	char word[] = "Hello";
	p_connector->upload(PATH,word,5,5);
	delete p_connector;
	return 0;
}

int test_block(){
	sftp* p_connector = new sftp;
	std::string path = PATH;
	int nread;
	char buf[512];

	memset(buf,0,sizeof(buf));
	block *b1 = new block(p_connector,&path,0);
	if(b1==NULL){
		return -1;
	}
	if(b1->download()<0){
		return -1;
	}
	if(b1->read(buf,0,512)<0){
		return -1;
	}
	std::cout << buf << std::endl;
	delete b1;
	delete p_connector;
	return 0;
}