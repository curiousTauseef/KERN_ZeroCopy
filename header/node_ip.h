#ifndef _HEADER_NODE_TREE_H
#define _HEADER_NODE_TREE_H

#define NODE_IP_HASH_SIZE 16

extern struct rb_root tree_ip_fist;
extern struct rb_root tree_ip_second;
extern struct rb_root *tree_ip_active;
extern struct rb_root *tree_ip_passive;

struct node_ip
{
	struct rb_node    tnode;
	struct hlist_head hashtable[1 << (NODE_IP_HASH_SIZE)];

	u8                index_part; /* The index part of rules for search */
	struct node_rule  *rule;
};

void node_ip_add(struct rb_root *, struct node_ip *);
void node_ip_del(struct rb_root *, struct node_ip *);
void node_ip_dst(struct rb_root *);	
struct node_ip *node_ip_new(struct node_rule *, u8);
struct node_ip *node_ip_get(struct rb_root *, struct in6_addr *);
void convert_ipv4_to_ipv6(u32 *, struct in6_addr *);
void convert_ipv6_to_ipv4(struct in6_addr *, u32 *);

#endif  /* _HEADER_NODE_TREE_H */
