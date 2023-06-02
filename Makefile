
# Make file will compile the software components, generate executables, 
# and move them to the bin folder to add later to the system envirnment

all:
	# for version `GLIBCXX_3.4.21' not found
	export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/usr/local/lib64:/usr/lib64
	g++ -o epsapg -std=c++0x epsapg.cpp utils.cpp custombuf.cpp -pthread
	rm -rf bin
	mkdir bin
	mv epsapg bin/.
	
	
