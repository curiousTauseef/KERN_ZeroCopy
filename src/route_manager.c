#include "./../header/common.h"

spinlock_t spin_search;		/* To search for and safe switching trees. */
struct mutex mutex_modify;	/* To secure the modification of rules. */


void route_manager_init(void)
{
	tree_ip_fist    = RB_ROOT;
	tree_ip_second  = RB_ROOT;
	tree_ip_active  = &tree_ip_fist;
	tree_ip_passive = &tree_ip_second;

	spin_lock_init(&spin_search);
	mutex_init(&mutex_modify);
}

void route_manager_switch(void)
{
	struct rb_root *tmp;

	spin_lock(&spin_search);
	tmp             = tree_ip_active;
	tree_ip_active  = tree_ip_passive;
	tree_ip_passive = tmp;
	spin_unlock(&spin_search);
}

void route_manager_del(struct node_rule *rule)
{
	mutex_lock(&mutex_modify);
	__route_manager_del(rule);
	route_manager_switch();	
	__route_manager_del(rule);
	node_rule_del(rule);
	mutex_unlock(&mutex_modify);
}

void __route_manager_del(struct node_rule *rule)
{
	int index;
	int count_parts;
	struct node_ip *nip;
	struct node_port *nport;

	switch (rule->route_type) {
		case ROUTE_TYPE_SIMPLEX:
			count_parts = 1;
			break;
		case ROUTE_TYPE_DUPLEX:
			count_parts = 2;
			break;
		default:
			DEBUG("ERROR", "invalid toute type");
			return;
	}

	for (index = 0; index < count_parts; index++) {
		nip = node_ip_get(tree_ip_passive, &(rule->parts[index].ip));
		if (!nip) {
			DEBUG("WARNING", "node ip not found");
			return;
		}
		nport = node_port_get(nip, rule->parts[index].port);
		if (nport)
			node_port_del(nport);
		else
			DEBUG("WARNING", "node port not found");
		if (hash_empty(nip->hashtable))
			node_ip_del(tree_ip_passive, nip);
	}
}

int route_manager_add(struct node_rule *rule)
{
	int err;

	mutex_lock(&mutex_modify);
	node_rule_add(&head_list_rule, rule);
	err = __route_manager_add(rule);
	if (err)
		goto abort;
	route_manager_switch();
	err = __route_manager_add(rule);
	if (err)
		goto abort;
	mutex_unlock(&mutex_modify);
	return err;

abort:
	__route_manager_del(rule);
	route_manager_switch();
	__route_manager_del(rule);
	node_rule_del(rule);
	mutex_unlock(&mutex_modify);
	return err;
}


int __route_manager_add(struct node_rule *rule)
{
	int index;
	int count_parts;
	struct node_ip *nip;
	struct node_port *nport;

	switch (rule->route_type) {
		case ROUTE_TYPE_SIMPLEX:
			count_parts = 1;
			break;
		case ROUTE_TYPE_DUPLEX:
			count_parts = 2;
			break;
		default:
			DEBUG("ERROR", "invalid toute type");
			return -EINVAL;
	}

	for (index = 0; index < count_parts; index++) {
		nip = node_ip_get(tree_ip_passive, &(rule->parts[index].ip));
		if (!nip) {
			nip = node_ip_new(rule, index);
			if (!nip) {
				DEBUG("ERROR", "failed to create a node ip");
				goto err;
			}
			node_ip_add(tree_ip_passive, nip);
		}
		else {
			nport = node_port_get(nip, rule->parts[index].port);
			if (nport) {
				DEBUG("ERROR", "node port already exists");
				goto err;
			}
		}
		nport = node_port_new(rule, index);
		if (!nport) {
			DEBUG("ERROR", "failed to create the a node port");
			goto err;
		}
		node_port_add(nip, nport);
	}
	return 0;

err:
	return -ENOMEM;
}

struct node_port *route_manager_get(struct sk_buff *skb)
{
	struct ethhdr *eth;
	struct in6_addr ip_addr;
	u16 port;
	u8  proto_transport;
	u16 proto_network;

	// Get the network layer data.
	eth = eth_hdr(skb);
#if defined(__LITTLE_ENDIAN_BITFIELD)
	proto_network = ntohs(eth->h_proto);
#else
	proto_network = eth->h_proto;
#endif
	switch (proto_network) {
		case ETH_P_IP: {
			struct iphdr *ip;

			ip = ip_hdr(skb);
			proto_transport = ip->protocol;
			memset(&ip_addr, 0, sizeof(struct in6_addr));
			*((__be32 *)(&ip_addr)) = ip->saddr;
			break;
		}
		case ETH_P_IPV6: {
			struct ipv6hdr *ipv6;

			ipv6 = ipv6_hdr(skb);
			ip_addr = ipv6->saddr;
			proto_transport = ipv6->nexthdr;
			break;
		}
		default:
			goto not_found;
	}

	// Get the data transport layer level.
#if defined(__LITTLE_ENDIAN_BITFIELD)
	proto_transport = ntohs(proto_transport);
#endif
	switch (proto_transport) {
		case IPPROTO_TCP: {
			struct tcphdr *tcp;

			tcp = tcp_hdr(skb);
			port = (u16)tcp->source;
			break;
		}
		case IPPROTO_UDP: {
			struct udphdr *udp;

			udp = udp_hdr(skb);
			port = (u16)udp->source;
			break;
		}
		default:
			goto not_found;
	}

	// Find rule.
	return __route_manager_get(&ip_addr, port);
not_found:
	return NULL;
}

struct node_port *__route_manager_get(struct in6_addr *ip, u16 port)
{
	struct node_ip *nip;
	struct node_port *nport = NULL;

	spin_lock(&spin_search);
	nip = node_ip_get(tree_ip_active, ip);
	if (!nip)
		goto complete;
	nport = node_port_get(nip, port);

complete:
	spin_unlock(&spin_search);
	return nport;
}