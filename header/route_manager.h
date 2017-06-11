#ifndef _HEADER_ROUTE_MANAGER_H
#define _HEADER_ROUTE_MANAGER_H

struct sk_parsed
{
	void *header_mac;
	void *header_network;
	void *header_transport;
	void *payload;

	u16 len_mac;
	u16 len_network;
	u16 len_transport;
	u16 len_payload;

	u16 protocol_network;
	u8  protocol_transport;
};

extern spinlock_t spin_search;
extern struct mutex mutex_modify;

#define DEBUG(type, message) \
	printk("%s %s %u: %s\n", type, __FUNCTION__, __LINE__, message)

int route_manager_init(void);
void route_manager_destroy(void);
void route_manager_switch(void);
bool route_manager_parse(struct sk_buff *, struct sk_parsed *);

void route_manager_del(struct node_rule *);
void __route_manager_del(struct node_rule *);

int route_manager_add(struct node_rule *);
int __route_manager_add(struct node_rule *);

struct node_port *route_manager_get(struct sk_buff *);
struct node_port *__route_manager_get(struct in6_addr *, u16);

int route_manager_redirect(struct sk_buff *, const struct nf_hook_state *,
				    struct node_port *);

unsigned int route_manager_sniffer(void *, struct sk_buff *,
				   const struct nf_hook_state *);

#endif  /* _HEADER_ROUTE_MANAGER_H */
