#include "./../header/common.h"

void route_manager_init(void)
{
	tree_ip_fist    = RB_ROOT;
	tree_ip_second  = RB_ROOT;
	tree_ip_active  = &tree_ip_fist;
	tree_ip_passive = &tree_ip_second;
	// mutex for find init
	// mutex modify init
}

void route_manager_switch(void)
{
	struct rb_root *tmp;

	// mutex for find lock
	tmp             = tree_ip_active;
	tree_ip_active  = tree_ip_passive;
	tree_ip_passive = tmp;
	// mutex for find unlock
}

void route_manager_del(struct node_rule *rule)
{
	// mutex modify lock

	// mutex modify unlock
}

int route_manager_add(struct node_rule *rule)
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
			printk("manager_rule_add: invalid value %d\n", rule->route_type);
			return -EINVAL;
	}

	// mutex modify lock
	for (index = 0; index < count_parts; index++) {
		nip = node_ip_get(tree_ip_passive, &(rule->parts[index].ip));
		if (!nip) {
			nip = node_ip_new(rule, index);
			if (!nip) {
				printk("manager_rule_add: node_ip_new return null\n");
				goto abort;
			}
			node_ip_add(tree_ip_passive, nip);
		}
		else {
			nport = node_port_get(nip, rule->parts[index].port);
			if (nport) {
				printk("manager_rule_add: port %d is exist\n", rule->parts[index].port);
				goto abort;
			}
		}
		nport = node_port_new(rule, index);
		if (!nport) {
			printk("manager_rule_add: node_port_new return null\n");
			goto abort;
		}
		node_port_add(nip, nport);
	}
	// mutex modify unlock
	return 0;
abort:
	// mutex modify unlock
	route_manager_del(rule);
	return -ENOMEM;
}