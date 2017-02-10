#ifndef _HEADER_NODE_PORT_H
#define _HEADER_NODE_PORT_H

struct node_port
{
	struct hlist_node hnode;
	u8                index_part; /* The index part of rules for search */
	struct node_rule  *rule;
};

void node_port_add(struct node_ip *, struct node_port *);
void node_port_del(struct node_port *);
void node_port_dst(struct node_ip *);
struct node_port *node_port_new(struct node_rule *, u8);
struct node_port *node_port_get(struct node_ip *, u16);

#endif  /* _HEADER_NODE_PORT_H */
