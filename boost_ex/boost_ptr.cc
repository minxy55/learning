
#include <boost/unordered_map.hpp>  
#include <boost/shared_ptr.hpp>  
#include <boost/weak_ptr.hpp>  
#include <boost/scoped_ptr.hpp>  
#include <iostream> 
#include <map>

using namespace std;
using namespace boost;

class A 
{
public:
	A(int v) { x = v; }
	int x;
};

int main (int argc, char ** argv)
{  
	scoped_ptr<A> ip(new A(3));
//	scoped_ptr<int> ip2 = ip;

	cout << ip->us_count() << endl;

	shared_ptr<A> wp = shared_ptr<A>(new A(4));
	weak_ptr<A> wp2 = weak_ptr<A>(wp);




	return 0;  
}  

