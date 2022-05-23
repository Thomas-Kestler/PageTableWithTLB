# CXX Make variable for compiler, Thomas Kestler 
CXX=g++
# Make variable for compiler options
#	-std=c++11  C/C++ variant to use, e.g. C++ 2011
#	-g          include information for symbolic debugger e.g. gdb 
CXXFLAGS=-std=c++11 -g


pagingwithtlb : main.cpp output_mode_helpers.h tracereader.h tracereader.cpp output_mode_helpers.cpp map.h pagelevel.h pagetable.h pagetable.cpp tlb.h
	$(CXX) $(CXXFLAGS) -o pagingwithtlb $^



clean :
	rm *.o

