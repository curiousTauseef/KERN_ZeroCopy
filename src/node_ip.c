#include "./../header/common.h"


struct rb_root tree_ip_fist;
struct rb_root tree_ip_second;

struct rb_root *tree_ip_active;
struct rb_root *tree_ip_passive;

void node_ip_add(struct rb_root *root, struct node_ip *nip)
{
	struct rb_node **new   = &root->rb_node;
	struct rb_node *parent = NULL;

	while (*new) {
		struct node_ip *tmp;
		int res;

		parent = *new;
		tmp = container_of(*new, struct node_ip, tnode);
		res = ipv6_addr_cmp(&(nip->rule->parts[nip->index_part].ip),
				    &(tmp->rule->parts[tmp->index_part].ip));
		if (res < 0)
			new = &((*new)->rb_left);
		else
			new = &((*new)->rb_right);
	}
	rb_link_node(&nip->tnode, parent, new);
  	rb_insert_color(&nip->tnode, root);
}

void node_ip_del(struct rb_root *root, struct node_ip *nip)
{
	rb_erase(&nip->tnode, root);
  	node_port_dst(nip);
  	kfree(nip);
}

void node_ip_dst(struct rb_root *root)
{
	// Под вопросом
}

struct node_ip *node_ip_new(struct node_rule *rule, u8 index)
{
	struct node_ip *new_node;

	new_node = kmalloc(sizeof(struct node_ip), GFP_KERNEL);
	if (new_node)
	{
		new_node->rule = rule;
		new_node->index_part = index;
		hash_init(new_node->hashtable);
		RB_CLEAR_NODE(&new_node->tnode);
	}
	return new_node;
}

struct node_ip *node_ip_get(struct rb_root *root, struct in6_addr *ip)
{
	struct rb_node *tmp = root->rb_node;

	while (tmp) {
		struct node_ip *nip;
		int rez;

		nip = container_of(tmp, struct node_ip, tnode);
		printk("node_ip_get: stack find %d\n", *((int *)(&(nip->rule->parts[nip->index_part].ip))));
		rez = ipv6_addr_cmp(ip, &(nip->rule->parts[nip->index_part].ip));

		if (rez < 0)
  			tmp = tmp->rb_left;
		else if (rez > 0)
  			tmp = tmp->rb_right;
		else
  			return nip;
	}
	return NULL;
}