#include <iostream>
#include <libssh/libssh.h>

class sftp {
private:
	ssh_session ssh;
public:
	sftp(){
		ssh = ssh_new();
		ssh_options_set(ssh, SSH_OPTIONS_HOST, "localhost");
		if(ssh_connect(ssh)!=SSH_OK){
			std::cerr << "ssh_connect failed: " << ssh_get_error(ssh) << std::endl;
		}
		std::cout << "connect success" << std::endl;
	}
	~sftp(){
		ssh_disconnect(ssh);
		ssh_free(ssh);
	}
};

int main(){
	sftp* p_connector = new sftp;
	return 0;
}