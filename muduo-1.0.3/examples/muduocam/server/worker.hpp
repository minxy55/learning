#ifndef __workder_h__
#define __workder_h__

#include <sys/types.h>

#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/Logging.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

using muduo::Timestamp;
using namespace boost;

namespace muduo
{
namespace net
{

class SeeingBox
{
public:
	TcpConnectionPtr conn;
	Timestamp time;
};

typedef shared_ptr<SeeingBox> SeeingBoxPtr;
typedef unordered_map<TcpConnectionPtr, SeeingBoxPtr> SeeingBoxList;

class ChannelInfo
{
public:
	uint64_t key;
	unsigned char cw[16];
	SeeingBoxList seeing_box_list;
};

typedef shared_ptr<ChannelInfo> ChannelInfoPtr;
typedef unordered_map<uint64_t, ChannelInfoPtr> ChannelInfoList;

class ClientConn
{
public:
	TcpConnectionPtr conn;
	uint64_t key;
};

typedef shared_ptr<ClientConn> ClientConnPtr;
typedef unordered_map<TcpConnectionPtr, ClientConnPtr> ClientConnList;

class ThreadInfo
{
public:
	ChannelInfoList ch_list;
	ClientConnList cli_list;
};

typedef shared_ptr<ThreadInfo> ThreadInfoPtr;
typedef unordered_map<int, ThreadInfoPtr> ThrInfoMap;

class TcpServer1 : boost::noncopyable
{
public:
	TcpServer1(EventLoop *loop, const InetAddress &listenAddr, int numThreads);
	void start() { tcpServer_.start(); };

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer*buf, Timestamp ts);
	void onThreadInit(EventLoop *loop);

	void onInfoTimeout();

	void newConnection(ThreadInfoPtr &thi, const TcpConnectionPtr &conn);
	void removeConnection(ThreadInfoPtr &thi, const TcpConnectionPtr &conn);

	void newRequestion(ThreadInfoPtr &thi, unsigned char sppp[], const TcpConnectionPtr &conn, Timestamp ts);
	void newFeed(ThreadInfoPtr &thi, unsigned char sppp[], const TcpConnectionPtr &conn, Timestamp ts);

private:
	ThrInfoMap thi_map;
	TcpServer tcpServer_;
};

}
}

#endif

