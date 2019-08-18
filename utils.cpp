#include "utils.h"
#include "custombuf.h" 
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <future>
#include <unistd.h>

using namespace std;

void writeToFile(const string& name, const string& content, bool append) {
    ofstream outfile;
    if (append)
        outfile.open(name, ios_base::app);
    else
        outfile.open(name);
    outfile << content;
}

//animation on console while parsing
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

void psiBlast(int i,string dbsize, string use_sw_tback, string output_profile, string output_pssm, string output_ascii_pssm) {
	string single_query_data;
	//Check if the query db is empty
	ifstream file("isar.db."+to_string(i));
	if (is_file_empty(file))
	{
		custombuf sbuf3(single_query_data);
		if (ostream(&sbuf3) << ifstream("isar.q."+to_string(i)).rdbuf() << flush) {
			writeToFile("isar.db."+to_string(i),single_query_data,false);
		}else {
			cout << "failed to read the query file number; "+to_string(i) +".\n";
		}
	}
	system(("makeblastdb -in isar.db."+to_string(i)+" -dbtype prot").data());
	
	
	string cmd = "psiblast -query isar.q."+to_string(i)+" -db isar.db."+to_string(i)+" -evalue 10";
	if(!dbsize.empty())
		cmd += " -dbsize "+dbsize;
	
	if(use_sw_tback == "1")
		cmd += " -use_sw_tback";	
	
	if(output_profile.empty() || output_profile == "1")
		cmd += " -out isar.result.q"+to_string(i)+".psiblast";

	if(output_pssm == "1")
		cmd += " -out_pssm isar.result.q"+to_string(i)+".pssm";
						
	if(output_ascii_pssm == "1")
		cmd += " -out_ascii_pssm isar.result.q"+to_string(i)+".ascii.pssm";		
						
	if(output_pssm == "1" || output_ascii_pssm == "1")
		cmd += " -save_pssm_after_last_round";		
	
	system((cmd).data());
			
	
	system(("rm -f isar.q."+to_string(i)).data());
	system(("rm -f isar.db."+to_string(i)).data());
	system(("rm -f isar.db."+to_string(i)+".*").data());
}
void runPsiblast(int startQ, int endQ, int additionalQ, string dbsize, string use_sw_tback, string output_profile, string output_pssm, string output_ascii_pssm) {
    for(int i=startQ;i<=endQ;i++)
		psiBlast(i,dbsize,use_sw_tback,output_profile,output_pssm,output_ascii_pssm);
		
	if(additionalQ)
		psiBlast(additionalQ,dbsize, use_sw_tback,output_profile, output_pssm, output_ascii_pssm);
}

void help(bool detailed){
	cout << " ++USAGE \n"
	     << "  isarpipeline [-h] [-help] [-query input_file] \n"
	     << "      [-db_load_mode int_value[1|2] ] [-dbsize num_amino_acids] \n"
		 << "      [-interm_path path_to_intermedite_folder] [-use_sw_tback boolean[0|1] ] \n"
		 << "      [-max_seqs int_value] [-threads num_threads] [-output_profile boolean [0|1] ] \n"
		 << "      [-output_pssm boolean [0|1] ] [-output_ascii_pssm boolean[0|1] ] \n"
		 << "\n ++DESCRIPTION \n"
		 << "      IsarPipelline: Super fast alignment tool that runs the muscles of MMseqs2 and generates PSI-BLAST output (profile and pssm)\n"
		 << endl;
	if(!detailed)
		return;
	cout << "\n ++MANDATORY PARAMETERS \n"
		 <<	" -query <Fasta_File_In> \n"
		 <<	"     Input file name in fasta format \n"
		 <<	"\n ++OPTIONAL PARAMETERS "
		 <<	"\n -interm_path <folder_path>"
		 <<	"\n   If run with indextable, which is the recommanded option, "
		 <<	"\n   MMseqs2 stores intermediate results in tmp folder. Using a fast" 
		 <<	"\n   local drive can reduce load on a shared filesystem and increase" 
		 <<	"\n   speed.(If not specified, the default path is 'tmp' folder in the" 
		 <<	"\n   same location as targetDB. Same one used while indexing the DB.)\n"
		 <<	"\n -db_load_mode <int_value[1|2]>"
		 <<	"\n   Option for using instant search power of MMseqs2, set to 2." 
		 <<	"\n   (default 1, loads db from disk). This option enhaces the search unless" 
		 <<	"\n   the indextable is loaded and locked into memory. Refer to: How to search" 
		 <<	"\n   small query sets fast in the MMseqs2 documentation.\n"
		 <<	"\n -dbsize <size>"
		 <<	"\n   Effective length of the database. Default uses the amino acid length of" 
		 <<	"\n   MMseqs2 result. For accurate statistics of the profile, provide the main database length.\n"
		 <<	"\n -use_sw_tback <boolean[0|1]>"
		 <<	"\n   Compute locally optimal Smith-Waterman alignments(default is set to 0) \n"
		 <<	"\n -max_seqs <int_value>"
		 <<	"\n   maximum result sequences per query (this parameter affects the sensitivity). Default is set to 1000.\n"
		 <<	"\n -threads <int_value>"
		 <<	"\n   number of cores used for the computation (uses all cores of the machine by default)"
		 <<	"\n -output_profile <boolean[0|1]>"
		 <<	"\n   Enable or disable outputing alignment profile of a query sequence against a database (enabled by default)\n"
		 <<	"\n -output_pssm <boolean[0|1]>"
		 <<	"\n    Enable or disable outputing position-specific scoring matrix checkpoint file (disabled by default)\n"
		 <<	"\n -output_ascii_pssm <boolean[0|1]>"
		 <<	"\n    Enable or disable outputing ASCII version of position-specific scoring matrix checkpoint file (disabled by default)"
		 <<	"\n"
		 <<	"\n"
		 << endl;
}