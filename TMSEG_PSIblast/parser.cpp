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

void loadingAnimation(future<void> futureObj){
	cout << "Parsing the result" ;
    while (futureObj.wait_for(chrono::milliseconds(1)) == future_status::timeout)
	{
        for (int i = 0; i < 4; i++) {
			cout << ".";
			cout.flush();
            sleep(1);
        }
        cout << "\b\b\b   \b\b\b\b";
	}
}

bool is_file_empty(ifstream& pFile)
{
    return pFile.peek() == ifstream::traits_type::eof();
}

bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return (bool)ifile;
}

void psiBlast(int i) {
	string single_query_data;
	//Check if the query db is empty
	ifstream file("isar.db."+to_string(i));
	if (is_file_empty(file))
	{
		custombuf sbuf3(single_query_data);
		if (ostream(&sbuf3) << ifstream("isar.q."+to_string(i)).rdbuf() << flush) {
			writeToFile("isar.db."+to_string(i),single_query_data);
		}else {
			cout << "failed to read the query file number; "+to_string(i) +".\n";
		}
	}
	system(("makeblastdb -in isar.db."+to_string(i)+" -dbtype prot").data());
	system(("psiblast -query isar.q."+to_string(i)+" -db isar.db."+to_string(i)+" -evalue 10 -out isar.result.q"+to_string(i)+".psiblast -out_pssm isar.result.q"+to_string(i)+".pssm -out_ascii_pssm isar.result.q"+to_string(i)+".ascii.pssm -save_pssm_after_last_round").data());
	system(("rm isar.q."+to_string(i)).data());
	system(("rm isar.db."+to_string(i)).data());
	system(("rm isar.db."+to_string(i)+".*").data());
}
void runPsiblast(int startQ, int endQ, int additionalQ = 0) {
    for(int i=startQ;i<=endQ;i++)
		psiBlast(i);
		
	if(additionalQ)
		psiBlast(additionalQ);
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
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
	
	query = argv[1];
	
	
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	
	// Starting Thread & move the future object in lambda function by reference
	//thread th(&loadingAnimation, move(futureObj));
	
	//get the list of query headers
	queryCount = 0;
	custombuf   sbuf(isar_pair_queries);
	if(fexists("isar.pair"))
		system("rm isar.pair");
	if (ostream(&sbuf) << ifstream(query).rdbuf() << flush) {
		boost::char_separator<char> sep{">"};
		tokenizer tok{isar_pair_queries, sep};
		for (const auto &q : tok){
			temp = q;
			boost::trim(temp);
			boost::char_separator<char> querySep{"\n"};
			tokenizer tok2{temp, querySep};
			tokenizer::iterator token = tok2.begin();
			temp = *token;
			boost::trim(temp);
			writeToFile("isar.pair",temp+"\tisar.result.q"+to_string(++queryCount)+"{ .pssm | .ascii.pssm}\n",true);
			queryDictionary.insert ( pair<string,int>(temp,queryCount) );
			system(("mkdir "+to_string(queryCount)).data());
		}
	}else {
		cout << "failed to read the query file.\n";
		return 0;
	}
	
		
	for (int i=1;i<=771;i++){
		ifstream in("isar.result.pssm."+to_string(i));
		string line;
		while (getline(in, line)){
			if (line.find("title") != std::string::npos)
				break;
		}
		replace(line, "title \"", "");
		replace(line, "\"", "");
		boost::trim(line);
		string folderNumber = to_string(queryDictionary.find(line)->second);
		system(("mv isar.result.pssm."+to_string(i) +" " +folderNumber+"/.").data());
		system(("mv isar.result.ascii.pssm."+to_string(i) +" " +folderNumber+"/.").data());
		
		cout << folderNumber << "\n";
		
	
	}

	printf("\nParsing Complete!\n");
	
	queryDictionary.clear();
	
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}
