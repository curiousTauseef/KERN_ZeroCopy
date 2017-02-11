#ifndef _HEADER_NODE_RILE_H
#define _HEADER_NODE_RILE_H

#define ROUTE_TYPE_SIMPLEX 1
#define ROUTE_TYPE_DUPLEX  2

struct node_rule_part
{
	struct in6_addr ip;   /* in6_addr for support IPv6. */
	u16 port;
};

struct node_rule
{
	struct list_head lnode;
	struct node_rule_part parts[2];

	pid_t pid;
	u8    route_type;
	u32   last_check_pid; /* The last time the check for 
			         the existence of the process. */
};

extern struct list_head head_list_rule;

void node_rule_add(struct list_head *, struct node_rule *);
void node_rule_del(struct node_rule *);
void node_rule_dst(struct list_head *);

#endif  /* _HEADER_NODE_RILE_H */
