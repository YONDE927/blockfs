#ifndef BASE_H
#define BASE_H
#include <iostream>
#include <fcntl.h> 
#include <string>

using namespace std;

class stdobj{
protected:
    const stdobj* base;
    const stdobj* parent;
public:
    stdobj(stdobj* parent){
	this->parent=parent;
	if(parent==NULL){
	    base=this;
	}else{
	    base=parent->base;
	}
    }
};

#endif
