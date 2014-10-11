#ifndef __forward_client_h__
#define __forward_client_h__

#include <muduo/net/TcpClient.h>

namespace muduo
{
namespace net
{

class ForwardClient : boost::noncopyable 
{
public:
	ForwardClient(EventLoop *loop, const InetAddress &connAddr);
	void connect();
	void disconnect();
	void write(Buffer *buf);
	void write(const StringPiece &message);
	void write(const void *message, int len);

private:
	void onConnection(const TcpConnectionPtr &conn);
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp t);

private:
	TcpClient tcpClient_;
	TcpConnectionPtr tcpConn_;
	MutexLock mutex_;
};

}
}

#endif /* __forward_client_h__ */

