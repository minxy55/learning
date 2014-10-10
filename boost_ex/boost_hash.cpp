
#include <boost/unordered_map.hpp>  
#include <boost/shared_ptr.hpp>  
#include <iostream> 
#include <map>
#include <sys/time.h>

using namespace std;
using namespace boost;

int main (int argc, char ** argv)
{
	struct timeval tv;
	{
		cout << "bucket: " << unordered_detail::default_bucket_count << endl;
		unordered_map<int, int> hash1;

		gettimeofday(&tv, NULL);
		cout << "1: " << tv.tv_sec << "-" << tv.tv_usec << endl;

		for (int i = 0; i < 10000000; i++)
		{
			hash1.insert(pair<int, int>(i, i));
		}

		gettimeofday(&tv, NULL);
		cout << "2: " << tv.tv_sec << "-" << tv.tv_usec << endl;

		for (int j = 0; j < 10000000; j++)
		{
			unordered_map<int, int>::iterator it = hash1.find(j);
		}
	}

	{
		gettimeofday(&tv, NULL);
		cout << "3: " << tv.tv_sec << "-" << tv.tv_usec << endl;

		map<int, int> hash2;

		for (int i = 0; i < 10000000; i++)
		{
			hash2.insert(pair<int, int>(i, i));
		}

		gettimeofday(&tv, NULL);
		cout << "4: " << tv.tv_sec << "-" << tv.tv_usec << endl;

		for (int j = 0; j < 10000000; j++)
		{
			map<int, int>::iterator it = hash2.find(j);
		}

		gettimeofday(&tv, NULL);
		cout << "5: " << tv.tv_sec << "-" << tv.tv_usec << endl;
	}

	return 0;  
}  

