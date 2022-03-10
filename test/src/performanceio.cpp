#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <chrono>
#include <filesystem>

using namespace std;


struct info{
    double time;
    double size;
};

//ファイルを読み込み、list<string>に格納。
long load(string path,list<string> &listbuf)
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
	return filesystem::file_size(path);
}

info tester(list<string> loadschedule)
{
	info output{0,0};
	long x,size{0};
	string enter;
	list<string> readbuf;
	//loadscheduleの順で読み込む,最初のファイルを読んで先読みキャッシュを待つ時間を設ける。
	//最初のファイル
	if(load(loadschedule.front(),readbuf)<0){
		cout << "load " << loadschedule.front() << " fail"<< endl;
		return output;
	}
	loadschedule.pop_front();
	//テスターにキャッシュが溜まったことを確認させる
	cout << "キャッシュが貯まったらEnterを押してください。";
	while(cin.get()!='\n'){
	    continue;
	}
	//現在時刻をセットし、リストのファイルを順次読み込む。
	auto start = chrono::high_resolution_clock::now();
	for(auto itr=loadschedule.begin();itr!=loadschedule.end();itr++){
	    x = load(*itr,readbuf);
	    if(x<0){
		cout << "load " << *itr << " fail"<< endl;
		return output;
	    }
	    size += x;
	}
	//読み込み後の時刻との差分を返す。
	auto finish = chrono::high_resolution_clock::now();
	output.time = std::chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
	output.size = size;
	return output;
}

int main(int argc,char* argv[])
{
	list<string> similarschedule;
	list<string> randomschedule;
	list<string> readbuf;
	string enter;
	info ss,rs;
	
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
	cout << "similar: [" << ss.time << " ns / " << ss.size << "byte = "
	    << ss.time / ss.size << "ns/bytes]" << endl;
	cout << "random: [" << rs.time << " ns / " << rs.size << "byte = "
	    << rs.time / rs.size << "ns/bytes]" << endl;
	return 0;
}
