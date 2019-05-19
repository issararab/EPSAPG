#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <map>
#include <unistd.h>
#include <thread>
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

void loadingAnimation(){
	std::cout << '-' << std::flush;
    for (;;) {
        sleep(1);
        std::cout << "\b\\" << std::flush;
        sleep(1);
        std::cout << "\b|" << std::flush;
        sleep(1);
        std::cout << "\b/" << std::flush;
        sleep(1);
        std::cout << "\b-" << std::flush;
    }
}


int main(int argc, char **argv)
{
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    string isar_tuple,isar_pair,isar_pair_queries;
	string query,temp; 
	int queryCount;
	map<string,int> queryDictionary;
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
	system(("mmseqs createdb "+query+" queryDB").data());//cow.protein.faa
	system("mmseqs search queryDB targetDB resultDB tmp");//  /home/issar.arab/uniref90TMP/tmp
	//convert the output to query and sequence result
	system("mmseqs convertalis queryDB targetDB resultDB isar.tuple --format-output \"qheader,theader,tseq\"");
	//delete intermediate files
	system("rm queryDB");
	system("rm queryDB.*");
	system("rm queryDB_h");
	system("rm queryDB_h.*");
	system("rm resultDB");
	system("rm resultDB.*");
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	//std::thread t1(loadingAnimation);
	//get the list of query headers
	queryCount = 0;
	custombuf   sbuf(isar_pair_queries);
	//writeToFile("isar.pair","");//To reconsider
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
	}
	//free(isar_pair_queries);
	custombuf   sbuf2(isar_tuple);
	if (ostream(&sbuf2) << ifstream("isar.tuple").rdbuf() << flush) {
		boost::char_separator<char> sep1{"\n"};
		tokenizer tok1{isar_tuple, sep1};
		for (const auto &row : tok1){
			boost::char_separator<char> sep2{"\t"};
			tokenizer tok2{row, sep2};
			tokenizer::iterator col = tok2.begin();
			temp = *col;//get the query header
			boost::trim(temp);//trim the query id
			string targetFile = "isar.db."+to_string(queryDictionary.find(temp)->second); //get the query id and assign it to the target result file 
			temp = *(++col);//get the retrieved sequece header
			boost::trim(temp);
			writeToFile(targetFile,">"+temp+"\n",true);
			temp = *(++col);//get the retrieved sequece
			boost::trim(temp);
			writeToFile(targetFile,temp+"\n",true);
			
		}
	}else {
		cout << "failed to read the isar.tuple file, search result file from MMseqs2.\n";
	}

	printf("Done!\n");
	queryDictionary.clear();
	system("rm isar.tuple");
	printf("###############################\n");
	printf("#      Starting PsiBlast      #\n");
	printf("###############################\n");
	sleep(3);
	system("rm isar.result.q*");
	for(int i=1;i<=queryCount;i++){
		system(("makeblastdb -in isar.db."+to_string(i)+" -dbtype prot").data());
		system(("psiblast -query isar.q."+to_string(i)+" -db isar.db."+to_string(i)+" -num_threads 12 -evalue 10 -out isar.result.q"+to_string(i)+".psiblast -out_pssm isar.result.q"+to_string(i)+".pssm -out_ascii_pssm isar.result.q"+to_string(i)+".ascii.pssm -save_pssm_after_last_round -save_each_pssm").data());
		system(("rm isar.q."+to_string(i)).data());
		system(("rm isar.db."+to_string(i)).data());
		system(("rm isar.db."+to_string(i)+".*").data());
	}
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}
