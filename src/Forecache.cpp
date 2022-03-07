#include "Forecache.hpp"
#include "Block.hpp"
#include "Manager.hpp"
#include "logger.hpp"

forecache::forecache(stdobj* parent):stdobj(parent){
    LOG_INFO("Called");
}

forecache::~forecache()
{
    LOG_INFO("Called");
    flag=false;
    loadth.join();
    decisionth.join();
}


//同じディレクトリのファイルをキューに与える。
int forecache::decidecache()
{
    LOG_INFO("Called");
    std::string dirname;
    std::string entity;
    list<Stat> attrs;
    while(flag){
	//LOG_INFO("Loop");
	dirname=((manager*)base)->p_cache->find_max("dir");
	//dir内ファイル名を取得
	LOG_INFO("Looking for files in %s",dirname.c_str());
	attrs=((manager*)base)->p_sftp->getdir(dirname);
	for(auto itr=attrs.begin();itr!=attrs.end();itr++){
	    if((itr->name == ".") or (itr->name == "..")){
		continue;
	    }
	    if(itr->type == 2){
		continue;
	    }
	    LOG_INFO("%s",itr->name.c_str());
	    if(dirname=="/"){
		entity = dirname + itr->name;
	    }else{
		entity = dirname + "/" + itr->name;
	    }
	    //for(auto i=loadqueue.begin();i!=loadqueue.end();i++){
	    //    exist = (entity == *i);
	    //}
	    //LOG_INFO("Push %s",entity.c_str());
	    loadqueue.push_back(entity);
        }
	sleep(5);
    }
    return 0;
}

int forecache::download(std::string path)
{
    LOG_INFO("Called against %s",path.c_str());
    std::string dest;
    StatCache sc;
    int block_size;
    //cacheにあるかチェック
    if(((manager*)base)->p_cache->find_stat(path,sc)<0){
	//cacheに存在しない
	dest=((manager*)base)->p_cache->get_location(path); 
	//fulldownload
	((manager*)base)->p_sftp->fulldownload(path,dest);	
	LOG_INFO("download 1 %s",path.c_str());
	//stat cacheに加える
	((manager*)base)->p_cache->add_stat(path,sc.size,sc.mtime);
	//block cacheに加える
	block_size=(sc.size / BLOCK_SIZE + 1);
	LOG_INFO("download 2 %s",path.c_str());
	for(int i=0;i<block_size;i++){
	    ((manager*)base)->p_cache->add_block(path,i);
	}
	LOG_INFO("download 3 %s",path.c_str());
    }
    return 0;
}

int forecache::loadtask()
{
    LOG_INFO("Called");
    while(flag){
	//LOG_INFO("Loop");
	std::string contents="";
	for(auto i=loadqueue.begin();i!=loadqueue.end();++i){
	    contents = contents + *i + " ";
	}
	LOG_INFO("loadqueue contents [%s]",contents.c_str());
	//load
	if(!loadqueue.empty()){
	    std::string path = loadqueue.front();
	    //LOG_INFO("Prefetching %s",path.c_str());
	    //download&cache
	    this->download(path);
	    //loadqueueから削除
	    loadqueue.pop_front();
	}else{
	    sleep(5);
	}
    }
    return 0;
}

void forecache::loopstart()
{
    LOG_INFO("Called");
    thread load_th(&forecache::loadtask,this);
    thread decision_th(&forecache::decidecache,this);
    loadth=move(load_th);
    decisionth=move(decision_th);
}

