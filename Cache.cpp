#include "Cache.h"

#define CONFIGPATH "/home/yonde/Documents/blockfs/build/config/cacheconfig"

cache::cache(){
	this->loadoption();
	cout << host << username << password << endl;
	SessionSettings settings(SessionOption::HOST,host,SessionOption::USER,username,SessionOption::PWD,password.c_str());
	p_session = new Session(settings);
	p_session->sql("USE cachedb").execute();
	p_session->sql("CREATE TABLE IF NOT EXISTS stats(path TEXT,size INT,mtime INT,PRIMARY KEY(path(256)))").execute();
	p_session->sql("CREATE TABLE IF NOT EXISTS blocks(path TEXT,block_index INT,location TEXT,PRIMARY KEY(path(256),block_index))").execute();
}

int cache::loadoption(){
	int index;
	std::string line,key;
	ConfigPath = CONFIGPATH;
	std::ifstream ifs;
	ifs.open(ConfigPath);
	if(!ifs){
		std::cerr << "open sshconfig failed" << std::endl;
	}
	while(std::getline(ifs,line)){
		index = line.find(" ");
		key=line.substr(0,line.find(" "));
		if(key=="host"){
			host = line.substr(index+1);
		}else if(key=="username"){
			username = line.substr(index+1);
		}else if(key=="password"){
			password = line.substr(index+1);
		}else if(key=="cachedir"){
			cache_dir = line.substr(index+1);
		}
	}
	return 0;
}

int cache::add_stat(std::string path,int size,int mtime){
	std::string insert = "INSERT INTO stats (path,size,mtime) VALUES('";
	insert += (path +"',"s + std::to_string(size) + ","s + std::to_string(mtime) + ")"s);
	std::string update = "ON DUPLICATE KEY UPDATE size=" + std::to_string(size) + ", mtime="s + std::to_string(mtime);
	cout << insert + update << endl;
	p_session->sql(insert + update).execute();
	return 0;
}

int cache::find_stat(std::string path,StatCache &sc){
	std::string select = "SELECT * FROM stats WHERE path = '" + path + "'"s;
	cout << select << endl;
	auto result = p_session->sql(select).execute();
	Row row = result.fetchOne();
	if(row){
		sc.path=path;
		sc.size=row[1];
		sc.mtime=row[2];
		return 0;
	}else{
		cout << "not found in cache" << endl;
		return -1;
	}
}

int cache::add_block(std::string path,int index){
	std::string location = this->get_location(path);
	std::string insert = "INSERT INTO blocks (path,block_index,location) VALUES('" + path + "',"s + std::to_string(index) + ",'"s + location + "')";
	std::string update = "ON DUPLICATE KEY UPDATE location='" + location + "'"s;
	cout << insert+update << endl;
	p_session->sql(insert + update).execute();
	return 0;
}

int cache::find_block(std::string path,int index,BlockCache &bc){
	std::string select = "SELECT * FROM blocks WHERE path='" + path + "'"s + " AND block_index=" + std::to_string(index);
	cout << select << endl;
	auto result = p_session->sql(select).execute();
	cout << "exed" << endl;
	Row row = result.fetchOne();
	if(row){
		cout << row[0] << row[1] << row[2] << endl;
		bc.path = path;
		bc.index = index;
		bc.location = std::string(row[2]);
		return 0;
	}else{
		cout << index << " block no cached" << endl;
		return -1;
	}
}

std::string cache::get_location(std::string path){
	int i{0};
	for(auto itr=path.begin();itr!=path.end();++itr){
		if(*itr=='/'){
			path.replace(i,1,"%"s);
		}
		i++;
	}
	return cache_dir + path;
}
