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

#include "source.hpp"

using namespace muduo;
using namespace muduo::net;

SourceClient::SourceClient(EventLoop *loop, const InetAddress &serverAddr, int internal) :
	tcpClient_(loop, serverAddr, "source")
{	
	tcpClient_.setConnectionCallback(
		boost::bind(&SourceClient::onConnection, this, _1));
	tcpClient_.setMessageCallback(
		boost::bind(&SourceClient::onMessage, this, _1, _2, _3));
	tcpClient_.enableRetry();
	loop_ = loop;

	id_ = ::rand();
	loop_->runEvery(internal, boost::bind(&SourceClient::onTimeout, this));
}

void SourceClient::connect()
{
	tcpClient_.connect();
}

void SourceClient::disconnect()
{
	tcpClient_.disconnect();
}

void SourceClient::write(const StringPiece &message)
{
	write(message.data(), message.size());
}

void SourceClient::write(const void *message, int len)

{
	MutexLockGuard lock(mutex_);
	if (tcpConn_)
	{
		//LOG_INFO << "write: " << message;
		
		Buffer buf;

		buf.append(message, len);
		
		tcpConn_->send(&buf);
	}
}

void SourceClient::onConnection(const TcpConnectionPtr &conn)
{
	LOG_DEBUG << conn->localAddress().toIpPort() << " -> "
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

void SourceClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{
	if (buf->readableBytes() > 0)
	{
		buf->retrieve(buf->readableBytes());
	}
}

void SourceClient::onTimeout()
{
	LOG_DEBUG << " feed";
	
	for (int i = 0; i < 20; i++)
	{
		if (tcpConn_)
		{
			unsigned char fee[25];
			
			fee[0] = 0xf7;
			fee[1] = 0x0a;
			fee[2] = 0x00;
			fee[3] = 0x01;
			fee[4] = static_cast<unsigned char>((id_ >> 24) & 0xff);
			fee[5] = static_cast<unsigned char>((id_ >> 16) & 0xff);
			fee[6] = static_cast<unsigned char>((id_ >> 8) & 0xff);
			fee[7] = static_cast<unsigned char>((id_ >> 0) & 0xff);
			fee[24] = 0x00;

			tcpConn_->send(fee, 25);
		}

		id_++;
	}
}

