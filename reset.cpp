#include <stdio.h>
#include <mariadb/conncpp.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace std;

#define CONFIGPATH "/home/yonde/Documents/blockfs/build/config/cacheconfig"

int loadoption(string &host,string &username,string &password,string &cache_dir){
	int index;
	string line,key;
	string ConfigPath = CONFIGPATH;
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


int main()
{
    string confirm,host,username,password,cache_dir;
    sql::Driver *driver;
    sql::Connection *p_session;

    loadoption(host,username,password,cache_dir);
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
    cout << "Do you really want it to be erased? (Y/n):";
    cin >> confirm;
    if((confirm == "Y") or (confirm == "y")){
    //deleting cachedb tables
    cout << "deleting cachedb and stats blocks history" << endl;
    std::unique_ptr<sql::Statement> stmt(p_session->createStatement());
    stmt->executeQuery("USE cachedb");
    stmt->execute("DROP TABLE IF EXISTS stats");
    stmt->execute("DROP TABLE IF EXISTS blocks");
    stmt->execute("DROP TABLE IF EXISTS history");

    //deleting cache dir
    cout << "deleting cachefiles" << endl;
    std::filesystem::remove_all(cache_dir);
    std::filesystem::create_directory(cache_dir);
    
    cout << "finish" << endl;
    }
}
