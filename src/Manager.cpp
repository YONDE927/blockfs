#include "Manager.hpp"
#include "logger.hpp"

manager::manager(sftp* _p_sftp,cache* _p_cache):stdobj(NULL){
    LOG_DEBUG("Called");
    p_sftp=_p_sftp;
    p_cache=_p_cache;
    //p_forecache= new forecache(this);
    //p_forecache->loopstart();
}

manager::~manager(){
    LOG_DEBUG("Called");
    for(auto itr=entrymap.begin();itr!=entrymap.end();++itr){
	LOG_TRACE("Deleting entry");
	delete itr->second;
    }
    //delete p_forecache;
}

entry* manager::lookup(std::string path){
    LOG_DEBUG("Called against %s",path.c_str());
    entry* et;
    auto pair=entrymap.find(path);
    if(pair==entrymap.end()){
	    et = this->load(path);
    }else{
	LOG_DEBUG("%s found",path.c_str());
	et = pair->second;
    }
    return et;
}

entry* manager::load(std::string path){
    LOG_DEBUG("Called against %s",path.c_str());
    int ec;
    Stat St;
    entry* et;
    ec = p_sftp->getstat(path,St);
    if(ec<0){
	    return NULL;
    }
    if(St.type==1){
	    et = new file(this,path);
    }else if(St.type==2){
	    et = new directory(this,path);
    }else{
	LOG_ERROR("St.type not match %s",path.c_str());
	et = 0;
    }
    entrymap[path]=et;
    return et;
}

int manager::release(std::string path){
    LOG_DEBUG("Called against %s",path.c_str());
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
