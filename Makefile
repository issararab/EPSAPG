
#Make file to compile software components, generate executables, and moving them to the bin folder

all:
	#for version `GLIBCXX_3.4.21' not found
	export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/usr/local/lib64:/usr/lib64
	g++ -o isarpipeline -std=c++0x isar_pipeline_v2.cpp utils.cpp custombuf.cpp -pthread
	rm -rf bin
	mkdir bin
	mv isarpipeline bin/.
	
	
