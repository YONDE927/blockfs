#include <iostream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

class sftp {
private:
	ssh_session m_ssh;
	sftp_session m_sftp;
public:
	sftp(){
		const char* username = "yonde";
		const char* password = "taiki927";
		m_ssh = ssh_new();
		ssh_options_set(m_ssh, SSH_OPTIONS_HOST, "localhost");
		if(ssh_connect(m_ssh)!=SSH_OK){
			ssh_free(m_ssh);
			std::cerr << "ssh_connect failed: " << ssh_get_error(m_ssh) << std::endl;
		}
		std::cout << "connect success" << std::endl;
		if(ssh_userauth_password(m_ssh,username,password)!=SSH_AUTH_SUCCESS){
			ssh_free(m_ssh);
			std::cerr << "ssh_userauth_password failed: " << ssh_get_error(m_ssh) << std::endl;
		}
		std::cout << "auth success" << std::endl;
		m_sftp = sftp_new(m_ssh);
		if(sftp_init(m_sftp)!=SSH_OK){
			sftp_free(m_sftp);
			std::cerr << "sftp_init failed: " << ssh_get_error(m_ssh) << std::endl;
		}
		std::cout << "sftp_init success" << std::endl;
	}
	~sftp(){
		sftp_free(m_sftp);
		ssh_disconnect(m_ssh);
		ssh_free(m_ssh);
	}
};

int main(){
	sftp* p_connector = new sftp;
	delete p_connector;
	return 0;
}