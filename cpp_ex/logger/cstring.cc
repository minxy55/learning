#include <iostream>
#include <stdio.h>
#include <string.h>

class CString
{
public:
	CString();
	CString(const char *s);
	CString(const CString &s2);
	~CString();

	CString& operator =(const CString &s2);
	CString& operator +=(const CString &s2);
	char *toString();

private:
	char *m_string;
};

CString::CString()
{
	m_string = NULL;
}

CString::CString(const char *s)
{
	m_string = new char[strlen(s)];
	strcpy(m_string, s);
}

CString::CString(const CString &s2)
{
	if (m_string)
		delete m_string;

	m_string = new char[::strlen(s2.m_string)];
	strcpy(m_string, s2.m_string);
}

CString::~CString()
{
	if (m_string)
		delete m_string;
}

CString& CString::operator =(const CString &s2)
{
	if (m_string)
		delete m_string;

	m_string = new char[strlen(s2.m_string)];
	strcpy(m_string, s2.m_string);
}

CString& CString::operator +=(const CString &s2)
{
	char *s = new char[strlen(m_string) + strlen(s2.m_string)];

	strcpy(s, m_string);
	strcat(s, s2.m_string);

	delete m_string;

	m_string = s;
}

char *CString::toString()
{
	return m_string;
}

int main(int argc, char *argv[])
{
	CString s("good morning!");

	std::cout << "" << s.toString() << std::endl;

	return 0;
}

