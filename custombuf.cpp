#include "custombuf.h" 
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


custombuf::custombuf(string& target): target_(target) {
	this->setp(this->buffer_, this->buffer_ + bufsize - 1);
}

int custombuf::overflow(int c) {
	if (!traits_type::eq_int_type(c, traits_type::eof()))
	{
		*this->pptr() = traits_type::to_char_type(c);
		this->pbump(1);
	}
	this->target_.append(this->pbase(), this->pptr() - this->pbase());
	this->setp(this->buffer_, this->buffer_ + bufsize - 1);
	return traits_type::not_eof(c);
}
int custombuf::sync() { this->overflow(traits_type::eof()); return 0; }
