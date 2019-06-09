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

using namespace std;
//using namespace boost;
	
class custombuf:
    public streambuf
	{
	public:
		custombuf(string& target): target_(target) {
			this->setp(this->buffer_, this->buffer_ + bufsize - 1);
		}

	private:
		string& target_;
		enum { bufsize = 8192 };
		char buffer_[bufsize];
		int overflow(int c) {
			if (!traits_type::eq_int_type(c, traits_type::eof()))
			{
				*this->pptr() = traits_type::to_char_type(c);
				this->pbump(1);
			}
			this->target_.append(this->pbase(), this->pptr() - this->pbase());
			this->setp(this->buffer_, this->buffer_ + bufsize - 1);
			return traits_type::not_eof(c);
		}
		int sync() { this->overflow(traits_type::eof()); return 0; }
	};

void writeToFile(const string& name, const string& content, bool append = false) {
    ofstream outfile;
    if (append)
        outfile.open(name, ios_base::app);
    else
        outfile.open(name);
    outfile << content;
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
	if (argc != 2)
	{
	   printf("You have to provide a command in the following format:\n\t isar_pipeline <queryDB_file.fasta>\n");
	   return 0;
	}
	query = argv[1];
	if (query == "queryDB" || boost::starts_with(query, "isar"))
	{
	   printf("'queryDB' and the words starting with 'isar' are preserved file names.\nPlease consider changing your file name and run the search again.\nGood luck!");
	   return 0;
	}
	
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	
	
	//get the list of query headers
	queryCount = 0;
	custombuf   sbuf(isar_pair_queries);
	
	if (ostream(&sbuf) << ifstream(query).rdbuf() << flush) {
		boost::char_separator<char> sep{">"};
		tokenizer tok{isar_pair_queries, sep};
		for (const auto &q : tok){
			temp = q;
			boost::trim(temp);
			writeToFile("query_files/isar.q"+to_string(++queryCount)+".fasta",">"+temp);
			
		}
	}else {
		cout << "failed to read the query file.\n";
		return 0;
	}
	
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}
