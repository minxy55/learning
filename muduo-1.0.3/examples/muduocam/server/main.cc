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

#include "worker.hpp"
#include "forward.hpp"

using namespace boost;

using namespace muduo;
using namespace muduo::net;

static const int numThreads = 5;

class TcpServer0 : boost::noncopyable 
{
public:
	TcpServer0(EventLoop *loop, const InetAddress &listenAddr);
	void start() { tcpServer_.start(); }

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t);
	void onThreadInit(EventLoop *loop);

	void onTimeout();

private:
	typedef shared_ptr<ForwardClient> ForwardPtr;
	typedef unordered_set<ForwardPtr> ForwardSet;
	typedef unordered_set<TcpConnectionPtr> FeederSet;

	ForwardSet forwardSet;
	FeederSet feederSet;

	TcpServer tcpServer_;
	EventLoop *loop_;
};

TcpServer0::TcpServer0(EventLoop *loop, const InetAddress &listenAddr) :
	tcpServer_(loop, listenAddr, "observe")
{	
	tcpServer_.setConnectionCallback(
		boost::bind(&TcpServer0::onConnection, this, _1));
	tcpServer_.setMessageCallback(
		boost::bind(&TcpServer0::onMessage, this, _1, _2, _3));
	tcpServer_.setThreadInitCallback(
		boost::bind(&TcpServer0::onThreadInit, this, _1));

	loop_ = loop;
}

void TcpServer0::onConnection(const TcpConnectionPtr &conn)
{
	LOG_DEBUG << " " << (conn->connected() ? "UP" : "DOWN");
	
	if (conn->connected())
	{
		feederSet.insert(conn);
	}
	else
	{
		feederSet.erase(conn);
	}
}

void TcpServer0::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{
	const void *data;
	unsigned char sync, tag;
	Buffer ts_buf;
	
	LOG_DEBUG << " readableBytes->" << buf->readableBytes();

	while (buf->readableBytes() >= 25)
	{
		data = buf->peek();
		sync = *static_cast<const unsigned char*>(data);
		tag = *(static_cast<const unsigned char*>(data) + 1);

		if (sync == 0xf7 && tag == 0x0a)
		{
			LOG_DEBUG << "forward ...";
			
			ts_buf.append(data, 25);
			buf->retrieve(25);			
		}
		else
		{
			buf->retrieve(1);
		}
	}

	for (ForwardSet::iterator it = forwardSet.begin(); it != forwardSet.end(); it++)
	{
		ForwardPtr f = *it;
		f->write(&ts_buf);
	}
}

void TcpServer0::onThreadInit(EventLoop *loop)
{
	for (int i = 0; i < numThreads; i++)
	{
		string name = " forwarder->" + i;
		ForwardPtr f(new ForwardClient(loop, InetAddress("127.0.0.1", 8001)));

		f->connect();

		forwardSet.insert(f);
	}

	loop->runEvery(30, boost::bind(&TcpServer0::onTimeout, this));
}

void TcpServer0::onTimeout()
{
	// no feed in 30s, so that disconnect that source...
}

int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::INFO);
	
	EventLoop loop;

	TcpServer1 tcpServer1(&loop, InetAddress(8001), numThreads);
	tcpServer1.start();

	CurrentThread::sleepUsec(1 * 1000 * 1000);

	TcpServer0 tcpServer0(&loop, InetAddress(8000));
	tcpServer0.start();

	loop.loop();

	return 0;
}

