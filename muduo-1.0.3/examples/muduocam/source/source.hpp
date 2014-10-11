#ifndef __source_client_h__
#define __source_client_h__

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

namespace muduo
{
namespace net
{

class SourceClient : boost::noncopyable 
{
public:
	SourceClient(EventLoop *loop, const InetAddress &connAddr, int internal);
	void connect();
	void disconnect();
	void write(const StringPiece &message);
	void write(const void *message, int len);

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t);
	void onTimeout();

private:
	EventLoop *loop_;
	TcpClient tcpClient_;
	TcpConnectionPtr tcpConn_;
	MutexLock mutex_;

	int id_;
};

}
}

#endif /* __source_client_h__ */

