#include "forecache.hpp"
#include "Block.hpp"
#include "Manager.hpp"

forecache::forecache(stdobj* parent):stdobj(parent){
   ;
}

forecache::~forecache()
{
    flag=false;
    loadth.join();
    decisionth.join();
}


//同じディレクトリのファイルをキューに与える。
int forecache::decidecache()
{
    std::string dirname;
    list<Stat> attrs;
    while(flag){
	dirname=((manager*)base)->p_cache->find_max("dir");
	if(dirname!=key){
	    //dir内ファイル名を取得
	    attrs=((manager*)base)->p_sftp->getdir(dirname);
	    for(auto itr=attrs.begin();itr!=attrs.end();++itr){
		if((itr->name == ".") or (itr->name == "..")){
		    continue;
		}
		if(dirname=="/"){
		    loadqueue.push(dirname + itr->name);
		}else{
		    loadqueue.push(dirname+"/"+itr->name);
		}
	    }
	}
    }
    return 0;
}

int forecache::download(std::string path)
{
    std::string dest;
    StatCache sc;
    int block_size;
    //cacheにあるかチェック
    if(((manager*)base)->p_cache->find_stat(path,sc)<0){
	//cacheに存在しない
	dest=((manager*)base)->p_cache->get_location(path); 
	//fulldownload
	((manager*)base)->p_sftp->fulldownload(path,dest);	
	//stat cacheに加える
	((manager*)base)->p_cache->add_stat(path,sc.size,sc.size);
	//block cacheに加える
	block_size=(sc.size / BLOCK_SIZE + 1);
	for(int i=0;i<block_size;i++){
	    ((manager*)base)->p_cache->add_block(path,i);
	}
    }
    return 0;
}

int forecache::loadtask()
{
    while(flag){
	//load
	if(!loadqueue.empty()){
	    std::string path = loadqueue.front();
	    //download&cache
	    this->download(path);
	    //loadqueueから削除
	    loadqueue.pop();
	}
    }
    return 0;
}

void forecache::loopstart()
{
    thread load_th(&forecache::loadtask,this);
    thread decision_th(&forecache::decidecache,this);
    loadth=move(load_th);
    decisionth=move(decision_th);
}

