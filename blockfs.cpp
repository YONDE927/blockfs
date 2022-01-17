#include "blockfs.h"

manager* p_manager;

void* b_init(struct fuse_conn_info *conn,struct fuse_config *fc){
	sftp* p_sftp = new sftp;
	p_manager = new manager(p_sftp);
	return NULL;
}

int b_getattr(const char *path,struct stat *stbuf,struct fuse_file_info *fi){
	entry* et;
	et=p_manager->lookup(std::string(path));
	std::cout << "b_getattr " << path << std::endl;
	if(et==NULL){
		std::cout << "b_getattr fail " << path << std::endl;
		return -errno;
	}
	std::cout << "b_getattr success " << path << "size : " << et->getattr().st_size << std::endl;
	*stbuf=et->getattr();
	std::cout << "stbuf " << stbuf->st_size << std::endl;
	return 0;
}

int b_open(const char *path,struct fuse_file_info *fi){
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -errno;
	}
	et->open();
	return 0;
}

int b_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	int nread;
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	nread=et->read(buf,offset,size);
	if(nread!=size){
		return -errno;
	}
	return 0;
}

int b_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	int nwritten;
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	nwritten=et->write(buf,offset,size);
	if(nwritten!=size){
		return -errno;
	}
	return 0;
}

int b_close(const char *path,struct fuse_file_info *fi){
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	et->close();
	return 0;
}

int b_opendir(const char *path,struct fuse_file_info *fi){
	std::cout << "b_opendir " << path << std::endl;
	directory* et;
	et=(directory*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -errno;
	}
	return 0;
}

int b_readdir(const char *path,void *buf,fuse_fill_dir_t filler,off_t offset,struct fuse_file_info *fi,enum fuse_readdir_flags flags){
	directory* et;
	std::list<attribute*> attrs;
	std::cout << "b_readdir " << path << std::endl;
	et=(directory*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -errno;
	}
	attrs=et->readdir();
	for(auto itr=attrs.begin();itr!=attrs.end();++itr){
		struct stat st = (*itr)->getattr();
		filler(buf,(*itr)->name.c_str(),&st,0,FUSE_FILL_DIR_PLUS);
	}
	return 0;
}

int b_closedir(const char *path,struct fuse_file_info *fi){
	std::cout << "b_closedir " << path << std::endl;
	return 0;
}

const struct fuse_operations b_oper = {
	.getattr = b_getattr,
	.open = b_open,
	.read = b_read,
	.write = b_write,
	.release = b_close,
	.opendir = b_opendir,
	.readdir = b_readdir,
	.releasedir = b_closedir,
	.init = b_init
};

int main(int argc, char *argv[])
{
        enum { MAX_ARGS = 10 };
        int i,new_argc;
        char *new_argv[MAX_ARGS];
        umask(0);
        for (i=0, new_argc=0; (i<argc) && (new_argc<MAX_ARGS); i++) {
                new_argv[new_argc++] = argv[i];
        }
        return fuse_main(new_argc, new_argv, &b_oper, NULL);
}
