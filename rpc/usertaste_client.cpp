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


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using usertaste::UserTaste;
using usertaste::Stat;
using usertaste::User;
using usertaste::Want;
using usertaste::File;

class UserTasteClient{
private:
  std::unique_ptr<UserTaste::Stub> stub_;
public:
    UserTasteClient(std::shared_ptr<Channel> channel)
	: stub_(UserTaste::NewStub(channel)) {}

    int Greet(const std::string &something,int size)
    {
	Want want;
	want.set_something(something);
	want.set_size(size);

	Stat stat_response;
	ClientContext context;
	Status status = stub_->greet(&context,want,&stat_response);
	if(status.ok()){
	    return stat_response.status();
	}else{
	    std::cout << status.error_code() << ": " << status.error_message()<< std::endl;
	    return -1;
	}
    }
    std::list<std::string> ListFile(const std::string &something,int size)
    {
	Want want;
	want.set_something(something);
	want.set_size(size);

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

int main()
{
    std::string target = "localhost:50051";
    UserTasteClient utc(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    std::string something = "hello";
    int size = 5;
    int replay = utc.Greet(something,size);
    std::cout << replay << std::endl;
    std::list<std::string> read_ahead_files;
    read_ahead_files = utc.ListFile(something,size);
    for(auto itr=read_ahead_files.begin();itr!=read_ahead_files.end();++itr){
	std::cout << *itr << std::endl;
    }
    return 0;
}

