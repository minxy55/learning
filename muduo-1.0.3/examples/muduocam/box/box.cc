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

#include "box.hpp"

using namespace muduo;
using namespace muduo::net;

BoxClient::BoxClient(EventLoop *loop, const InetAddress &serverAddr) :
	tcpClient_(loop, serverAddr, "box")
{	
	tcpClient_.setConnectionCallback(
		boost::bind(&BoxClient::onConnection, this, _1));
	tcpClient_.setMessageCallback(
		boost::bind(&BoxClient::onMessage, this, _1, _2, _3));
	tcpClient_.enableRetry();
	loop_ = loop;

	id_ = ::rand();
	loop_->runEvery(1, boost::bind(&BoxClient::onTimeout, this));
}

void BoxClient::connect()
{
	tcpClient_.connect();
}

void BoxClient::disconnect()
{
	tcpClient_.disconnect();
}

void BoxClient::write(const StringPiece &message)
{
	write(message.data(), message.size());
}

void BoxClient::write(const void *message, int len)

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

void BoxClient::onConnection(const TcpConnectionPtr &conn)
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

void BoxClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t)
{
	if (buf->readableBytes() >= 19)
	{
		LOG_DEBUG << " new reply";
		
		buf->retrieve(buf->readableBytes());
	}
}

void BoxClient::onTimeout()
{
	if (tcpConn_)
	{
		unsigned char req[9];
		
		req[0] = 0xf7;
		req[1] = 0x09;
		req[2] = 0x00;
		req[3] = 0x01;
		req[4] = static_cast<unsigned char>((id_ >> 24) & 0xff);
		req[5] = static_cast<unsigned char>((id_ >> 16) & 0xff);
		req[6] = static_cast<unsigned char>((id_ >> 8) & 0xff);
		req[7] = static_cast<unsigned char>((id_ >> 0) & 0xff);
		req[8] = 0;

		LOG_DEBUG << " request";
		tcpConn_->send(req, 9);
	}

	id_++;
}

