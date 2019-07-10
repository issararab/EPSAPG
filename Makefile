
#compiling the pipeline and putting the executable in bin

all:
	#for version `GLIBCXX_3.4.21' not found
	export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/usr/local/lib64:/usr/lib64
	g++ -o isarpipeline -std=c++0x isar_pipeline_v2.cpp -pthread
	rm -rf bin
	mkdir bin
	mv isarpipeline bin/.
	
	