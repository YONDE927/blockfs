#include "Cache.hpp"
#include <mariadb/conncpp/Connection.hpp>
#include <mariadb/conncpp/Driver.hpp>
#include <mariadb/conncpp/PreparedStatement.hpp>

#define CONFIGPATH "/home/yonde/Documents/blockfs/build/config/cacheconfig"

cache::cache(){
	this->loadoption();
	cout << host << " " << username << " "<< password << endl;
	//connect db
	sql::SQLString url = "jdbc:mariadb://" + host + ":3306/cachedb";
	sql::Properties props({
		{"user",username},
		{"password",password}
		});
	//std::unique_ptr<sql::Driver> dp(sql::mariadb::get_driver_instance());
	driver = sql::mariadb::get_driver_instance();
	cout << driver->getName() << endl;
	p_session = driver->connect(url,props);
	if(p_session==NULL){
	    std::cerr << "can't connect to db" << std::endl;
	}
	cout << p_session->getUsername() << " " << p_session->getHostname() << endl;
	cout << "create session" << endl;
	//check existence of table
	std::unique_ptr<sql::Statement> stmt(p_session->createStatement());
	stmt->executeQuery("USE cachedb");
	stmt->execute("CREATE TABLE IF NOT EXISTS stats(path TEXT,size INT,mtime INT,PRIMARY KEY(path(256)))");
	stmt->execute("CREATE TABLE IF NOT EXISTS blocks(path TEXT,block_index INT,location TEXT,PRIMARY KEY(path(256),block_index))");
	stmt->execute("CREATE TABLE IF NOT EXISTS history(time INT,path TEXT,dir TEXT,name TEXT,ext TEXT,size INT)");
	cout << "use cachedb and stats blocks history" << endl;
}

cache::~cache(){
    cout << "deleting cache" << endl;
    //delete p_session;
    //delete driver;
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
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"REPLACE INTO stats (path,size,mtime) VALUES(?,?,?)"
		));
    stmt->setString(1,sql::SQLString(path));
    stmt->setInt(2,size);
    stmt->setInt(3,mtime);
    stmt->executeUpdate();
    return 0;
}

int cache::find_stat(std::string path,StatCache &sc){
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"SELECT * FROM stats WHERE path = ?"
		));
    stmt->setString(1,path);
    std::shared_ptr<sql::ResultSet> res(stmt->executeQuery());
    if(res->next()){
	sc.path=res->getString(1);
	sc.size=res->getInt(2);
	sc.mtime=res->getInt(3);
    }else{
	cout << "not found in cache" << endl;
	return -1;
    }
    return 0;
}

int cache::add_block(std::string path,int index){
    std::string location = this->get_location(path);
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"REPLACE INTO blocks (path,block_index,location) VALUES (?,?,?)"
		));
    stmt->setString(1,sql::SQLString(path));
    stmt->setInt(2,index);
    stmt->setString(3,sql::SQLString(location));
    stmt->executeUpdate();
    return 0;
}

int cache::find_block(std::string path,int index,BlockCache &bc){
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"SELECT * FROM blocks WHERE path = ? AND block_index = ?"
		));
    stmt->setString(1,path);
    stmt->setInt(2,index);
    std::shared_ptr<sql::ResultSet> res(stmt->executeQuery());
    if(res->next()){
	bc.path=res->getString(1);
	bc.index=res->getInt(2);
	bc.location=res->getString(3);
    }else{
	cout << index << "block no cached" << endl;
	return -1;
    }
    return 0;
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

int cache::add_history(std::string path,int size)
{
    std::time_t now = std::time(NULL);
    //get name of file
    std::filesystem::path p = path;
    std::string ext = p.extension();
    std::string dir = p.parent_path();
    std::string name = p.filename();
    std::string location = this->get_location(path);
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"INSERT INTO history (time,path,dir,name,ext,size) VALUES (?,?,?,?,?,?)"
		));
    stmt->setInt(1,now);
    stmt->setString(2,sql::SQLString(path));
    stmt->setString(3,sql::SQLString(dir));
    stmt->setString(4,sql::SQLString(name));
    stmt->setString(5,sql::SQLString(ext));
    stmt->setInt(6,size);
    stmt->executeUpdate();
    return 0;
}

int cache::count_history(std::string key,std::string col)
{
    int i=0;
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::PreparedStatement> stmt(p_session->prepareStatement(
		"SELECT * FROM history WHERE " + col + " = ?"
		));
    stmt->setString(1,col);
    stmt->setString(2,key);
    std::shared_ptr<sql::ResultSet> res(stmt->executeQuery());
    while(res->next()){
	i++;
    }
    return i;
}

std::string cache::find_max(std::string col)
{
    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<sql::Statement> stmt(p_session->createStatement(
		//"SELECT ? AS ?, COUNT(*) cnt FROM history GROUP BY ? ORDER BY cnt DESC"
		));
    std::shared_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT " + col + " AS " + col + ", COUNT(*) cnt FROM history GROUP BY " + col + " ORDER BY cnt DESC"));
    if(res->next()){
	cout << std::string(res->getString(col)) << " " << res->getInt("cnt") << endl;
	return std::string(res->getString(col));
    }else{
	return std::string();
    }
}

