

#include <iostream>

using namespace std;

struct st {
	int &ra;
	int &rb;
	int &rc;
};

int main(int argc, char *argv[])
{
	int a = 3;
	int &r = a;

	cout << "a:r -> " << a << ":" << r << endl;
	cout << "sizeof(r) -> " << sizeof(r) << endl;
	cout << "sizeof(st) -> " << sizeof(struct st) << endl;

	return 0;
}

