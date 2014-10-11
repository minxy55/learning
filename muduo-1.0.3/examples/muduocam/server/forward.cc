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
#include <boost/unordered_map.hpp>

#include <sys/socket.h>  // SO_REUSEPORT

#include "forward.hpp"

using namespace muduo;
using namespace muduo::net;

ForwardClient::ForwardClient(EventLoop *loop, const InetAddress &serverAddr) :
	tcpClient_(loop, serverAddr, "forward")
{	
	tcpClient_.setConnectionCallback(
		boost::bind(&ForwardClient::onConnection, this, _1));
	tcpClient_.setMessageCallback(
		boost::bind(&ForwardClient::onMessage, this, _1, _2, _3));
	tcpClient_.enableRetry();
}

void ForwardClient::connect()
{
	tcpClient_.connect();
}

void ForwardClient::disconnect()
{
	tcpClient_.disconnect();
}

void ForwardClient::write(const StringPiece &message)
{
	write(message.data(), message.size());
}

void ForwardClient::write(Buffer *buf)
{
	MutexLockGuard lock(mutex_);
	if (tcpConn_)
	{		
		tcpConn_->send(buf);
	}
}

void ForwardClient::write(const void *message, int len)
{
	Buffer buf;
	buf.append(message, len);
		
	write(&buf);
}

void ForwardClient::onConnection(const TcpConnectionPtr &conn)
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

void ForwardClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{
	if (buf->readableBytes() > 0)
	{
		buf->retrieve(buf->readableBytes());
	}
}

