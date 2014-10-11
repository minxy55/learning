#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>

#include <map>
#include <set>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sys/socket.h>  // SO_REUSEPORT

using namespace muduo;
using namespace muduo::net;

class ChatClient : boost::noncopyable 
{
public:
	ChatClient(EventLoop *loop, const InetAddress &listenAddr);
	void connect();
	void disconnect();
	void write(const StringPiece &message);

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t);

private:
	TcpClient tcpClient_;
	TcpConnectionPtr tcpConn_;
	MutexLock mutex_;
};

ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr) :
	tcpClient_(loop, serverAddr, "ChatClient")
{	
	tcpClient_.setConnectionCallback(
		boost::bind(&ChatClient::onConnection, this, _1));
	tcpClient_.setMessageCallback(
		boost::bind(&ChatClient::onMessage, this, _1, _2, _3));
	tcpClient_.enableRetry();
}

void ChatClient::connect()
{
	tcpClient_.connect();
}

void ChatClient::disconnect()
{
	tcpClient_.disconnect();
}

void ChatClient::write(const StringPiece &message)
{
	MutexLockGuard lock(mutex_);
	if (tcpConn_)
	{
		//LOG_INFO << "write: " << message;
		
		Buffer buf;

		buf.append(message.data(), message.size());
		
		tcpConn_->send(&buf);
	}
}

void ChatClient::onConnection(const TcpConnectionPtr &conn)
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
			 << conn->peerAddress().toIpPort() << " is "
			 << (conn->connected() ? "UP" : "DOWN");

	MutexLockGuard lock(mutex_);
	if (conn->connected())
	{
		tcpConn_ = conn;
	}
	else
	{
		tcpConn_.reset();
	}
}

void ChatClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{
	if (buf->readableBytes() > 0)
	{
		std::string message(buf->peek(), buf->readableBytes());
		buf->retrieve(buf->readableBytes());

		LOG_INFO << "message:" << message;
	}
}

int main(int argc, char* argv[])
{	
	EventLoopThread loopThread;

	ChatClient chatClient(loopThread.startLoop(), InetAddress("127.0.0.1", 8000));
	chatClient.connect();

	std::string line;

	while (std::getline(std::cin, line))
	{
		chatClient.write(line);
	}
	chatClient.disconnect();

	CurrentThread::sleepUsec(30 * 1000000);
}

