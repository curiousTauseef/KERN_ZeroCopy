#ifndef _HEADER_ROUTE_MANAGER_H
#define _HEADER_ROUTE_MANAGER_H

#define DEBUG(type, message) \
	printk("%s %s %u: %s\n", type, __FUNCTION__, __LINE__, message)

void route_manager_init(void);
void route_manager_switch(void);
void route_manager_del(struct node_rule *);
void __route_manager_del(struct node_rule *);
int route_manager_add(struct node_rule *);
int __route_manager_add(struct node_rule *);
struct node_port *route_manager_get(struct sk_buff *);
struct node_port *__route_manager_get(struct in6_addr *, u16 );


#endif  /* _HEADER_ROUTE_MANAGER_H */