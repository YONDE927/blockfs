#include "Sftp.h"

#define FROM "/home/yonde/Documents/研究室.txt"
#define DEST "/home/yonde/Documents/blockfs/build/sample"

void printstat(Stat &st);

int test_sftp();

int main(){
	if(test_sftp()==0){
		std::cout << "sftp clear" << std::endl;
	}
	return 0;
}

void printstat(Stat &st){
	std::cout << st.name << " " <<  st.st.st_size << " " << st.st.st_atime << std::endl;
}

int test_sftp(){
	sftp* p_connector = new sftp;
	Stat attribute;
	attribute = p_connector->getstat("/home/yonde/Documents/研究室.txt");
	printstat(attribute);
	std::list<Stat> attrs = p_connector->getdir("/home/yonde/Documents");
	std::list<Stat>::iterator itr;
	for(itr=attrs.begin();itr!=attrs.end();itr++){
		printstat(*itr);
	}
	//fulldownload
	p_connector->fulldownload(FROM, DEST);
	char buffer[8];
	//download
	p_connector->download(FROM,buffer,2,8);
	std::cout << buffer << std::endl;
	//upload
	char word[] = "Hello";
	p_connector->upload(FROM,word,5,5);
	delete p_connector;
	return 0;
}