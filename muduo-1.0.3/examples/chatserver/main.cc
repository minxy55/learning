#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/Logging.h>

#include <map>
#include <set>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sys/socket.h>  // SO_REUSEPORT

using namespace muduo;
using namespace muduo::net;

class ChatServer : boost::noncopyable 
{
public:
	ChatServer(EventLoop *loop, const InetAddress &listenAddr);
	void start() { tcpServer_.start(); }

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t);

private:
	typedef std::set<TcpConnectionPtr> ConnectionList;

	ConnectionList connections_;
	TcpServer tcpServer_;
};

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr) :
	tcpServer_(loop, listenAddr, "ChatServer")
{	
	tcpServer_.setConnectionCallback(
		boost::bind(&ChatServer::onConnection, this, _1));
	tcpServer_.setMessageCallback(
		boost::bind(&ChatServer::onMessage, this, _1, _2, _3));
}


void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
			 << conn->peerAddress().toIpPort() << " is "
			 << (conn->connected() ? "UP" : "DOWN");
	if (conn->connected())
	{
		connections_.insert(conn);
	}
	else
	{
		connections_.erase(conn);
	}
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{	
	if (buf->readableBytes() > 0)
	{	
		ConnectionList::iterator it;
		
		StringPiece str(buf->peek(), buf->readableBytes());
		buf->retrieve(buf->readableBytes());

		LOG_INFO << "message: " << str;
		
		for (it = connections_.begin(); it != connections_.end(); it++)
		{
			(*it)->send(str);
		}
	}
}

int main(int argc, char* argv[])
{	
	EventLoop loop;

	ChatServer chatServer(&loop, InetAddress(8000));
	chatServer.start();

	loop.loop();
}

