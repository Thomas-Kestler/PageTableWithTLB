#ifndef MAP_H
#define MAP_H
#include "pagetable.h"
#include <vector>
//Thomas Kestler  - Map structure used for NextLevel, mapping VPN to PFN
typedef struct Map{

    unsigned int VPN;
    unsigned int PFN;
    bool valid=false;
    Map(){
        
    }
    Map(unsigned int v, unsigned int p, bool va){//constructor to initialize attribute accordingly
        VPN=v;
        PFN=p;
        valid=va;
    }
}Map;




#endif