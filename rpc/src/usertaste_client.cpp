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
	std::cout << "loop start" << std::endl;
	while(true){
	    this->loader();
	    sleep(10);
	}
    }
    void loader(){
	std::cout << "loader called" << std::endl;
	std::list<std::string> reserved_path;
	std::string most_used_path = p_cache->find_max("path");
	std::string dir_path = std::filesystem::path(most_used_path).parent_path();
	std::string remote_path = p_sftp->sftproot + most_used_path;
	int dir_len = p_sftp->sftproot.size();

	reserved_path = p_utc->ListFile(remote_path,10);
	for(auto i = reserved_path.begin();i != reserved_path.end(); i++){
	    std::cout << "file is " << *i << std::endl;
	    std::string tmp = i->substr(dir_len);
	    std::string dest = p_cache->get_location(tmp); 
	    if(std::filesystem::exists(dest)){
		std::cout << " already exist " << dest  << std::endl;
		continue;
	    }
	    std::cout << *i << " >> " << dest << std::endl;
	    p_sftp->fulldownload(*i,dest);	
	}
    }
};

int main()
{
    Logger::Set_Priority(DebugPriority);
    std::string target = "localhost:50051";
    UserTasteClient utc(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    Prefetcher pft = Prefetcher(&utc);
    pft.loop();
    return 0;
}

