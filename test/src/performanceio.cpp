#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <ctime>
#include <time.h>
#include <unistd.h>

using namespace std;


//ファイルを読み込み、list<string>に格納。
int load(string path,list<string> &listbuf)
{
	list<string> output;
	string buf;
	int line{0};
	ifstream ifs(path);
	if(!ifs){
		cout << "can't open " << path << endl;
		return -1;
	}
	while(!ifs.eof()){
		ifs >> buf;
		line++;
		listbuf.push_back(buf);
	}
	return 0;
}

int tester(list<string> loadschedule)
{
	time_t t;
	string enter;
	list<string> readbuf;
	//loadscheduleの順で読み込む,最初のファイルを読んで先読みキャッシュを待つ時間を設ける。
	//最初のファイル
	if(load(loadschedule.front(),readbuf)<0){
		cout << "load " << loadschedule.front() << endl;
		return 0;
	}
	loadschedule.pop_front();
	//テスターにキャッシュが溜まったことを確認させる
	cout << "キャッシュが貯まったらEnterを押してください。";
	cin >> enter;
	//現在時刻をセットし、リストのファイルを順次読み込む。
	t = time(NULL);
	for(auto itr=loadschedule.begin();itr!=loadschedule.end();itr++){
		load(*itr,readbuf);
	}
	//読み込み後の時刻との差分を返す。
	return t-time(NULL);
}

int main(int argc,char* argv[])
{
	list<string> similarschedule;
	list<string> randomschedule;
	list<string> readbuf;
	string enter;
	int ss,rs;
	
	//引数が足りない
	if(argc < 3){
		cout << "call with similarschedule and randomschedule" << endl;
		return 0;
	}
	//loadscheduleを読み込み、読み込み予定のファイルをリスト化
	if(load(argv[1],similarschedule)<0){
		cout << "set similarschedule fail" << endl;
	}
	if(load(argv[2],randomschedule)<0){
		cout << "set randomschedule fail" << endl;
	}

	//各リストに対してテスト
	ss = tester(similarschedule);
	rs = tester(randomschedule);
	cout << "similar: " << ss << "	random: " << rs << endl;
	return 0;
}
