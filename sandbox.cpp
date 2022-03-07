#include <iostream>
#include <fcntl.h> 
#include <string>

using namespace std;

class stdobj{
public:
    const stdobj* base;
    const stdobj* parent;
    stdobj(stdobj* parent){
	this->parent=parent;
	if(parent==NULL){
	    base=this;
	}else{
	    base=parent->base;
	}
    }
};

class human:public stdobj{
private:
    string name;
public:
    human(stdobj* parent,string name):stdobj(parent){
	this->name=name;
    }
    void print(){
	cout << "-----------------" << endl;
	cout<<"this name is " << name << endl;
	if(parent){
	    cout<<"parent name is " << ((human*)parent)->name << endl;
	}
	if(base){
	    cout<<"base name is " << ((human*)base)->name << endl;
	}
	cout << "-----------------" << endl;
    }
};

int main(){
    human a = human(NULL,"masahiro");
    human b = human(&a,"azumi");
    human c = human(&b,"taiki");
    human d = human(&c,"yuta");
    a.print();
    b.print();
    c.print();
    d.print();
    return 0;
}
