#ifndef CUSTOMBUF_H  // Header File - for function prototypes and variables
#define CUSTOMBUF_H
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

// streambuf is a base class in C++ that provides an interface for reading from
// or writing to a sequence of characters. The class improves performance in 
// the case of file reading by reading data from the underlying operating system
// in larger chunks.	
class custombuf : public streambuf {
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