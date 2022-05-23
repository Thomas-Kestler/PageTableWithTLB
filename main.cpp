
//main.cpp Thomas Kestler 
#include <iostream>
#include <fstream> 
#include <string.h>
#include <cstring>
#include <string>
#include <exception>
#include <ctype.h>

#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <cstdio>
#include <cctype>
#include <vector>
#include <map>
#include "output_mode_helpers.h"
#include "pagetable.h"
#include "tracereader.h"
#include "map.h"
#include "pagelevel.h"
#include "tlb.h"

using namespace std;
#define BADFLAG 3;//used as an exit code in case trace file can't be opened
#define MRUARYSIZE 10;


void getOptionalArguments(){//not being used
    
}

unsigned int getVPNfromAT(map<unsigned int, unsigned int> lruMap, unsigned int accessTime){//given an access time, look for a vpn in the LRU map that has that access time
//inputs:map of VPN to access time, and access time
    auto mruIter = lruMap.begin();//used to iterate through the map until the access time is found
    while(mruIter != lruMap.end()){
        if(mruIter->second==accessTime){
            //cout<<" newOldest. ";
            return mruIter->first;//returns the VPN associated with the input access time
        }
        mruIter++;
        
    }
    return -1;//this is returned if the access time was not found in the map

}

unsigned int findOldestInLRU(map<unsigned int, unsigned int> lruMap, unsigned int curIndex){//finds the value in the tlbLru map with the lowest('oldest') access time.
//lruMap holds the LRU map for the TLB object, and curIndex is the current value of MRUindex which tracks the highest('newest') access time 
    unsigned int curOldest=curIndex;
    auto mruIter = lruMap.begin();
    while(mruIter != lruMap.end()){
        if(mruIter->second<curOldest){//iterates through the map, updating curOldest every time a lower access time is found
            //cout<<" newOldest. ";
            curOldest=mruIter->second;
        }
        mruIter++;
        
    }
    return curOldest;//returns final value of the oldest access time
}

unsigned int findNextOldestInLRU(map<unsigned int, unsigned int> lruMap, unsigned int curIndex, unsigned int oldest){//this finds the value with the 'next oldest' address in the lru map.
//this function is the same as the above but it only sets the curOldest value if the access time value exceeds the 'oldest' parameter, which is the oldest access time in the LRU map
    unsigned int curOldest=curIndex;
    auto mruIter = lruMap.begin();//iterator to go through each mapping in the lru Map(mapping VPN to access time)
    while(mruIter != lruMap.end()){
        if(mruIter->second<curOldest){
            if(mruIter->second>oldest){
                curOldest=mruIter->second;//this will be the 'next oldest' value in the LRU
            }//the idea of this method is to not get the absolute lowest access time, in the case that VPN is not in the TLB. this will instead return the lowest access time that is still larger than a given range.
            else{
                //don't update curOldest 
                
            }
        }
        mruIter++;
    }
    return curOldest;
}

//test case from sample output for tlb caching:
// ./pagingwithtlb -n 20 -c 2 -o v2p_tlb_pt trace.tr 4 4 12

//./pagingwithtlb trace.tr 2 12 2 -o virtual2physical
// ./pagingwithtlb -n 4800 -c 32 -o v2p_tlb_pt trace.tr 8 6 10
// ./pagingwithtlb -o v2p_tlb_pt -n 4800 -c 32 trace.tr 8 6 10
// ./pagingwithtlb trace.tr 8 8 8 -c 10
//g++ -std=c++11 -g -o pagingwithtlb main.cpp output_mode_helpers.h tracereader.h tracereader.cpp output_mode_helpers.cpp map.h pagelevel.h pagetable.h pagetable.cpp
int main(int argc, char **argv){
    try{//simple try catch in case the main fails to load an input file
    
    //handling of optional command line arguments
    int optionalCount=0;//increments when optional arguments are discovered, and the final value is used to calculate where to start looking for mandatory arguments
    //int firstOptArgIndex=0;//this is used later to prevent optional argument values from being included in the total bits in the page table
    
    int cacheCapacity=0;//will be set if an 'c' opt arg is processed
    
    char *outputMode;//will be set appropriately if an 'o' optarg is processed
    
    OutputOptionsType *outputOptions = new OutputOptionsType();//object to hold boolean values for output modes
    outputOptions->summary=true;//summary is the default output mode. this will be changed if another output mode is specified in the arguments
    
    struct PageTable *pagetable = new PageTable();//initialize the page table that will be used throughout the program
    struct TLB *TLB_CACHE = new TLB();//initialize TLB cache. won't be used if the size is 0
    TLB_CACHE->size=0;//initial value prevents TLB from being used unless the arguments specify a size
    int numAddressesToProcess=0;//will be set if an 'n' opt arg is processed
    bool optArgsFirst=true;

    //check if argv[1] is a file or just a string with something like '-x'. if its not in the format -x, then assume the file is the first argument and try to open it.
    int Option=0;//used to hold getopt returned values for opt args
    int idx=0;//used to determine the location of the mandatory arguments after optional arguments are processed
    int curArg=0;//this will hold integer or string values entered in optional arguments
    
    bool TLBcaching = false;//additional variable to prevent TLB from being used if size is zero


    while ( (Option = getopt(argc, argv, "n:o:c:")) != -1) {//handle optional command arguments
        switch (Option) {
        case 'n': /* Number of addresses to process */
            // optarg will contain the string following -n 
            optionalCount++;
            
            for(int i=0;i<argc;i++){//loop through argv to find the associated # of addresses
                if(strcmp(argv[i],"-n")==0){
                    
                    
                    curArg=atoi(argv[i+1]);
                    numAddressesToProcess=curArg;
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    
                }
            }
           
            if(curArg<=0){//prevents number of 0 or less from being used as # of addresses
                cout<<"number of addresses to process should be 1 or greater"<<endl;
                exit(0);
            }

            break;
        case 'o': /* produce map of pages */
            // optarg contains the output method…
            outputMode=optarg;
            outputOptions->summary=false;//set default output mode to false unless 'summary' is specified
            
            if(strcmp(outputMode,"bitmasks")==0){//set output mode if specified
                            
                            outputOptions->bitmasks=true;
                            optionalCount++;
                        }
                        else if(strcmp(outputMode,"virtual2physical")==0){
                            outputOptions->virtual2physical=true;
                            optionalCount++;
                        }
                        else if(strcmp(outputMode,"v2p_tlb_pt")==0){
                            outputOptions->v2p_tlb_pt=true;
                            optionalCount++;
                        }
                        else if(strcmp(outputMode,"vpn2pfn")==0){
                            outputOptions->vpn2pfn=true;
                            optionalCount++;
                        }
                        else if(strcmp(outputMode,"offset")==0){
                            outputOptions->offset=true;
                            optionalCount++;
                        }
                        else if(strcmp(outputMode,"summary")==0){
                            outputOptions->summary=true;
                            optionalCount++;
                        }
                        else{
                            cout<<"Invalid output mode argument, exiting"<<endl;
                            exit(0);
                        }
            break;
        case 'c': /* tlb cache size */
            
            optionalCount++;
           TLBcaching = true;
            
            for(int i=0;i<argc;i++){
                if(strcmp(argv[i],"-c")==0){//loop through argv to find the associated cache size
                    
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    else{
                    
                        if(argv[i+1]<0)//handles possibility of a negative cache size, which would cause an error
                        {cout<<"Cache capacity must be a number, greater than or equal to 0"<<endl;}
                        else {
                            curArg=atoi(argv[i+1]);
                            TLB_CACHE->size=curArg;
                            
                        }

                    }
                }
            }
            
            if(curArg<0){
                cout<<"Cache capacity must be a number, greater than or equal to 0"<<endl;
                exit(0);
            }
            
            break;
        default://for some reason this default case never gets reached, even if optargs are at the end after the mandatory ones.
            // print something about the usage and die…
            
            optArgsFirst=false;
                
                    cout<<"b_invalid optional argument detected"<<endl;
                    int badflagerror = BADFLAG;
                    exit(badflagerror); // BADFLAG is an error # defined in a header
                
            }
        }
    /* first mandatory argument, optind is defined by getopt */
    idx = optind;
    
    /* argv[idx] now has the first mandatory argument: the trace file
    path*/
    /* argv[idx+1] through argv[argc-1] would be the arguments for
    specifying the number of bits for each page table level, one number
    per each level */
    FILE *ifp;
    unsigned int fileIndex;
    for(int i=1;i<argc;i++){//loops through argv to find the input file, which helps find where the mandatory arguments start
        if ((ifp = fopen(argv[i],"rb")) == NULL) {
        //keep looking until the correct index is found
        }
        else{
            fileIndex=i;
            
            break;
        }
    }

    
    if(fileIndex==1){
        optArgsFirst=false;//if the file index is the first argument, we can assume the optional arguments are at the end rather than the beginning
    }
    
    idx=fileIndex;
    unsigned int pageSizeForSummary=0;//this will hold the # bytes per page in the table
    unsigned int offsetShift;//will hold the bit shift value for the offset of each address
    
    if (idx < argc)
    {

        unsigned int curShift = ADDRESS_SPACE;//sets this to 32, this will be subtracted by the # of bits for the current level to get the cur bitShift value
        int mandArgCount;
        unsigned int vpn_bits=0;
        if(!optArgsFirst){
            mandArgCount=argc-idx-(optionalCount*2)-1;//calculates the number of levels in the page table, used to initilize the array attributes in the table
        }
        else{//if optargs are before mandatory args
            mandArgCount=argc-(optionalCount*2)-2;
        }
        
       
        
        pagetable->bitmask=new unsigned int[mandArgCount];//initialize attributes for page table
        pagetable->bitShift=new unsigned int[mandArgCount];
        pagetable->entryCount=new int[mandArgCount];
        
        bool stopLoop=false;//this will be set to true if optional arguments are at the end and cause an error
        int levelCountIndex=0;
        for(int i=idx+1;i<argc;i++){
            
            if(strcmp(argv[i],"-o")==0 || (strcmp(argv[i],"-n")==0) || (strcmp(argv[i],"-c")==0)){
               
                break;//stop if optional arguments are at the end
                stopLoop=true;
            }
            
            int index=i-fileIndex-1;//index of first mand argument after the trace file
            
            if(stopLoop==false){
                unsigned int curBits = atoi(argv[i]);//# bits in current page table level
                
                if(curBits<=0){
                    
                    cout<<"Level "<<index<<" page table must be at least 1 bit";
                    
                    exit(0);
                }
                levelCountIndex++;//whenever a new level is found, update the count of levels in the table
                pagetable->levelCount=levelCountIndex;
                pagetable->entryCount[index]= pow(2,curBits);   //entry count for each level is 2^n where n is # bits for cur level
                
                curShift-=curBits;//calculate shift for current level of table
                vpn_bits+=curBits;//will hold total # of bits in the full VPN
                
                pagetable->bitShift[index]=curShift; 
                
                pagetable->bitmask[index]= ((1<<curBits)-1)<<pagetable->bitShift[index];//construct bitmask for current level
            }
            else{
                i=argc;//forces the loop to stop incase the break statement doesn't work
            }
        }
        
        offsetShift=ADDRESS_SPACE-vpn_bits;//ex for 32 bit addr space, 3 levl table with 8 bits per level. vpnbits=24, so offsetshift=32-24=8
        pagetable->offsetShift=offsetShift;//used in offset output mode later
        pageSizeForSummary=pow(2,offsetShift);//calculate # of bytes per page
        
            unsigned int offsetMask = ((1<<curShift)-1);//final value of curShift would be 32-total# bits for VPN
            
            pagetable->offsetMaskValue=offsetMask;//bit mask to get offset of each virtual address
            
            if(vpn_bits>28){//if total # of bits in page table is too big, exit program
                cout<<"Too many bits used in page tables";
                
                exit(0);
            }
            else{
                
            }
    }
    
    
    

    //bitmask output mode
    

    if(outputOptions->bitmasks==true){//print the bitmasks and exit program if the bitmask output mode is set
        
        report_bitmasks(pagetable->levelCount,pagetable->bitmask);
        exit(0);
    }
    
    
    struct pageLevel *firstLevel = newLevel(pagetable, pagetable->entryCount[0], 0);//initialize the first level of the page table and set the level pointer in the table
    pagetable->Level=firstLevel;
   

    //code to process file taken from tracereader.cpp
    	        /* trace file */
    unsigned long i = 0;  /* instructions processed */
    p2AddrTr trace;	/* traced address */

    /* check usage */
    /*if(argc != 2) {
        fprintf(stderr,"usage: %s input_byutr_file\n", argv[0]);
        exit(1);
    }*/
    
    /* attempt to open trace file */
    if ((ifp = fopen(argv[idx],"rb")) == NULL) {
        fprintf(stderr,"cannot open %s for reading\n",argv[idx]);
        exit(1);
    }
    else{
        //cout<<"successfully opened file."<<endl;
    }
    bool TLBmiss=true;//used to indicate a TLB/page table hit/miss in some of the output modes
    bool TLBhit=true;
    bool tablemiss=true;
    bool tablehit=true;
    
    unsigned long frameNum=0;//used to track the current frame #
    unsigned int fullMask=0;
    unsigned int fullShift = pagetable->bitShift[0];//fullmask and fullshift are used to get offsets for offset output mode
    if(pagetable->levelCount>1){
        fullShift=ADDRESS_SPACE-pagetable->bitShift[0];//full shift is calculated differently if the page table has more than one level
    }
    for(int i=0;i<pagetable->levelCount;i++){
        fullMask+=pagetable->bitmask[i];//construct full mask by adding up the bitmasks for each level
    }
    
    unsigned int physAddress;//will hold current physical address
    // ./pagingwithtlb -n 5 -c 2 -o v2p_tlb_pt trace.tr 4 4 12
    int MRUIndex=1;//used to track access time in the LRU map of the TLB
    int MRU_size=MRUARYSIZE;//sets size of LRU map to 10
    /*if(TLB_CACHE->size<10){
        MRU_size=TLB_CACHE->size;
    }*/
    
    while (!feof(ifp)) {
        /* get next address and process */
        if (NextAddress(ifp, &trace)) {
        //AddressDecoder(&trace, stdout);
        unsigned int virtualAddress = trace.addr;//current virtual address
        
        //process here
        
        unsigned int curOffset;
        curOffset =virtualAddressToPageNum(virtualAddress,pagetable->offsetMaskValue,0);//offset for current virtual address
        if(outputOptions->offset==true){//handle offset output mode
            hexnum(curOffset);
        }
        else{//in offset output mode, don't need to do any of this with the page table
            //check if mapping is already in the TLB

                unsigned int fullVPN;
                fullVPN = virtualAddressToPageNum(virtualAddress,fullMask,fullShift);//get the entire VPN for the current address
                unsigned int *vpnAry = new unsigned int[pagetable->levelCount];//this is used for the vpn2pfn output mode

                if(TLB_CACHE->size!=0){//only do TLB caching if the size is set by an optional argument '-c'
                
                    map<unsigned int,unsigned int>::iterator tlbMapping = TLB_CACHE->tlbCache.find(fullVPN);//get mapping for current address if it is in the TLB, will point to end() if not in TLB
                    if(tlbMapping==TLB_CACHE->tlbCache.end()){//if mapping not found in TLB, insert it into the TLB
                    
                        //indicate TLB miss, then insert mapping to TLB
                        TLBmiss=true;
                        TLBhit=false;
                        pagetable->TLBmisses+=1;
                        
                        if(TLB_CACHE->tlbCache.size()>=TLB_CACHE->size){//if TLB is full, do the LRU replacement here

                            unsigned int curOldest=MRUIndex;//this is the highest current access time
                            
                            unsigned int oldestAC = findOldestInLRU(TLB_CACHE->tlbLru,curOldest);//this will return an access count/time to a VPN to be removed from the TLB
                            
                            unsigned int oldestVPN=getVPNfromAT(TLB_CACHE->tlbLru,oldestAC);//get the VPN from the LRU map corresponding to the access time
                            if(TLB_CACHE->tlbLru.find(oldestVPN)==TLB_CACHE->tlbLru.end()){
                                //cout<<"couldn't find oldest. ";
                            }

                            bool victimFound=true;//used to indicate if a victim from the LRU is located that is also in the TLB
                            int whileCount=0;//count for while loop
                            if(TLB_CACHE->tlbCache.find(oldestVPN) == TLB_CACHE->tlbCache.end()){
                                while(TLB_CACHE->tlbCache.find(oldestVPN) == TLB_CACHE->tlbCache.end()){//keep searching the LRU until we find the oldest address that is also present in the TLB cache map
                                    //
                                    oldestAC = findNextOldestInLRU(TLB_CACHE->tlbLru,MRUIndex,oldestAC);
                                    
                                    oldestVPN = getVPNfromAT(TLB_CACHE->tlbLru, oldestAC);//get key which is the VPN to search in the TLB
                                    whileCount++;
                                    if(whileCount>10){
                                        victimFound=false;//indicate something has gone wrong: only 10 values in the LRU, so if none are in the TLB, an error has occurred
                                        break;
                                    }

                                }
                            }

                            if(victimFound==false){
                                //cout<<"something is wrong."<<endl;
                            }
                            else{
                                if(TLB_CACHE->tlbCache.find(oldestVPN)==TLB_CACHE->tlbCache.end()){//
                                    
                                }
                                
                                TLB_CACHE->tlbCache.erase(TLB_CACHE->tlbCache.find(oldestVPN));//find the appropriate mapping in the TLB to remove
                            }

                            TLB_CACHE->tlbCache.insert(pair<unsigned int, unsigned int>(fullVPN,tlbMapping->second));//insert current mapping into the TLB
                            
                            
                        }
                        else{//if not full, add to the TLB and to the most recently accessed queue/list/whatever structure

                            TLB_CACHE->tlbCache.insert(pair<unsigned int, unsigned int>(fullVPN,tlbMapping->second));//tlbMapping->second contains the value(PFN)
                            
                        }
                        
//this used to check if >= MRU_size-1, change back if errors come up
                            

                            //check if the lru map is full
                        map<unsigned int,unsigned int>::iterator findLruMapping = TLB_CACHE->tlbLru.find(fullVPN);
                        if(findLruMapping!=TLB_CACHE->tlbLru.end()){//check if the current address is already in the LRU. if it is, then update the access time to indicate it is the newest access
                            //cout<<"lru size: "<<TLB_CACHE->tlbLru.size()<<". ";
                            findLruMapping->second=MRUIndex;
                            //if we are only modifying an entry in the LRU instead of adding a new mapping, don't remove anything. 
                            
                        }
                        else{//if address isn't in the LRU, add it with the current access time as normal
                            
                            if(TLB_CACHE->tlbLru.size()>=MRU_size){//this is done before inserting new addresses since the LRU map should never have more than 10 addresses in it

                                //LRU map is full, need to remove entry with oldest 'access time'
                                
                                unsigned int oldestAC = findOldestInLRU(TLB_CACHE->tlbLru,MRUIndex);
                               
                                unsigned int oldestVPN = getVPNfromAT(TLB_CACHE->tlbLru, oldestAC);//get key which is the VPN to search in the TLB
                                if(oldestVPN==-1){//-1 means the function didn't work
                                    //cout<<" couldnt find value to remove from mru. ";
                                }
                                TLB_CACHE->tlbLru.erase(oldestVPN);
                                
                                
                            }
                            else{
                                
                            }

                            TLB_CACHE->tlbLru.insert(pair<unsigned int, unsigned int>(fullVPN,MRUIndex));
                            //cout<<"lru size: "<<TLB_CACHE->tlbLru.size()<<". ";
                            
                        }

                        //end checking if lru map is full
                            MRUIndex+=1;//increment the access time counter

                    }
                    else{//indicate TLB Hit: tlbMapping will hold the PFN corresponding to the VPN
                        TLBmiss=false;
                        TLBhit=true;
                        
                        pagetable->TLBhits+=1;
                        map<unsigned int,unsigned int>::iterator findLruMapping = TLB_CACHE->tlbLru.find(fullVPN);
                        if(findLruMapping!=TLB_CACHE->tlbLru.end()){//check if the current address is already in the LRU. if it is, then update the access time to indicate it is the newest access
                            findLruMapping->second=MRUIndex;
                            
                        }
                        else{//if address isn't in the LRU, add it with the current access time as normal
                            TLB_CACHE->tlbLru.insert(pair<unsigned int, unsigned int>(fullVPN,MRUIndex));
                            
                        }
                        MRUIndex+=1;
                    }

            }
            else{//this doesn't really need to be here, but I included it for clarity 
                
            }
                
            //2. if TLB miss(always in case of tlb size=0), then do a page table walk(pagelookup) to see if mapping is present
            
            Map *curMapping = pageLookup(pagetable,virtualAddress);
           
            if(!curMapping){//wanted to check if mapping is valid, but that causes seg faults
                //indicate page table miss, call page insert
                if (TLBcaching==true && TLBhit==true){
                    //don't do the page table walk if there was a TLB hit 
                }
                else{
                    tablemiss=true;//indicate page table miss if the page table has no mapping for the current address
                    
                    tablehit=false;
                    
                    pagetable->pagemisses++;
                }
                
                bool inserted = pageInsert(pagetable,virtualAddress,frameNum);//insert mapping into page table
                
                //if(inserted==true){
                    Map *mapping=pageLookup(pagetable,virtualAddress);//this gets the current mapping to be used for the vpn2pfn output mode
                    if(outputOptions->virtual2physical==true){
                        
                        physAddress = frameNum<<(pagetable->offsetShift);
                        physAddress += curOffset;//the PFN for the current virtual address
                        report_virtual2physical(virtualAddress,physAddress);//print virtual to physical address mapping for this output mode
                    }
                    else if(outputOptions->vpn2pfn==true){
                        for(int i=0;i<pagetable->levelCount;i++){
                            vpnAry[i]=virtualAddressToPageNum(virtualAddress,pagetable->bitmask[i],pagetable->bitShift[i]);
                        }
                        report_pagemap(pagetable->levelCount,vpnAry,mapping->PFN);//print the VPN for each page for the current virtual address
                    }
                    
                    frameNum++;//increment frame number
                    
                //}
                if(inserted==false){//pageInsert returns false if the page was not inserted correctly
                    //cout<<"something went wrong when inserting page with address "<<virtualAddress<<endl;
                }
            }
            else{
                //indicate page table hit
                if (TLBcaching==true && TLBhit==true){//don't indicate a page table hit if there was a TLB hit

                }
                else{
                    tablemiss=false;
                    tablehit=true;
                    pagetable->pagehits++;
                }
                
                physAddress = curMapping->PFN<<(pagetable->offsetShift);//get current physical address
                physAddress += curOffset;
                if(outputOptions->virtual2physical==true){//need to handle this output mode in case of a page table hit as well
                        
                    report_virtual2physical(virtualAddress,physAddress);
                }
                else if(outputOptions->vpn2pfn==true){//v2p2pfn output mode, same as above
                    for(int i=0;i<pagetable->levelCount;i++){
                            
                            vpnAry[i]=virtualAddressToPageNum(virtualAddress,pagetable->bitmask[i],pagetable->bitShift[i]);
                        }
                        report_pagemap(pagetable->levelCount,vpnAry,curMapping->PFN);
                }
                
                
            }

       // }
        //else{//if TLB hit is true

        //}
            if(outputOptions->v2p_tlb_pt==true){//handle v2p tlb pt output mode
                
                Map *curMapping = pageLookup(pagetable,virtualAddress);
                //construct the physical address whether there was a table hit or not
                if(tablehit==false){
                    physAddress = curMapping->PFN<<(pagetable->offsetShift);
                    physAddress += curOffset;
                    report_v2pUsingTLB_PTwalk(virtualAddress,physAddress,TLBhit,tablehit);
                }
                else{//only try to get PFN if the mapping was found in the page table
                physAddress = curMapping->PFN<<(pagetable->offsetShift);
                physAddress += curOffset;
                report_v2pUsingTLB_PTwalk(virtualAddress,physAddress,TLBhit,tablehit);
                }
                }
        }
        
        i++;
        if(numAddressesToProcess!=0){//if n is not provided, then don't limit the amount of addresses to process
            if(i>=numAddressesToProcess){
                //after printing the first 'n' addresses(if that optional argument was provided), exit the loop and stop processing addresses
                break;
            }
        }

        if ((i % 100000) == 0)
        fprintf(stderr,"%dK samples processed\r", i/100000);
        }
    }//end of while loop	
    pagetable->bytesused+=(sizeof(pagetable));
    
    if(outputOptions->summary==true){
        report_summary(pageSizeForSummary,pagetable->TLBhits,pagetable->pagehits,i,frameNum,pagetable->bytesused);
        //cout<<"TLB misses: "<<pagetable->TLBmisses<<" with # of elements in TLB: "<<TLB_CACHE->tlbCache.size()<<endl;
    }
    /* clean up and return success */
    //cout<<"done with processing addresses"<<endl;
    fclose(ifp);
    delete(pagetable);//prevent memory leak

    }
    catch (std::exception const &exc){//handle exceptions caught during running program
        std::cerr << "Exception caught "<< exc.what() << "\n";
    }
    catch (...){
        std::cerr << "Unknown exception caught\n";
    }
}