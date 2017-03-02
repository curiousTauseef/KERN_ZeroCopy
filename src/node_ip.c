#include "./../header/common.h"

struct rb_root tree_ip_fist;
struct rb_root tree_ip_second;

struct rb_root *tree_ip_active;
struct rb_root *tree_ip_passive;

/**
 * node_ip_new - создает и заполняет узел дерева в соответствии
 * с правилом.
 *
 * @rule: указатель на правило.
 * @index: индекс части правила по которой будет производится поиск.
 *
 * Возвращает указатель на новый узел или NULL при ошибке.
 */
struct node_ip *node_ip_new(struct node_rule *rule, u8 index)
{
	struct node_ip *new_node;

	new_node = kmalloc(sizeof(struct node_ip), GFP_KERNEL);
	if (new_node) {
		new_node->rule = rule;
		new_node->index_part = index;
		hash_init(new_node->hashtable);
		RB_CLEAR_NODE(&new_node->tnode);
	}
	return new_node;
}

/**
 * node_ip_dst - удаляет все дерево и хэштаблицы узлов.
 * @root: указатель корень дерева.
 */
void node_ip_dst(struct rb_root *root)
{
	// Под вопросом
}

/**
 * node_ip_add - добавляет узел в дерево.
 *
 * @root: указатель на корень дерева.
 * @nip: указатель на добавляемый узел.
 */
void node_ip_add(struct rb_root *root, struct node_ip *nip)
{
	struct rb_node **new = &root->rb_node;
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

/**
 * node_ip_del - удаляет узел из дерева, включая его хэш таблицу портов.
 *
 * @root: указатель на корень дерева.
 * @nip: указатель на удаляемый узел.
 */
void node_ip_del(struct rb_root *root, struct node_ip *nip)
{
	rb_erase(&nip->tnode, root);
  	node_port_dst(nip);
  	kfree(nip);
}

/**
 * node_ip_get - производит поиск узела дерева по ip.
 *
 * @root: указатель на корень дерева.
 * @ip: искомый ip адрес.
 *
 * Возвращает NULL, если елемент не найден, иначе указатель на него.
 */
struct node_ip *node_ip_get(struct rb_root *root, struct in6_addr *ip)
{
	struct rb_node *tmp = root->rb_node;

	while (tmp) {
		struct node_ip *nip;
		int rez;

		nip = container_of(tmp, struct node_ip, tnode);
		rez = ipv6_addr_cmp(ip, 
				    &(nip->rule->parts[nip->index_part].ip));
		if (rez < 0)
  			tmp = tmp->rb_left;
		else if (rez > 0)
  			tmp = tmp->rb_right;
		else
  			return nip;
	}
	return NULL;
}

/**
 * convert_ipv4_to_ipv6 - выполняет преобразование ipv4 в ipv6.
 *
 * @ipv4: указатель на ip адрес верси 4.
 * @ipv6: указатель по которому будет размещен результат.
 */
void convert_ipv4_to_ipv6(u32 *ipv4, struct in6_addr *ipv6)
{
	memset(ipv6, 0, sizeof(struct in6_addr)); // Не обнулять первые 32?
	memmove(ipv6, ipv4, sizeof(u32));
}

/**
 * convert_ipv6_to_ipv4 - выполняет преобразование ipv6 в ipv4.
 *
 * @ipv6: указатель на ip адрес верси 6.
 * @ipv4: указатель по которому будет размещен результат.
 */
void convert_ipv6_to_ipv4(struct in6_addr *ipv6, u32 *ipv4)
{
	*ipv4 = *((u32 *)ipv6);
}
