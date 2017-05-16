#include "./../header/common.h"

void node_port_add(struct node_ip *nip, struct node_port *new)
{
	int key;

	key = (int)(new->rule->parts[new->index_part].port);
	hash_add(nip->hashtable, &(new->hnode), key);
}

void node_port_del(struct node_port *nport)
{
	hash_del(&nport->hnode);
	kfree(nport);
}

void node_port_dst(struct node_ip *nip)
{
	int key;
	struct node_port *nport;
	struct hlist_node *tmp;

	hash_for_each_safe(nip->hashtable, key, tmp, nport, hnode)
	{
		hash_del(&nport->hnode);
		kfree(nport);
	}
}

struct node_port *node_port_new(struct node_rule *rule, u8 index)
{
	struct node_port *new_node;

	new_node = kmalloc(sizeof(struct node_port), GFP_KERNEL);
	if (new_node)
	{
		new_node->rule = rule;
		new_node->index_part = index;
		INIT_HLIST_NODE(&new_node->hnode);
	}
	return new_node;
}

struct node_port *node_port_get(struct node_ip *nip, u16 port)
{
	struct node_port *nport;

	hash_for_each_possible(nip->hashtable, nport, hnode, (int)port)
	{
		if (nport->rule->parts[nport->index_part].port == port)
			return nport;
	}
	return NULL;
}
