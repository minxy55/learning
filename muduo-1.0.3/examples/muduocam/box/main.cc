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

#include "box.hpp"

using namespace boost;

using namespace muduo;
using namespace muduo::net;

typedef shared_ptr<BoxClient> BoxClientPtr;
typedef unordered_set<BoxClientPtr> BoxClientList;

int main(int argc, char* argv[])
{
	BoxClientList bcl;
	EventLoop loop;
	
	Logger::setLogLevel(Logger::DEBUG);

	for (int i = 0; i < 1; i++)
	{
		BoxClientPtr cp(new BoxClient(&loop, InetAddress("127.0.0.1", 8001)));
		cp->connect();

		bcl.insert(cp);
	}

	loop.loop();

	return 0;
}

