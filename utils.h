#ifndef UTILS_H  // Header File - for function prototypes and variables
#define UTILS_H
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <future>

using namespace std;

void writeToFile(const string& name, const string& content, bool append);

//animation on console while parsing
void loadingAnimation(future<void> futureObj);

bool is_file_empty(ifstream& pFile);

bool fexists(const char *filename);

void psiBlast(int i, string dbsize, string use_sw_tback, string output_profile, string output_pssm, string output_ascii_pssm);

void runPsiblast(int startQ, int endQ, int additionalQ, string dbsize, string use_sw_tback, string output_profile, string output_pssm, string output_ascii_pssm);

void help(bool detailed);
    

#endif // UTILS_H