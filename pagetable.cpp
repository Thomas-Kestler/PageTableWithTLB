//Thomas Kestler , page table implementation
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
#include "pagetable.h"
#include "pagelevel.h"
#include "output_mode_helpers.h"
#include "map.h"
using namespace std;

struct pageLevel *newLevel(PageTable *tempTable, int curSize, int d){//use this to create each level and hold attributes for each level. takes page table pointer, the size for the current level, and the current depth
    
    struct PageTable *temp = tempTable;
    struct pageLevel *page = new pageLevel(curSize, d);    
    return page;

}



unsigned int virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift){
    //takes an input virtual address, applies a mask and shift and returns a VPN
    
    unsigned int masked = mask & virtualAddress;//uses bitwise and to apply the bitmask
    
    unsigned int result = masked>>shift;//shift the masked address by the appropriate # of bits to get the result VPN

    
    return result;
}

long int calcTotalBytesUsed(PageTable *tempTable,pageLevel *root,int curLevelEntryCount){//this method is not being used and it probably would cause errors anyway if I tried to use it
    PageTable *pagetable=tempTable;
    pageLevel *curLevel=root;
    int count=curLevelEntryCount;
    long int totalBytes=0;
    //for(int j=0;j<pagetable->levelCount;j++){

        for(int i=0;i<count;i++){
            if(!curLevel->nextLevel[i]){
                continue;
            }
            else{
                totalBytes+=(count*4*2);
                totalBytes=calcTotalBytesUsed(pagetable,curLevel->nextLevel[i],pagetable->entryCount[curLevel->nextLevel[i]->depth]);
            }
            totalBytes+=sizeof(curLevel);
        }


    //}
    
    totalBytes+=sizeof(pagetable);
    
    return 0;
}

struct Map *pageLookup(PageTable *pageTableLevel, unsigned int virtualAddress){
//look for a VPN to PFN mapping for a given virtual address and page table pointer, and return the mapping if it is in the page table
    
    PageTable *temp = pageTableLevel;
    pageLevel *tempLevel = temp->Level;//set the level pointer for this pagetable 
    int curLevel=0;//tracks the current depth
    bool vpnFound=false;//will be set if the mapping for the VPN is found
   
    for(int curLevel=0;curLevel<temp->levelCount;curLevel++){//loop through each level of the page table to look for the mapping
        vpnFound=false;//reset on every level
        unsigned int mask = temp->bitmask[curLevel];
        unsigned int shift = temp->bitShift[curLevel];
        unsigned int curVPN = virtualAddressToPageNum(virtualAddress,mask,shift);//calculate these values only at the start and when we advance to a new level

            tempLevel->nextLevel.resize(temp->entryCount[curLevel]);
            //hexnum(curVPN);
            if(tempLevel->nextLevel[curVPN]==NULL){//don't want to advance to the next level if there is no level currently there
            
                if(curLevel>=temp->levelCount-1){//if node is a leaf node, then check for the mapping in MapAry
                   
                    if(!tempLevel->MapAry[curVPN]){//if no mapping is found, return Null to indicate a page table miss
                        return NULL;
                    }
                    else{
                        
                    return tempLevel->MapAry[curVPN];//if the mapping exists, return it
                    }
                }
                else{
                
                return NULL;//if the current level is not a leaf and the VPN was not found, then there must not be a mapping in the array for that virtual address
                }
                
            }
            else{//in the case if the VPN was found for the current level
               
                vpnFound=true;
                
                if(curLevel>=temp->levelCount-1 || tempLevel->isLeaf==true){//if node is a leaf node
                   
                    return tempLevel->MapAry[curVPN];
                }
                else{//in case when current level is not the last level
                
                tempLevel=tempLevel->nextLevel[curVPN];//advance to the next level if the VPN was found
                
                }
            }
        //}
        if(vpnFound==false){//if the for loop exited without setting this value to true, that means the VPN is not in the page table
           
            return NULL;
        }
    }
    
    return NULL;//if somehow both for loops run fully without ever returning something, something must have gone wrong, so return null
}

bool pageInsert(PageTable *pageTableLevel, unsigned int virtualAddress, unsigned int frame){
    //takes virtual address, page table pointer, and current frame number. uses these to insert a VPN to PFN mapping to the page table for a given virtual address
    //insert address to page table. use virtualAddresstopagenum method to get the portion of the address
  
    //make sure Level pointer of the table is set
    struct PageTable *temp = pageTableLevel;
    struct pageLevel *tempLevel = temp->Level;
    temp->bytesused+=sizeof(tempLevel);//increment total bytes used in the page table
    
    tempLevel->table=temp;//
    
    int curLevel = 0;//keep track of current depth
    if(tempLevel==NULL){//prevent seg fault when accessing depth
        return false;
    }
    
    tempLevel->depth=curLevel;//set current level depth if the level is not null
  
    unsigned int curAdrPortion=virtualAddressToPageNum(virtualAddress,temp->bitmask[tempLevel->depth], temp->bitShift[tempLevel->depth]);//get vpn for current level
   
    for(int curLevel=0;curLevel<temp->levelCount;curLevel++){//BEGIN FOR LOOP
    /*if(tempLevel->isLeaf==false){
       
    }*/
        if(tempLevel->isLeaf==false && tempLevel->nextLevel[curAdrPortion]==NULL){//isLeaf check should prevent any seg faults here, and also prevents the table from making a newlevel when it is a leaf node already
           
            tempLevel->nextLevel[curAdrPortion]=newLevel(temp,temp->entryCount[curLevel+1], curLevel+1);//initialize a new level if the nextlevel for the current VPn is null
            temp->bytesused+=(temp->entryCount[curLevel]*8);//increment total bytes used
           
        }
        
            if(tempLevel->depth>=temp->levelCount-1){//then this is a leaf node, and we should set the map attribute instead of nextLevel
                
                unsigned int fullVPN, fullMask, fullShift;
                fullMask=0;
                    for(int i=0;i<temp->levelCount;i++){
                        fullMask+=temp->bitmask[i];//get the full mask to construct the full VPN
                        
                    }
                    
                    fullShift = temp->bitShift[0];
                    if(temp->levelCount>1){
                        fullShift=ADDRESS_SPACE-temp->bitShift[0];//just like in main, the full shift is calcualted differently if the page table has multiple levels
                    }
                    
                    fullVPN = virtualAddressToPageNum(virtualAddress,fullMask,fullShift);//get full VPN with the full mask and full shift
                    
               
                tempLevel->MapAry.resize(temp->entryCount[curLevel]);//resize the map array to the entry count for the final level if we have reached the final level of the page table
                
                temp->totalPageEntries+=1;//increment total page entries since this is a new level
                if(!tempLevel->MapAry[curAdrPortion]){//if mapping is null, don't check if it is valid, this would be a seg fault
                    tempLevel->MapAry[curAdrPortion]=new Map(fullVPN,frame,true);//insert the entire VPN (virtual address without offset) to the map
                    tempLevel->MapAry[curAdrPortion]->valid=true;//possibly redundant
                    tempLevel->isLeaf=true;
                    
                return true;
                }
                if(tempLevel->MapAry[curAdrPortion]->valid==false){//don't want to replace the mapping if it is already there, only check after determining it is not null!
                tempLevel->MapAry[curAdrPortion]=new Map(fullVPN,frame,true);//insert the entire VPN (virtual address without offset) to the map
                tempLevel->MapAry[curAdrPortion]->valid=true;//possibly redundant
                
                }
                else if(tempLevel->MapAry[curAdrPortion]->valid==true){
                    //don't replace the mapping if this level is set to valid

                }
                
                tempLevel->isLeaf=true;//this step should be completed whether or not the valid flag is false
                return true;
                
            }//this also makes sure we don't add more depths of the table than specified
            else{//this covers all other cases besides when we are at the final level of the table.
               
                tempLevel=tempLevel->nextLevel[curAdrPortion];//advance to the next level of the page table
                temp->bytesused+=sizeof(tempLevel);//update total bytes used and total page entries
                temp->totalPageEntries+=1;
                //set the depth of the new level accordingly since we advanced to the next level
                tempLevel->depth=curLevel+1;
                if(curLevel+1>=temp->levelCount-1){//indicate a leaf level if it is the last level in the table
                    tempLevel->isLeaf=true;
                    
                }
                curAdrPortion=virtualAddressToPageNum(virtualAddress,temp->bitmask[tempLevel->depth], temp->bitShift[tempLevel->depth]);//get the current VPN for the new level
               
            //get new address portion after advancing to the next level
            }
        //}//end of if statment curAdrPortion==i
    }//END FOR LOOP

   
    tempLevel->isLeaf=true;//
    
    return true;//do this if add was successful
}