#ifndef BLOCKFS_H
#define BLOCKFS_H

#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string>
#include <list>

#include "Manager.h"
#include "Entry.h"

void* b_init(struct fuse_conn_info *conn,struct fuse_config *fc);
int b_getattr(const char *path,struct stat *stbuf,struct fuse_file_info *fi);
int b_open(const char *path,struct fuse_file_info *fi);
int b_read(const char *path,char *buf,size_t size,off_t offset,
		   struct fuse_file_info *fi);
int b_write(const char *path,const char *buf,size_t size,
			off_t offset,struct fuse_file_info *fi);
int b_close(const char *path,struct fuse_file_info *fi);
int b_opendir(const char *path,struct fuse_file_info *fi);
int b_readdir(const char *path,void *buf,fuse_fill_dir_t filler,
			  off_t offset,struct fuse_file_info *fi,
			  enum fuse_readdir_flags flags);
int b_closedir(const char *path,struct fuse_file_info *fi);
int b_statfs(const char *path, struct statvfs *stbuf);

#endif