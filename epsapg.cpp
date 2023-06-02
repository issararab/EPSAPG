#include "custombuf.h"
#include "utils.h"
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>         // cast uuid to string
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
//using namespace boost;

int main(int argc, char **argv) {
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    string isar_tuple,isar_pair,isar_pair_queries,single_query_data;
	string query,temp; 
	int queryCount;
	map<string,int> queryDictionary;
	//paramters of the software command 
	map<string, string> commandParameters;
	string parameters[] = {
		"-query",
		"-db_load_mode",
		"-dbsize",
		"-interm_path",
		"-use_sw_tback",
		"-max_seqs",
		"-threads",
		"-output_profile",
		"-output_pssm",
		"-output_ascii_pssm"
	};
	// Create a std::promise object
	promise<void> exitSignal;
	//Fetch std::future object associated with promise
	future<void> futureObj = exitSignal.get_future();
	//multiple threads datastructure
	vector<thread> threads;
	unsigned concurentThreadsSupported = 0;
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	string UUID = "-"+boost::lexical_cast<std::string>(uuid);
	
	//parsing the command
	if (argc <= 2){
	   if(argc == 1)
		   help(true);
	   else if(string(argv[1]) == "-h")
			help(false);
	   else if(string(argv[1]) == "-help")
			help(true);
	   else {
			printf("You have to provide a command in the following format:\n");
			help(true);
	   }
	   return 0;
	} else {
		if(argc%2 == 0) {
			printf("You have to provide a command in the following format:\n");
			help(false);
			return 0;
		}
		for(int i=1;i<argc;i+=2) {
			string *parm = std::find(std::begin(parameters), std::end(parameters), argv[i]);
			if (parm != std::end(parameters))
				commandParameters.insert( pair<string,string>(argv[i],argv[i+1]) );
			else {
				printf("\n Unknown parameter: %s \nYou have to provide a command in the following format:\n",argv[i]);
				help(false);
				return 0;
			}
		}
	}
	
	//clean folder from previous results
	system("rm -f isar.result.*");
	//check if command include input file
	if ( commandParameters.find("-query") == commandParameters.end() ) {
		printf("\n Please provide an input fasta file. '-query' parameter is mandatory to run the software.  \n"
				"You have to provide a command in the following format:\n");
		help(false);
		return 0;
	}
	query = commandParameters.find("-query")->second;
	if (query == "queryDB" || boost::starts_with(query, "isar"))
	{
	   printf("'queryDB' and the words starting with 'isar' are preserved file names.\n"
				"Please consider changing your file name and run the search again.\nGood luck!");
	   return 0;
	}
	printf("###############################\n");
	printf("#      Starting MMSEQS2        #\n");
	printf("###############################\n");
	
	//Check if mmseqs2 is installed
	if (system("which mmseqs > /dev/null 2>&1")) {
		cout << "mmseqs: command not found.\n\tPlease make sure you have installed MMseqs2 on your"
				" machine and included its path to the envirnment variables.\n";
		return 0;
	}
	
	//Check if psiblast is installed
	if (system("which psiblast > /dev/null 2>&1")) {
		cout << "psiblast: command not found.\n\tPlease make sure you have installed psiblast on"
				" your machine and included its path to the envirnment variables.\n";
		return 0;
	}
	
	//Constructing the command according to the parameters provided
	string max_seqs,interm_path;
	if ( commandParameters.find("-max_seqs") == commandParameters.end() )
		max_seqs = "1000";
	else
		max_seqs = commandParameters.find("-max_seqs")->second;
	if ( commandParameters.find("-interm_path") == commandParameters.end() )
		interm_path = "tmp";
	else
		interm_path = commandParameters.find("-interm_path")->second;
	
	//start mmseqs search
	system(("mmseqs createdb "+query+" queryDB"+UUID).data());
	if ( commandParameters.find("-db_load_mode") != commandParameters.end() && commandParameters.find("-db_load_mode")->second == "2") {
		if ( commandParameters.find("-threads") != commandParameters.end() ) {
			system(("mmseqs search --threads " + commandParameters.find("-threads")->second + " --num-iterations 1 --max-seqs " +
					max_seqs + " queryDB" + UUID + " targetDB resultDB" + UUID + " " + interm_path + " --db-load-mode 2").data());
			system(("mmseqs convertalis --threads " + commandParameters.find("-threads")->second + " queryDB" + UUID + " targetDB resultDB" +
					UUID + " isar.tuple --format-output \"qheader,theader,tseq\" --db-load-mode 2").data()); 
		} else {
			system(("mmseqs search --num-iterations 1 --max-seqs " + max_seqs + " queryDB" + UUID + " targetDB resultDB" + UUID + " " +
					interm_path + " --db-load-mode 2").data());
			system(("mmseqs convertalis queryDB" + UUID + " targetDB resultDB" + UUID + 
					" isar.tuple --format-output \"qheader,theader,tseq\" --db-load-mode 2").data()); 
		}
	} else {
		if ( commandParameters.find("-threads") != commandParameters.end() ) {
			system(("mmseqs search --threads " + commandParameters.find("-threads")->second + " --num-iterations 1 --max-seqs " + max_seqs + 
					" queryDB" + UUID + " targetDB resultDB" + UUID + " " + interm_path).data());
			system(("mmseqs convertalis --threads " + commandParameters.find("-threads")->second + " queryDB" + UUID + " targetDB resultDB" + 
					UUID + " isar.tuple --format-output \"qheader,theader,tseq\"").data()); 
		} else {
			system(("mmseqs search --num-iterations 1 --max-seqs " + max_seqs + " queryDB" + UUID + " targetDB resultDB" + 
					UUID + " " + interm_path).data());
			system(("mmseqs convertalis queryDB" + UUID + " targetDB resultDB" + UUID + 
					" isar.tuple --format-output \"qheader,theader,tseq\"").data()); 
		}
	}
	
	//delete intermediate files
	system(("rm -f queryDB" + UUID).data());
	system(("rm -f queryDB" + UUID+".*").data());
	system(("rm -f queryDB" + UUID+"_h").data());
	system(("rm -f queryDB" + UUID+"_h.*").data());
	system(("rm -f resultDB" + UUID+"").data());
	system(("rm -f resultDB" + UUID+".*").data());
	printf("###############################\n");
	printf("#   Parsing MMSEQS2's output  #\n");
	printf("###############################\n");
	
	// Starting Thread & move the future object in lambda function by reference
	thread th(&loadingAnimation, move(futureObj));
	
	//get the list of query headers and creating a file, isar.pair, referencing all the corresponding outputs
	queryCount = 0;
	custombuf   sbuf(isar_pair_queries);
	//if(fexists("isar.pair"))
	system("rm -f isar.pair");

	if (ostream(&sbuf) << ifstream(query).rdbuf() << flush) {
		boost::char_separator<char> sep{">"};
		tokenizer tok{isar_pair_queries, sep};
		for (const auto &q : tok) {
			temp = q;
			boost::trim(temp);
			writeToFile("isar.q." + to_string(++queryCount),">"+temp,false);
			writeToFile("isar.db." + to_string(queryCount),"",false);
			boost::char_separator<char> querySep{"\n"};
			tokenizer tok2{temp, querySep};
			tokenizer::iterator token = tok2.begin();
			temp = *token;
			boost::trim(temp);
			writeToFile("isar.pair", temp + "\tisar.result.q" + to_string(queryCount) + 
						"{.psiblast | .pssm | .ascii.pssm}\n",true);
			queryDictionary.insert ( pair<string,int>(temp,queryCount) );
		}
	}else {
		cout << "failed to read the query file.\n";
		return 0;
	}
	//Parsing isar.tuple - The output file containing all the alignments from MMseqs2
	string dbOutput = "";
	custombuf   sbuf2(isar_tuple);
	if (ostream(&sbuf2) << ifstream("isar.tuple").rdbuf() << flush) {
		string previousQueryHeader = "";
		string targetFile = "";
		boost::char_separator<char> sep1{"\n"};
		tokenizer tok1{isar_tuple, sep1};
		for (const auto &row : tok1) {
			boost::char_separator<char> sep2{"\t"};
			tokenizer tok2{row, sep2};
			tokenizer::iterator col = tok2.begin();
			temp = *col;//get the query header
			boost::trim(temp);//trim the query id
			//block to avoid searching in the dictionary for the query id for each line/sequece retrieved
			if(previousQueryHeader != temp) {
				if(previousQueryHeader != "") {
					writeToFile(targetFile,dbOutput,false);
					dbOutput = "";
				}
				//get the query id and assign it to the target result file 
				targetFile = "isar.db."+to_string(queryDictionary.find(temp)->second); 
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
		//write to file the db corresponding to the last query in the batch
		writeToFile(targetFile,dbOutput,false);
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
    system("rm -f isar.tuple");
	printf("###############################\n");
	printf("#      Starting PsiBlast      #\n");
	printf("###############################\n");
	
	sleep(2);
	system("rm -f isar.result.q*");
	
	if ( commandParameters.find("-threads") != commandParameters.end() )
		concurentThreadsSupported = std::stoul (commandParameters.find("-threads")->second,nullptr,0);
	else
		concurentThreadsSupported = thread::hardware_concurrency();
	//extract parameter values for psiblast runs
	string dbsize, use_sw_tback, output_profile, output_pssm, output_ascii_pssm;
	if ( commandParameters.find("-dbsize") != commandParameters.end() )
		dbsize = commandParameters.find("-dbsize")->second;
	if ( commandParameters.find("-use_sw_tback") != commandParameters.end() )
		use_sw_tback = commandParameters.find("-use_sw_tback")->second;
	if ( commandParameters.find("-output_profile") != commandParameters.end() )
		output_profile = commandParameters.find("-output_profile")->second;
	if ( commandParameters.find("-output_pssm") != commandParameters.end() )
		output_pssm = commandParameters.find("-output_pssm")->second;
	if ( commandParameters.find("-output_ascii_pssm") != commandParameters.end() )
		output_ascii_pssm = commandParameters.find("-output_ascii_pssm")->second;
	
	if(concurentThreadsSupported) {
		//Launch a group of threads
		int step = queryCount/concurentThreadsSupported;
		//Stratified distribution of thread jobs
		for (int i = 0; i < concurentThreadsSupported; ++i) {
			if(i < queryCount%concurentThreadsSupported)
				threads.push_back(thread(runPsiblast, 1 + step * i,step * ( i + 1), queryCount-i, dbsize, 
										use_sw_tback, output_profile, output_pssm, output_ascii_pssm));
			else              
				threads.push_back(thread(runPsiblast, 1 + step * i,step * ( i + 1), 0, dbsize, 
										use_sw_tback, output_profile, output_pssm, output_ascii_pssm));
		}
	} else
		threads.push_back(thread(runPsiblast, 1, queryCount, 0, dbsize, use_sw_tback, 
								output_profile, output_pssm, output_ascii_pssm));
	

    //Join the threads with the main thread
    for(auto &th : threads) {
        th.join();
    }
	printf("###############################\n");
	printf("#        End of Search        #\n");
	printf("###############################\n");
	return 0;
}