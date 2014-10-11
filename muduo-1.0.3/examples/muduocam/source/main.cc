#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/Logging.h>


#include <sys/types.h>
#include <sys/socket.h>  // SO_REUSEPORT

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "source.hpp"

using namespace boost;

using namespace muduo;
using namespace muduo::net;

typedef shared_ptr<SourceClient> SourceClientPtr;
typedef unordered_set<SourceClientPtr> SourceClientList;

int main(int argc, char* argv[])
{
	EventLoop loop;
	SourceClientList scl;
	
	Logger::setLogLevel(Logger::DEBUG);
	
	for (int i = 0; i < 500; i++)
	{
		SourceClientPtr scp(new SourceClient(&loop, InetAddress("127.0.0.1", 8000), i % 10 + 1));
		scp->connect();

		scl.insert(scp);
	}

	loop.loop();
}

