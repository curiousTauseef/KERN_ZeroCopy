#include "./../header/common.h"

LIST_HEAD(head_list_rule);

void node_rule_add(struct list_head *head, struct node_rule *nrule)
{
	INIT_LIST_HEAD(&nrule->lnode);
	list_add_tail(&nrule->lnode, head);
}

void node_rule_del(struct node_rule *nrule)
{
	list_del(&nrule->lnode);
	kfree(nrule);
}

void node_rule_dst(struct list_head *head)
{
	while (!list_empty(head))
	{
		struct node_rule *nrule;

		nrule = list_first_entry(head, struct node_rule, lnode);
		list_del(&nrule->lnode);
		kfree(nrule);
	}
}
