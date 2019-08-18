#ifndef CUSTOMBUF_H  // Header File - for function prototypes and variables
#define CUSTOMBUF_H
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//streambuf improves performance in the case of file reading by reading data from the underlying operating system in bigger chunks.	
class custombuf:
    public streambuf
	{
	public:
		custombuf(string& target);

	private:
		string& target_;
		enum { bufsize = 8192 };
		char buffer_[bufsize];
		int overflow(int c);
		int sync();
	};
    

#endif // CUSTOMBUF_H