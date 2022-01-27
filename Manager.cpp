#include "Manager.h"

manager::manager(sftp* _p_sftp){
	p_sftp=_p_sftp;
}

manager::~manager(){
	for(auto itr=entrymap.begin();itr!=entrymap.end();++itr){
		delete itr->second;
	}
	delete p_sftp;
}

entry* manager::lookup(std::string path){
	entry* et;
	std::cout << "lookup " << path << std::endl;
	auto pair=entrymap.find(path);
	if(pair==entrymap.end()){
		et = this->load(path);
	}else{
		std::cout << path << " found " << std::endl;
		et = pair->second;
	}
	return et;
}

entry* manager::load(std::string path){
	int ec;
	Stat St;
	entry* et;
	std::cout << "load " << path << std::endl;
	ec = p_sftp->getstat(path,St);
	if(ec<0){
		return NULL;
	}
	if(St.type==1){
		et = new file(path,p_sftp);
	}else if(St.type==2){
		et = new directory(path,p_sftp);
	}else{
		std::cout << "St.type not match" << path << std::endl;
	}
	entrymap[path]=et;
	return et;
}

int manager::release(std::string path){
	entry* et;
	auto pair=entrymap.find(path);
	if(pair==entrymap.end()){
		return -1;
	}else{
		et = pair->second;
		entrymap.erase(path);
		delete et;
	}
	return 0;
}

int manager::size(){
	return entrymap.size();
}