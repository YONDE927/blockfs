#include "Sftp.h"

#define FROM "/home/yonde/Documents/研究室.txt"
#define DEST "/home/yonde/Documents/blockfs/build/sample"

void printstat(Stat &st);

int main(){
	sftp* p_connector = new sftp;
	Stat attribute;
	attribute = p_connector->getstat("/home/yonde/Documents/研究室.txt");
	printstat(attribute);
	std::list<Stat> attrs = p_connector->getdir("/home/yonde/Documents");
	std::list<Stat>::iterator itr;
	for(itr=attrs.begin();itr!=attrs.end();itr++){
		printstat(*itr);
	}
	p_connector->download(FROM, DEST);
	delete p_connector;
	return 0;
}

void printstat(Stat &st){
	std::cout << st.name << " " <<  st.size << " " << st.atime << std::endl;
}