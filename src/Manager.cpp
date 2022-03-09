#include "Manager.hpp"
#include "Entry.hpp"
#include "logger.hpp"

#define MEM_FILE_MAX_NUM 100
manager::manager(sftp* _p_sftp,cache* _p_cache):stdobj(NULL){
    LOG_DEBUG("Called");
    p_sftp=_p_sftp;
    p_cache=_p_cache;
}

manager::~manager(){
    LOG_DEBUG("Called");
    entrymap.clear();
}

entry* manager::lookup(std::string path){
    LOG_TRACE("Called against %s",path.c_str());
    this->log_queue(file_queue);
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
	//もうメモリ上にファイルをおけない場合はNULLを返す。
	if(this->remove_limit_entry()<0){
	    return NULL; 
	}
	et = new file(this,path);
	file_queue.push(path);
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
	if(((file*)et)->fd>0){
	    return -1;
	}
	entrymap.erase(path);
    }
    return 0;
}

int manager::size(){
    return entrymap.size();
}

//MEM_FILE_MAX_NUMを超えたメモリ上のファイルを削除
//or
//MAX_ALIVE_TIMEを超えたファイルをメモリ上から削除
int manager::remove_limit_entry(){
    int cnt=0;
    while(file_queue.size()>MEM_FILE_MAX_NUM){
	std::string path = file_queue.front();
	file_queue.pop();
	if(this->release(path)<0){
	    file_queue.push(path);
	    cnt++;
	}	
	//もし一周したら−1を返す。
	if(cnt > MEM_FILE_MAX_NUM){
	    return -1;
	}
    }
    return 0;
}

void manager::log_queue(std::queue<std::string> fq){
    std::string path_list = "";
    while(!fq.empty()){
	path_list = path_list + fq.front() + " ";
	fq.pop();
    }
    LOG_DEBUG("file_queue is [%s]",path_list.c_str());
}
