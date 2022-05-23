#ifndef PAGELEVEL_H
#define PAGELEVEL_H
//Thomas Kestler : Page Level declaration for the page table
#include <iostream>
#include <fstream>
#include <string.h>
#include <cstring>
#include <string>
#include <exception>

#include <stdlib.h>
#include <cstdlib>
#include <cctype>
#include <list>
#include <map>
#include <vector>
#include "pagetable.h"
#include "map.h"

using namespace std;

//struct Map;

typedef struct pageLevel{
    bool isLeaf=false;//indicate if current level is a leaf level
    int depth=-1;//initial depth value to be used in error checking
    struct PageTable *table;//pointer to page table
    vector<pageLevel*> nextLevel = vector<pageLevel*>(1024);//array/vector of Level pointers with initial size 1024. this will be changed according to the # bits in the current level
    
    struct vector<struct Map*> MapAry = vector<struct Map*>(1024);//vector of struct Map pointers to be used for leaf levels/nodes
    pageLevel(int curDepthSize, int d){
        depth=d;
        nextLevel.resize(curDepthSize);//constructor that resizes the nextlevel vector appropriately
        
    }
    pageLevel(){
        //nextLevel = new pageLevel[1024];
    }

}pageLevel;

struct pageLevel *newLevel(PageTable *tempTable, int curSize, int d);//used to initialize a new page level


#endif