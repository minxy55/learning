
#include <boost/thread.hpp>  
#include <iostream>  

void task1() {   
	// do stuff  
	std::cout << "This is task1!" << std::endl;  
}  

void task2() {   
	// do stuff  
	std::cout << "This is task2!" << std::endl;  
}  

int main (int argc, char ** argv) {  
	using namespace boost;   
	thread thread_1 = thread(task1);  
	thread thread_2 = thread(task2);  

	// do other stuff  
	thread_2.join();  
	thread_1.join();  
	return 0;  
}  

