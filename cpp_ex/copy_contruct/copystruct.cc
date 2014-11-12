#include <iostream>
#include <stdio.h>
#include <string.h>

class CB
{
public:
	CB() 
	{
		std::cout << "CB()" << std::endl;
	};
	~CB()
	{
		std::cout << "~CB()" << std::endl;
	}

private:
	int w;
};

class CA
{
public:
	CA(const char *name);
	CA(const CA &);
	~CA();

	CA& operator =(const CA &);
	CB new_CB(const char *);
	CA new_CA(const char *);

private:
	char *m_name;
};

CA::CA(const char *name)
{
	std::cout << "CA(" << name << ")" << std::endl;
}

CA::CA(const CA &c2)
{
	std::cout << "CA(const CA &)" << std::endl;
}

CA& CA::operator =(const CA &c2)
{	
	std::cout << "=(const CA &) " << std::endl;

	return *this;
}

CA::~CA()
{
	std::cout << "~CA()" << std::endl;
}

CB CA::new_CB(const char *name)
{
	int a;
	CB cb;
	printf("a = %p, cb = %p\n", &a, &cb);
	return cb;
}

CA CA::new_CA(const char *name)
{
	int b;
	CA ca(name);
	printf("b = %p, ca = %p\n", &b, &ca);
	return ca;
}

int main(int argc, char *argv[])
{
	int e;
	CA c1("c1");
	printf("e = %p, c1 = %p\n", &e, &c1);
	CB c2 = c1.new_CB("c2");
	printf("e = %p, c2 = %p\n", &e, &c2);
	CA c3 = c1.new_CA("c3");
	int e2;
	printf("e2 = %p, c3 = %p\n", &e2, &c3);

	return 0;
}

