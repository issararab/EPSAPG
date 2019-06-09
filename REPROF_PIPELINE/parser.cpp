#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <map>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>         // cast uuid to string

#include <algorithm>
#include <iterator>
#include <sys/types.h>
#include <dirent.h>

using namespace std;
//using namespace boost;
	
typedef std::vector<std::string> stringvec;

void read_directory(const std::string& name, stringvec& v)
{
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}

int main(int argc, char **argv)
{
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    string isar_tuple,isar_pair,isar_pair_queries,single_query_data;
	string query,temp; 
	int queryCount;
	map<string,int> queryDictionary;
	// Create a std::promise object
	promise<void> exitSignal;
	//Fetch std::future object associated with promise
	future<void> futureObj = exitSignal.get_future();
	//multiple threads datastructure
	vector<thread> threads;
	unsigned concurentThreadsSupported = 0;
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	string UUID = "-"+boost::lexical_cast<std::string>(uuid);
	
	stringvec v;
	
	
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	
	for(int i=1; i<=265;i++){
		//read_directory(""+to_string(i)+"/.", v);
		cout << i << "\n";
		//std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
		system(("cp isar.result.q"+to_string(i)+".ascii.pssm ascii_files/.").data());
		system(("mv ascii_files/isar.result.q"+to_string(i)+".ascii.pssm ascii_files/isar.q"+to_string(i)+".pssm").data());
		v.clear();
	
	}
	//system(("mv isar.result.pssm."+to_string(i) +" " +folderNumber+"/.").data());

	printf("\nParsing Complete!\n");
	
	queryDictionary.clear();
	
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}
