#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/Logging.h>


#include <sys/types.h>
#include <sys/socket.h>  // SO_REUSEPORT

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "worker.hpp"

using namespace boost;
using namespace muduo;
using namespace muduo::net;

TcpServer1::TcpServer1(EventLoop *loop, const InetAddress& listenAddr, int numThreads)
	:tcpServer_(loop, listenAddr, "worker")
{
	tcpServer_.setThreadNum(numThreads);
	tcpServer_.setConnectionCallback(
		boost::bind(&TcpServer1::onConnection, this, _1));
	tcpServer_.setMessageCallback(
		boost::bind(&TcpServer1::onMessage, this, _1, _2, _3));
	tcpServer_.setThreadInitCallback(
		boost::bind(&TcpServer1::onThreadInit, this, _1));
}

void TcpServer1::onConnection(const TcpConnectionPtr &conn)
{
	int tid = CurrentThread::tid();

	LOG_DEBUG << " ->" << (conn->connected() ? "UP" : "DOWN");

	ThrInfoMap::iterator it = thi_map.find(tid);
	if (it == thi_map.end())
	{
		LOG_ERROR << "there is no tid info " << tid;
		return;
	}

	ThreadInfoPtr thi = it->second;
	
	if (conn->connected())
	{
		newConnection(thi, conn);
	}
	else
	{
		removeConnection(thi, conn);
	}
}

#define REQ_SYNC_BYTE	0xf7
#define REQ_PKT_LEN		9

#define FEED_SYNC_BYTE	0xf8
#define FEED_PKT_LEN	25

#define BUF_LEN			25

void TcpServer1::onMessage(const TcpConnectionPtr &conn, Buffer*buf, Timestamp ts)
{
	int tid = CurrentThread::tid();
	const void *data;
	unsigned char sync, tag;
	unsigned char sp[BUF_LEN];
	
	LOG_DEBUG << " readalbeBytes->" << buf->readableBytes();

	ThrInfoMap::iterator it = thi_map.find(tid);
	if (it == thi_map.end())
	{
		LOG_ERROR << "there is no tid info " << tid;
		return;
	}

	ThreadInfoPtr thi = it->second;

	while (buf->readableBytes() >= REQ_PKT_LEN || buf->readableBytes() >= FEED_PKT_LEN)
	{
		data = buf->peek();
		sync = *static_cast<const unsigned char *>(data);

		if (sync == 0xf7)
		{
			tag = *(static_cast<const unsigned char *>(data) + 1);
			
			if (tag == 0x0a)
			{
				::memcpy(sp, data, FEED_PKT_LEN);
				buf->retrieve(FEED_PKT_LEN);
				
				newFeed(thi, sp, conn, ts);
			}
			else if (tag == 0x09)
			{
				::memcpy(sp, data, REQ_PKT_LEN);
				buf->retrieve(REQ_PKT_LEN);
				
				newRequestion(thi, sp, conn, ts);
			}
			else
			{
				buf->retrieve(sizeof(unsigned char));
			}
		}
		else
		{
			buf->retrieve(sizeof(unsigned char));
		}
	}
}

void TcpServer1::onThreadInit(EventLoop *loop)
{
	int tid = CurrentThread::tid();

	ThrInfoMap::iterator it = thi_map.find(tid);
	if (it != thi_map.end())
	{
		LOG_INFO << "already exist thread info";
	}
	
	LOG_DEBUG << " create thread info";
		
	ThreadInfoPtr tip(new ThreadInfo());
		
	thi_map.insert(std::pair<int, ThreadInfoPtr>(tid, tip));
	loop->runEvery(30, boost::bind(&TcpServer1::onInfoTimeout, this));
}

void TcpServer1::newConnection(ThreadInfoPtr &thi, const TcpConnectionPtr &conn)
{
	int tid = CurrentThread::tid();
	LOG_DEBUG << " new client came";
	
	ClientConnList::iterator it = thi->cli_list.find(conn);
	if (it != thi->cli_list.end())
	{
		LOG_INFO << "there is conn info " << tid << ", but I don't know why it is here";
	}

	ClientConnPtr ccp(new ClientConn());
	ccp->conn	= conn;
	ccp->key	= 0;

	thi->cli_list.insert(std::pair<TcpConnectionPtr, ClientConnPtr>(conn, ccp));
}

void TcpServer1::removeConnection(ThreadInfoPtr &thi, const TcpConnectionPtr &conn)
{
	int tid = CurrentThread::tid();
	
	ClientConnList::iterator it_cc = thi->cli_list.find(conn);
	if (it_cc == thi->cli_list.end())
	{
		LOG_ERROR << "can not find ClientConn with conn " << conn << " @tid " << tid;
		return;
	}

	ClientConnPtr ccp = it_cc->second;
	if (ccp == NULL)
	{
		LOG_ERROR << "not ClientConn" << " @tid " << tid;
		return;
	}
	
	thi->cli_list.erase(ccp->conn);

	if (ccp->key > 0ULL)
	{
		ChannelInfoList::iterator it_ci = thi->ch_list.find(ccp->key);
		if (it_ci == thi->ch_list.end())
		{
			LOG_INFO << "not find channel with key " << ccp->key << " @tid " << tid;
			return;
		}

		ChannelInfoPtr cip = it_ci->second;
		SeeingBoxList::iterator it_sbl = cip->seeing_box_list.find(ccp->conn);
		if (it_sbl == cip->seeing_box_list.end())
		{
			LOG_INFO << "no client info in group @tid " << tid;
			return;
		}

		cip->seeing_box_list.erase(ccp->conn);
		LOG_DEBUG << " client removed";
	}
}

void TcpServer1::onInfoTimeout()
{
	int tid = CurrentThread::tid();

	ThrInfoMap::iterator it = thi_map.find(tid);
	if (it == thi_map.end())
	{
		LOG_INFO << "timeout";
		return;
	}
	
	// cleanup channel_info which was out of date
}

void TcpServer1::newRequestion(ThreadInfoPtr &thi, unsigned char sp[], const TcpConnectionPtr &conn, Timestamp ts)
{
	unsigned int deg;
	unsigned int freq;
	unsigned int sid;
	uint64_t key;
	unsigned char resp[19];

	LOG_DEBUG << " new request came";

	deg = sp[2]; 
	deg = (deg << 8) + sp[3];
	freq = sp[4]; 
	freq = (freq << 8) + sp[5];
	sid = sp[6]; 
	sid = (sid << 8) + sp[7];
	key = deg;
	key += (key << 32) + (freq << 16) + sid;

	ClientConnList::iterator it_cc = thi->cli_list.find(conn);
	if (it_cc == thi->cli_list.end())
	{
		LOG_INFO << "I can not find conn info";
		return;
	}

	SeeingBoxPtr sbp;
	ClientConnPtr ccp = it_cc->second;
	ChannelInfoList::iterator it_cil = thi->ch_list.find(ccp->key);
	if (it_cil != thi->ch_list.end())
	{
		ChannelInfoPtr cip = it_cil->second;
		SeeingBoxList::iterator it_sbl = cip->seeing_box_list.find(ccp->conn);
		if (it_sbl != cip->seeing_box_list.end())
		{	
			sbp = it_sbl->second;
			cip->seeing_box_list.erase(ccp->conn);
			LOG_DEBUG << " move box_info";
		}
		else
		{
			sbp = SeeingBoxPtr(new SeeingBox());
			sbp->conn = conn;
			LOG_DEBUG << " add box_info";
		}
	}
	else
	{
		sbp = SeeingBoxPtr(new SeeingBox());
		sbp->conn = conn;
	}

	ccp->key = key;
		
	ChannelInfoList::iterator it_cil2 = thi->ch_list.find(key);
	if (it_cil2 != thi->ch_list.end())
	{
		LOG_DEBUG << " add box_info & reply cw";
		
		ChannelInfoPtr cip = it_cil->second;
		cip->seeing_box_list.insert(std::pair<TcpConnectionPtr, SeeingBoxPtr>(ccp->conn, sbp));
		
		::memcpy(&resp[3], cip->cw, 16);
	}
	else
	{
		LOG_DEBUG << " add channal_info & box_info, then reply null cw";
		
		ChannelInfoPtr cip(new ChannelInfo());
		::memset(cip->cw, 0xff, 16);
		
		cip->seeing_box_list.insert(std::pair<TcpConnectionPtr, SeeingBoxPtr>(ccp->conn, sbp));
		thi->ch_list.insert(std::pair<uint64_t, ChannelInfoPtr>(key, cip));
		
		::memcpy(&resp[3], cip->cw, 16);
	}

	resp[0] = 0xf7;
	resp[1] = 0x00;
	resp[2] = 0x00;
	conn->send(resp, 19);
}

void TcpServer1::newFeed(ThreadInfoPtr &thi, unsigned char sp[], const TcpConnectionPtr &conn, Timestamp ts)
{
	unsigned int deg;
	unsigned int freq;
	unsigned int sid;
	uint64_t key;
	unsigned char reply[19];
	
	LOG_DEBUG << " new feed came";

	deg = sp[2]; 
	deg = (deg << 8) + sp[3];
	freq = sp[4]; 
	freq = (freq << 8) + sp[5];
	sid = sp[6]; 
	sid = (sid << 8) + sp[7];
	key = deg;
	key += (key << 32) + (freq << 16) + sid;

	ChannelInfoPtr cip;
	ChannelInfoList::iterator it_cil = thi->ch_list.find(key);
	if (it_cil != thi->ch_list.end())
	{
		cip = it_cil->second;
		::memcpy(cip->cw, &sp[8], 16);
	}
	else
	{
		cip = ChannelInfoPtr(new ChannelInfo());
		::memcpy(cip->cw, &sp[8], 16);
	}

	LOG_DEBUG << " notify observers";
	reply[0] = 0xf7;
	reply[1] = 0x00;
	reply[2] = 0x00;
	::memcpy(&reply[3], cip->cw, 16);
	
	SeeingBoxList::iterator it_sbl = cip->seeing_box_list.begin();
	for (; it_sbl != cip->seeing_box_list.end(); it_sbl++)
	{
		SeeingBoxPtr sbp = it_sbl->second;
		TcpConnectionPtr box_conn = sbp->conn;

		box_conn->send(reply, 19);
	}
}

