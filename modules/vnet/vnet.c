
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>

#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>

MODULE_AUTHOR("Pengli");
MODULE_LICENSE("Dual BSD/GPL");

#define NR_RX_QSIZE	10

struct vnet_data {
	struct net_device_stats stats;
	spinlock_t lock;

	struct sk_buff *rx_skb[NR_RX_QSIZE];
	int rx_read;
	int rx_size;
};

static struct net_device *net_devs[2] = {NULL, NULL};

static int vnet_open(struct net_device *dev)
{
	memcpy(dev->dev_addr, "\0VNET0", ETH_ALEN);
	dev->addr_len = ETH_ALEN;
	if (dev == net_devs[1])
		dev->dev_addr[ETH_ALEN - 1]++;
	netif_start_queue(dev);

	return 0;
}

static int vnet_stop(struct net_device *dev)
{
	netif_stop_queue(dev);

	return 0;
}

static int vnet_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP)
		return -EBUSY;

	if (map->base_addr != dev->base_addr)
		return -EOPNOTSUPP;

	if (map->irq != dev->irq)
		dev->irq = map->irq;

	return 0;
}

static void vnet_rx(struct net_device *dev)
{
	unsigned long flags;
	struct vnet_data *priv = netdev_priv(dev);
	struct sk_buff *skb, *skb2;
	struct iphdr *ih;
	int size;

	spin_lock_irqsave(&priv->lock, flags);

	if (priv->rx_size <= 0) {
		spin_unlock_irqrestore(&priv->lock, flags);
		return;
	}

	skb = priv->rx_skb[priv->rx_read % NR_RX_QSIZE];
	size = priv->rx_size--;
	priv->rx_read++;
	priv->rx_read %= NR_RX_QSIZE;
	
	spin_unlock_irqrestore(&priv->lock, flags);

	ih = ip_hdr(skb);	
	printk(KERN_DEBUG "rx: %08x->%08x\n", ntohl(ih->saddr), ntohl(ih->daddr));

	skb2 = dev_alloc_skb(skb->len + 2);
	if (skb2 != NULL) {

		skb_reserve(skb2, 2);
		memcpy(skb_put(skb2, skb->len), skb->data, skb->len);
		
		skb2->dev = dev;
		skb2->protocol = eth_type_trans(skb2, dev);
		skb2->ip_summed = CHECKSUM_UNNECESSARY;

		netif_rx(skb2);
	}

	dev_kfree_skb(skb);
}

static void vnet_dump(int index, struct sk_buff *skb)
{
	struct ethhdr *eh = eth_hdr(skb);
	struct iphdr *ih = ip_hdr(skb);


	printk(KERN_DEBUG "vnet%d v %d, ihl %d, ttl %d, tos %d, protocol %d, tot_len %d\n", 
			index,
			ih->version, 
			ih->ihl,
			ih->ttl,
			ih->tos,
			ih->protocol, 
			ih->tot_len);

	if (ih->protocol == IPPROTO_UDP) {
		struct udphdr *uh = udp_hdr(skb);
		printk(KERN_DEBUG "UDP source %d, dest %d, len %d\n",
				uh->source, uh->dest, uh->len);
	}
	if (ih->protocol == IPPROTO_TCP) {
		struct tcphdr *th = tcp_hdr(skb);
		printk(KERN_DEBUG "TCP source %d, dest %d, seq %d, ack_seq %d, window %d\n",
				th->source, th->dest, th->seq, th->ack_seq, th->window);
	}
	if (ih->protocol == IPPROTO_ICMP) {
		//struct icmphdr *mh = icmp_hdr(skb);
		//printk(KERN_DEBUG "ICMP type %d, code %d\n",
		//		mh->type, mh->code);
	}
}

static int vnet_tx(struct sk_buff *skb, struct net_device *dev)
{
	unsigned long flags;
	unsigned int index = dev == net_devs[0] ? 0 : 1;
	unsigned int target = dev == net_devs[1] ? 0 : 1;
	struct vnet_data *priv = netdev_priv(net_devs[target]);
	struct iphdr *ih = ip_hdr(skb);
	u32 *saddr, *daddr;

	printk(KERN_DEBUG "tx: vnet%d %08x->%08x\n", index, ih->saddr, ih->daddr);

	dev->trans_start = jiffies;

	if (skb->len < sizeof(struct iphdr) + sizeof(struct ethhdr)) {
		printk(KERN_WARNING "too short\n");
		return 0;
	}
#if 0
	spin_lock_irqsave(&priv->lock, flags);

	if (priv->rx_size >= NR_RX_QSIZE) {
		spin_unlock_irqrestore(&priv->lock, flags);
		return NETDEV_TX_BUSY;
	}

	saddr = &ih->saddr;
	daddr = &ih->daddr;

	((u8*)saddr)[2] ^= 1;
	((u8*)daddr)[2] ^= 1;

	ih->check = 0;
	ih->check = ip_fast_csum((unsigned char *)ih, ih->ihl);

	priv->rx_skb[(priv->rx_read + priv->rx_size) % NR_RX_QSIZE] = skb;
	priv->rx_size++;
	priv->rx_size %= NR_RX_QSIZE;

	spin_unlock_irqrestore(&priv->lock, flags);

	vnet_rx(net_devs[target]);
#else
	vnet_dump(index, skb);
#endif
	
	return 0;
}

static int vnet_change_mtu(struct net_device *dev, int nmtu)
{
	unsigned long flags;
	struct vnet_data *priv = netdev_priv(dev);

	if (nmtu < 64 || nmtu > 1500)
		return -EINVAL;

	spin_lock_irqsave(&priv->lock, flags);
	dev->mtu = nmtu;
	spin_unlock_irqrestore(&priv->lock, flags);

	return 0;
}

static int vnet_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	return 0;
}

static void vnet_tx_timeout(struct net_device *dev)
{
	netif_wake_queue(dev);
}

static struct net_device_ops vnet_netdev_ops = {
	.ndo_open		= vnet_open,
	.ndo_stop		= vnet_stop,
	.ndo_start_xmit	= vnet_tx,
	.ndo_do_ioctl	= vnet_ioctl,
	.ndo_set_config	= vnet_config,
	.ndo_change_mtu	= vnet_change_mtu,
	.ndo_tx_timeout	= vnet_tx_timeout,
};

static int vnet_header(struct sk_buff *skb, struct net_device *dev,
			unsigned short type, const void *daddr, const void *saddr, 
			unsigned len)
{
	struct ethhdr *eh = (struct ethhdr*)skb_push(skb, ETH_HLEN);

	eh->h_proto = htons(type);
	memcpy(eh->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
	memcpy(eh->h_dest, daddr ? daddr : dev->dev_addr, dev->addr_len);
	eh->h_dest[ETH_ALEN - 1] ^= 1;

	return dev->hard_header_len;
}

static int vnet_rebuild_header(struct sk_buff *skb)
{
/*	struct net_device *dev = skb->dev;
	struct ethhdr *eh = (struct ethhdr*)skb->data;

	memcpy(eh->h_source, dev->dev_addr, dev->addr_len);
	memcpy(eh->h_dest, dev->dev_addr, dev->addr_len);
	eh->h_dest[ETH_ALEN - 1] ^= 1;
*/
	return 0;
}

static struct header_ops vnet_header_ops = {
	.create		= vnet_header,
	.rebuild	= vnet_rebuild_header
};


static __init int vnet_init(void)
{
	int i;
	struct vnet_data *priv;

	for (i = 0; i < 2; i++) {
		net_devs[i] = alloc_etherdev(sizeof(*priv));

		if (net_devs[i] == NULL) {
			goto out;
		}

		ether_setup(net_devs[i]);

		net_devs[i]->netdev_ops = &vnet_netdev_ops;
		net_devs[i]->header_ops = &vnet_header_ops;
		net_devs[i]->watchdog_timeo = 5;

		net_devs[i]->flags |= IFF_NOARP;
		net_devs[i]->features |= NETIF_F_HW_CSUM;

		priv = netdev_priv(net_devs[i]);

		memset(priv, 0, sizeof(*priv));

		spin_lock_init(&priv->lock);
	}

	for (i = 0; i < 2; i++) {
		int rc = register_netdev(net_devs[i]);
		if (rc) {
			printk(KERN_ERR "error %d registering device\n", i);
		}
	}

	return 0;

out:
	for (i = 0; i < 2; i++) {
		if (net_devs[i] != NULL)
			free_netdev(net_devs[i]);
	}
	return -1;
}

static __exit void vnet_cleanup(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		unregister_netdev(net_devs[i]);
		free_netdev(net_devs[i]);
	}
}

module_init(vnet_init);
module_exit(vnet_cleanup);

