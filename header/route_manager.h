#ifndef _HEADER_ROUTE_MANAGER_H
#define _HEADER_ROUTE_MANAGER_H

void route_manager_init(void);
void route_manager_switch(void);
void route_manager_del(struct node_rule *);
int route_manager_add(struct node_rule *);

#endif  /* _HEADER_ROUTE_MANAGER_H */