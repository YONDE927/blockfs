#include "Sftp.h"
#include "Block.h"
#include "Entry.h"
#include "Manager.h"

#define DEST "/home/yonde/Documents/blockfs/build/sample"
#define PATH "/sample"
#define PATH2 "/sample.d"
#define PATH3 "hello/whale.txt"

void printstat(Stat &st);

int test_sftp();
int test_block();
int test_entry();
int test_manager();



int main(){
	if(test_sftp()==0){
		std::cout << "sftp clear" << std::endl;
	}
	if(test_block()==0){
		std::cout << "block clear" << std::endl;
	}
	if(test_entry()==0){
		std::cout << "entry clear" << std::endl;
	}
	if(test_manager()==0){
		std::cout << "manager clear" << std::endl;
	}
	return 0;
}

void printStat(Stat &st){
	std::cout << st.path << " " << st.name << " " <<  st.st.st_size << " " << st.st.st_atime << std::endl;
}
void printstat(struct stat &st){
	std::cout << st.st_size << " " << st.st_atime << " " <<  std::endl;
}

int test_sftp(){
	int ec;
	auto root = "/home/yonde/Documents/cpro";
	auto p2 = "hello/hello.c";
	std::cout << add_path(root,p2) << std::endl;
	sftp* p_connector = new sftp;
	Stat attribute;
	ec = p_connector->getstat(PATH,attribute);
	if(ec==0){
		printStat(attribute);
	}
	std::list<Stat> attrs = p_connector->getdir("hello");
	std::list<Stat>::iterator itr;
	for(itr=attrs.begin();itr!=attrs.end();itr++){
		printStat(*itr);
	}
	//fulldownload
	p_connector->fulldownload(PATH, DEST);
	//download
	char buffer[8];
	p_connector->download(PATH,buffer,2,8);
	buffer[7]='\0';
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
	char buf[32];

	memset(buf,0,sizeof(buf));
	block *b1 = new block(p_connector,&path,0);
	if(b1==NULL){
		return -1;
	}
	if(b1->download()<0){
		return -1;
	}
	nread=b1->read(buf,0,32);
	if(nread<0){
		return -1;
	}
	buf[31]='\0';
	std::cout<< '\n' << nread << "\n" << buf<< '\n'  << std::endl;
	delete b1;
	delete p_connector;
	return 0;
}

int test_entry(){
	sftp* p_connector = new sftp;
	std::string path = PATH;

	//attribute
	attribute* a1 = new attribute(p_connector,path);
	struct stat s1 = a1->getattr();
	std::cout << "\ns1.st_size : " << s1.st_size << '\n'  << std::endl;
	printstat(s1);

	//entry
	entry* e1 = new entry(path,p_connector);
	if(e1->getattr().st_size<0){
		return -1;
	}

	//directory
	std::string path2 = PATH2;
	directory* d1 = new directory(path2,p_connector);
	d1->ls();

	//file
	int wsize,size = 242;
	char buf[size];
	char wbuf[] = "yuta";
	file* fi = new file(PATH,p_connector);
	fi->open();
	fi->read(buf,0,size);
	buf[size-1]='\0';
	std::cout << '\n'  << buf << '\n' << std::endl;
	wsize = fi->write(wbuf,0,sizeof(wbuf)-1);
	std::cout << "write size " << wsize << std::endl;
	fi->read(buf,0,size);
	buf[size-1]='\0';
	std::cout << '\n'  << buf << '\n' << std::endl;
	fi->close();

	//delete
	delete a1;
	delete e1;
	delete fi;
	delete p_connector;
	return 0;
}

int test_manager(){
	int ec;
	file* et;
	char buf[8];
	sftp* p_connector = new sftp;
	manager* man = new manager(p_connector);
	et=(file*)man->lookup(PATH);
	if(et==NULL){
		return -1;
	}
	et->open();
	et->read(buf,0,8);
	buf[7]='\0';
	std::cout << buf << std::endl;
	et->close();
	std::cout << "entrymap size : " << man->size() << std::endl;
	ec=man->release(PATH);
	if(ec<0){
		return -1;
	}
	std::cout << "entrymap size : " << man->size() << std::endl;
	delete man;
	delete p_connector;
	return 0;
}