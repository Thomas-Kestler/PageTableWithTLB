#ifndef TLB_H
#define TLB_H
#include "pagetable.h"
#include "pagelevel.h"
#include <map>
using namespace std;
//Thomas Kestler 
//TLB cache declaration
typedef struct TLB{
    map<unsigned int, unsigned int> tlbCache = map<unsigned int, unsigned int>();
    unsigned int *recentlyAccessed= new unsigned int[10];
    map<unsigned int, unsigned int> tlbLru = map<unsigned int, unsigned int>();
    //using two std:: maps: one for the tlb cache mapping VPN to PFN, and the tlbLru LRU cache which maps VPN to Access Time
    int size;
    
    TLB(){

    }
    TLB(int cacheSize){
        size=cacheSize;//initialize a new TLB with the appropriate cache size
    }

}TLB;






#endif