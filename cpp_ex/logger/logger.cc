#include <iostream>
#include <stdio.h>
#include <string.h>

class Logger
{
	typedef Logger self;
public:
	Logger();
	~Logger();

	static void operator <<(const char *s);
	static void operator <<(long long);
	static void operator <<(unsigned long long);
	static void operator <<(long);
	static void operator <<(unsigned long);
	static void operator <<(int);
	static void operator <<(unsigned int);
	static void operator <<(short);
	static void operator <<(unsigned short);
	static void operator <<(char);
	static void operator <<(unsigned char);
};

void Logger::operator <<(const char *s)
{
}

void Logger::operator <<(const char *s)
{
}

void Logger::operator <<(long long ll)
{
}

void Logger::operator <<(unsigned long long ull)
{
}

void Logger::operator <<(long l)
{
}

void Logger::operator <<(unsigned long ul)
{
}

void Logger::operator <<(int i)
{
}

void Logger::operator <<(unsigned int ui)
{
}

void Logger::operator <<(short s)
{
}

void Logger::operator <<(unsigned short us)
{
}

void Logger::operator <<(char c)
{
}

void Logger::operator <<(unsigned char uc)
{
}


int main(int argc, char *argv[])
{

	return 0;
}

