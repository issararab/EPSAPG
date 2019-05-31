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
	printf("#      Starting MMSEQS2        #\n");
	printf("###############################\n");
	//Check if mmseqs2 is installed
	if (system("which mmseqs > /dev/null 2>&1")) {
		cout << "mmseqs: command not found.\n\tPlease make sure you have installed MMseqs2 on your machine and included its path to the envirnment variables.\n";
		return 0;
	} 
	system(("mmseqs createdb "+query+" queryDB"+UUID).data());//cow.protein.faa
	system(("mmseqs search --num-iterations 1 --max-seqs 1000 queryDB"+UUID+" targetDB resultDB"+UUID+" tmp").data());//  /home/issar.arab/uniref90TMP/tmp
	//convert the output to query and retrieved sequence results
	system(("mmseqs convertalis queryDB"+UUID+" targetDB resultDB"+UUID+" isar.tuple --format-output \"qheader,theader,tseq\"").data());
	//delete intermediate files
	system(("rm queryDB"+UUID).data());
	system(("rm queryDB"+UUID+".*").data());
	system(("rm queryDB"+UUID+"_h").data());
	system(("rm queryDB"+UUID+"_h.*").data());
	system(("rm resultDB"+UUID+"").data());
	system(("rm resultDB"+UUID+".*").data());
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	
	// Starting Thread & move the future object in lambda function by reference
	thread th(&loadingAnimation, move(futureObj));
	
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
			writeToFile("isar.q."+to_string(++queryCount),">"+temp);
			writeToFile("isar.db."+to_string(queryCount),"");
			boost::char_separator<char> querySep{"\n"};
			tokenizer tok2{temp, querySep};
			tokenizer::iterator token = tok2.begin();
			temp = *token;
			boost::trim(temp);
			writeToFile("isar.pair",temp+"\tisar.result.q"+to_string(queryCount)+"{.psiblast | .pssm | .ascii.pssm}\n",true);
			queryDictionary.insert ( pair<string,int>(temp,queryCount) );
		}
	}else {
		cout << "failed to read the query file.\n";
		return 0;
	}
	
	string dbOutput = "";
	custombuf   sbuf2(isar_tuple);
	if (ostream(&sbuf2) << ifstream("isar.tuple").rdbuf() << flush) {
		string previousQueryHeader = "";
		string targetFile = "";
		boost::char_separator<char> sep1{"\n"};
		tokenizer tok1{isar_tuple, sep1};
		for (const auto &row : tok1){
			boost::char_separator<char> sep2{"\t"};
			tokenizer tok2{row, sep2};
			tokenizer::iterator col = tok2.begin();
			temp = *col;//get the query header
			boost::trim(temp);//trim the query id
			if(previousQueryHeader != temp){//block to avoid searching in the dictionary for the query id for each line/sequece retrieved
				if(previousQueryHeader != ""){
					writeToFile(targetFile,dbOutput);
					dbOutput = "";
				}
				targetFile = "isar.db."+to_string(queryDictionary.find(temp)->second); //get the query id and assign it to the target result file 
				previousQueryHeader = temp;
			}
			temp = *(++col);//get the retrieved sequece header
			boost::trim(temp);
			//writeToFile(targetFile,">"+temp+"\n",true);
			dbOutput += ">"+temp+"\n";
			temp = *(++col);//get the retrieved sequece
			boost::trim(temp);
			//writeToFile(targetFile,temp+"\n",true);
			dbOutput += temp+"\n";
		}
		//Set the value in promise
		exitSignal.set_value();
	 
		//Wait for thread to join
		th.join();
		printf("\nParsing Complete!\n");
		sleep(1);
	}else {
		cout << "failed to read the isar.tuple file, search result file from MMseqs2.\n";
		return 0;
	}

	
	queryDictionary.clear();
	system("rm isar.tuple");
	printf("###############################\n");
	printf("#      Starting PsiBlast      #\n");
	printf("###############################\n");
	//Check if mmseqs2 is installed
	if (system("which psiblast > /dev/null 2>&1")) {
		cout << "psiblast: command not found.\n\tPlease make sure you have installed psiblast on your machine and included its path to the envirnment variables.\n";
		return 0;
	}
	sleep(2);
	system("rm isar.result.q*");
	
	concurentThreadsSupported = thread::hardware_concurrency();
	if(concurentThreadsSupported){
		//Launch a group of threads
		int step = queryCount/concurentThreadsSupported;
		//Stratified distribution of thread jobs
		for (int i = 0; i < concurentThreadsSupported; ++i) {
			if(i < queryCount%concurentThreadsSupported)
				threads.push_back(thread(runPsiblast,1+step*i,step*(i+1),queryCount-i));
			else
				threads.push_back(thread(runPsiblast,1+step*i,step*(i+1),0));
		}
	}else
		threads.push_back(thread(runPsiblast,1,queryCount,0));
	

    //Join the threads with the main thread
    for(auto &th : threads){
        th.join();
    }
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}
