
#Make file to compile software components, generate executables, and moving them to the bin folder

all:
	g++ -o epsapg -std=c++0x epsapg.cpp utils.cpp custombuf.cpp -pthread
	rm -rf bin
	mkdir bin
	mv epsapg bin/.
	
	
