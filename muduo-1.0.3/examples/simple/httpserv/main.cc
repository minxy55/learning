#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <memory>

using namespace muduo;
using namespace muduo::net;

/*
void httpCallback(const HttpRequest &r, HttpResponse *rr)
{
	LOG_INFO << "HttpRequest: ";
	LOG_INFO << r.method();
	LOG_INFO << r.path();
	LOG_INFO << r.receiveTime().toFormattedString();

	rr->setStatusCode(HttpResponse::k200Ok);
	rr->setStatusMessage("That's working well.");
	rr->setContentType("text/html");
	rr->setBody("<html><head><h1>Sample :)</h1></head><body></body></html>");
}


int main(int argc, char *argv[])
{
	LOG_INFO << "pid = " << getpid();

	if (argc < 3) {
		printf("Usage: %s <Port> <ThreadNum>\n", argv[0]);
		return -1;
	}
	
	unsigned short port = static_cast<unsigned short>(atoi(argv[1]));
	int threadNum = atoi(argv[2]);

	
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(port);
	HttpServer server(&loop, listenAddr, "dummy");

	server.setHttpCallback(httpCallback);
	server.setThreadNum(threadNum);

	server.start();
	loop.loop();
}
*/

class Song 
{
public:
	Song(std::string name)
	{
		_name = name;
		LOG_INFO << "in constrcutor of Song" << _name;
	}
	~Song()
	{
		LOG_INFO << "in desconstrcutor of Song" << _name;
	}

private:
	std::string _name;
};

int main(int argc, char *argv[])
{
	boost::shared_ptr<Song> sp2;
		
	for (int j = 0; j < 10; j++)
	{
		LOG_INFO << j;
		boost::shared_ptr<Song> sp1 = boost::shared_ptr<Song>(new Song(("lamada " + j)));

		if (j == 5)
		{
			sp2 = sp1;
		}
	}
	
	return 0;
}

