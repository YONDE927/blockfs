#include <iostream>
#include <memory>
#include <string>
#include <list>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "usertaste.grpc.pb.h"
#include "Cache.hpp"
#include "Sftp.hpp"
#include "logger.hpp"
#include <string>
#include <filesystem>

#define BLOCK_SIZE 1024

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using usertaste::UserTaste;
using usertaste::User;
using usertaste::Want;
using usertaste::File;

class UserTasteClient{
private:
  std::unique_ptr<UserTaste::Stub> stub_;
public:
    UserTasteClient(std::shared_ptr<Channel> channel)
	: stub_(UserTaste::NewStub(channel)) {}

    std::list<std::string> ListFile(const std::string &path,int size)
    {
	Want want;
	want.set_most_used_file(path);
	want.set_prefetch_num(size);

	File fi;
	std::list<std::string> output;
	ClientContext context;
	std::unique_ptr<grpc::ClientReader<File>> reader(stub_->listfile(&context,want));
	while (reader->Read(&fi)){
	    output.push_back(fi.path());
	}
	Status status = reader->Finish();
	if(status.ok()){
	    std::cout << "ListFile succeeded" << std::endl;
	}else{
	    std::cout << "ListFile failed" << std::endl;
	}
	return output;
    }	
};

class Prefetcher
{
private:
    cache* p_cache;
    sftp* p_sftp;
    UserTasteClient* p_utc;
public:
    Prefetcher(UserTasteClient* _p_utc){
	p_sftp = new sftp;
	p_cache = new cache;
	p_utc = _p_utc;
    }
    void loop(){
	LOG_INFO("loop start");
	while(true){
	    this->loader();
	    sleep(10);
	}
    }
    void loader(){
	LOG_INFO("loader start");
	int rc,block_num;
	Stat st;
	std::list<std::string> reserved_path;
	std::string most_used_path = p_cache->find_max("path");
	std::string dir_path = std::filesystem::path(most_used_path).parent_path();
	std::string remote_path = p_sftp->sftproot + most_used_path;
	int dir_len = p_sftp->sftproot.size();

	reserved_path = p_utc->ListFile(remote_path,10);
	for(auto i = reserved_path.begin();i != reserved_path.end(); i++){
	    std::string tmp = i->substr(dir_len);
	    std::string dest = p_cache->get_location(tmp); 
	    if(std::filesystem::exists(dest)){
		std::cout << " already exist " << dest  << std::endl;
		continue;
	    }
	    rc = p_sftp->fulldownload(tmp,dest);	
	    if(rc<0){
		LOG_ERROR("loading %s failed",tmp.c_str());
	    }
	    //add to cachedb
	    //add stat
	    if(p_sftp->getstat(tmp, st)<0){
		LOG_ERROR("Failed to get block size",tmp.c_str());
		continue;
	    }
	    p_cache->add_stat(tmp,st.st.st_size,st.st.st_mtime);
	    //add blocks
	    block_num = (st.st.st_size / BLOCK_SIZE) + 1;
    	    for(int i=0;i<block_num;i++){
		p_cache->add_block(tmp,i);
    	    }
	}
    }
};

int main()
{
    Logger::Set_Priority(TracePriority);
    std::string target = "localhost:50051";
    UserTasteClient utc(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    Prefetcher pft = Prefetcher(&utc);
    pft.loop();
    return 0;
}

