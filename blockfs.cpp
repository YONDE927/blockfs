#include "blockfs.hpp"

manager* p_manager;

static void printstat(struct stat &st){
	std::cout << "st.st_mode " << st.st_mode 
	<< "\nst.st_size " << st.st_size 
	<< "\nst.st_mtime " << st.st_mtime 
	<< "\nst.st_nlink " << st.st_nlink << std::endl;
}

void* b_init(struct fuse_conn_info *conn,struct fuse_config *fc){
	sftp* p_sftp = new sftp;
	cache* p_cache = new cache;
	p_manager = new manager(p_sftp,p_cache);
	//conn->want |= (conn->capable & FUSE_CAP_READDIRPLUS);
    return fuse_get_context()->private_data;
}

int b_getattr(const char *path,struct stat *stbuf,struct fuse_file_info *fi){
	std::cout<< "\n"  << "b_getattr " << path << std::endl;
	struct stat st;
	entry* et;
	et=p_manager->lookup(std::string(path));
	if(et==NULL){
		std::cout << "b_getattr fail " << path << "\n" << std::endl;
		return -ENOENT;
	}
	*stbuf=et->getattr(1);
	return 0;
}

int b_open(const char *path,struct fuse_file_info *fi){
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -ENOENT;
	}
	et->fopen();
	return 0;
}

int b_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	int nread;
	file* et;
	std::cout << "b_read " << path << std::endl;
	et=(file*)p_manager->lookup(std::string(path));
	nread=et->fread(buf,offset,size);
	std::cout << "read " << nread << " size. " << path << std::endl;
	if(nread<0){
		std::cout << "b_read failed" << path << std::endl;
		return -ENOENT;
	}
	return nread;
}

int b_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	std::cout << "b_write " << path << std::endl;
	int nwritten;
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	nwritten=et->fwrite(buf,offset,size);
	if(nwritten<0){
		std::cout << "b_write failed" << path << std::endl;
		return -ENOENT;
	}
	return nwritten;
}

int b_close(const char *path,struct fuse_file_info *fi){
	file* et;
	et=(file*)p_manager->lookup(std::string(path));
	et->fclose();
	return 0;
}

int b_opendir(const char *path,struct fuse_file_info *fi){
	std::cout << "b_opendir " << path << std::endl;
	directory* et;
	et=(directory*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -ENOENT;
	}
	fi->fh=0;
	return 0;
}

int b_readdir(const char *path,void *buf,fuse_fill_dir_t filler,off_t offset,struct fuse_file_info *fi,enum fuse_readdir_flags flags){
	directory* et;
	std::list<attribute*> attrs;
	std::cout << "b_readdir " << path << std::endl;
	et=(directory*)p_manager->lookup(std::string(path));
	if(et==NULL){
		return -ENOENT;
	}
	attrs=et->readdir();
	for(auto itr=attrs.begin();itr!=attrs.end();++itr){
		struct stat st = (*itr)->getattr(1);
		filler(buf,(*itr)->name.c_str(),&st,0,FUSE_FILL_DIR_PLUS);
		// filler(buf,(*itr)->name.c_str(),NULL,0,FUSE_FILL_DIR_PLUS);
	}
	return 0;
}

int b_closedir(const char *path,struct fuse_file_info *fi){
	std::cout << "b_closedir " << path << std::endl;
	return 0;
}

// int b_statfs(const char *path, struct statvfs *buf){
//     memset(buf, 0, sizeof(struct statvfs));
// 	buf->f_bsize = 4096;
// 	buf->f_frsize = 4096;
// 	buf->f_blocks = 1024;     
// 	buf->f_bfree = 1024;
// 	buf->f_bavail = 1024;
// 	buf->f_files = 1000;
// 	buf->f_ffree = 100;
// 	buf->f_favail = 100;
// 	buf->f_fsid = 0;
// 	buf->f_flag = 0;
// 	buf->f_namemax = 255;
//     return 0;
// }

struct fuse_operations b_oper = {
	.getattr = b_getattr,
	.open = b_open,
	.read = b_read,
	.write = b_write,
	// .statfs = b_statfs,
	.release = b_close,
	//.opendir = b_opendir,
	.readdir = b_readdir,
	//.releasedir = b_closedir,
	.init = b_init
};

int main(int argc, char *argv[])
{
    return fuse_main(argc,argv, &b_oper, NULL);
}
