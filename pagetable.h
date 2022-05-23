//page tree
#ifndef PAGETABLE_H
#define PAGETABLE_H
//Thomas Kestler : page table declaration
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
#include <iterator>
#include "pagelevel.h"
#include "map.h"

//struct pageLevel;
using namespace std;
#define ADDRESS_SPACE 32 /* 32 bit address space */

typedef struct PageTable
{
    //attributes: PageTable, Level, Map
    struct pageLevel *Level;//pointer to arbitrary level
    int levelCount;//number of levels in the table
    unsigned int offsetShift;
    unsigned int offsetMaskValue;//holds shift and mask for offsets
    //int sizeIndex;
    int pagehits=0;
    int TLBhits=0;//these variables hold the # of hits/misses for the tlb and table
    int bytesused=0;
    int pagemisses=0;
    int totalPageEntries=0;
    int TLBmisses=0;
    unsigned int *bitmask;
    unsigned int *bitShift;
    int *entryCount;//holds number of pages in each level, will be initialized as an array. as will be bitmask and bitShift
    
    

    PageTable(){//default constructor
        
    }

    /*PageTable(int levels, int index){//this constructor is not used
       // Level = level;
        //Level->depth=0;//GETTING INCOMPLETE TYPE ERROR HERE
        //Level->table=this;//GETTING INCOMPLETE TYPE ERROR HERE
        levelCount=levels;
        sizeIndex=index;
        bitmask = new unsigned int[sizeIndex];
        bitShift = new unsigned int[sizeIndex];
        entryCount = new int[sizeIndex];
    }*/
 
}PageTable;



long int calcTotalBytesUsed(PageTable *tempTable,pageLevel *root,int curLevelEntryCount);//not fully implemented

unsigned int virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift);

bool pageInsert(PageTable *pageTableLevel, unsigned int virtualAddress, unsigned int frame);

struct Map *pageLookup(PageTable *pageTableLevel, unsigned int virtualAddress);//uses the Map structure defined in map.h
 

#endif