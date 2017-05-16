#include "./../header/common.h"

spinlock_t spin_search;		/* Для безопасного поиска и переключения */
struct mutex mutex_modify;	/* Для безопасного добавления и удаления */
struct nf_hook_ops nfhops;

/**
 * route_manager_init - выполняет инициализацию все структур и
 * подготовку модуля.
 */
void route_manager_init(void)
{
	tree_ip_fist    = RB_ROOT;
	tree_ip_second  = RB_ROOT;
	tree_ip_active  = &tree_ip_fist;
	tree_ip_passive = &tree_ip_second;

	spin_lock_init(&spin_search);
	mutex_init(&mutex_modify);

	memset(&nfhops, 0, sizeof(struct nf_hook_ops));
	nfhops.hook	= route_manager_sniffer;
	nfhops.pf	= PF_INET;
	nfhops.hooknum	= NF_INET_PRE_ROUTING;
	nfhops.priority	= NF_IP_PRI_FIRST;
	nf_register_hook(&nfhops);
}

/**
 * route_manager_destroy - выполняет снятие хука для перехвата
 * трафика и очищает память.
 */
void route_manager_destroy(void)
{
	nf_unregister_hook(&nfhops);
	node_ip_dst(tree_ip_passive);
	node_ip_dst(tree_ip_active);
	node_rule_dst(&head_list_rule);
}

/**
 * route_manager_switch - производит переключение деревьев с активного
 * на пассивное.
 */
void route_manager_switch(void)
{
	struct rb_root *tmp;

	spin_lock(&spin_search);
	tmp = tree_ip_active;
	tree_ip_active = tree_ip_passive;
	tree_ip_passive = tmp;
	spin_unlock(&spin_search);
}

/**
 * route_manager_del - удаляет правило и все принадлежащие ему 
 * узлы ip и портов.
 */
void route_manager_del(struct node_rule *rule)
{
	mutex_lock(&mutex_modify);
	__route_manager_del(rule);
	route_manager_switch();	
	__route_manager_del(rule);
	node_rule_del(rule);
	mutex_unlock(&mutex_modify);
}

/**
 * __route_manager_del - удаляет правило и принадлежащие ему 
 * узлы ip и порта из пассивного дерева.
 */
void __route_manager_del(struct node_rule *rule)
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
			// DEBUG("ERROR", "invalid toute type");
			return;
	}

	for (index = 0; index < count_parts; index++) {
		nip = node_ip_get(tree_ip_passive, &(rule->parts[index].ip));
		if (!nip) {
			// DEBUG("WARNING", "node ip not found");
			return;
		}
		nport = node_port_get(nip, rule->parts[index].port);
		if (nport)
			node_port_del(nport);
		else
			// DEBUG("WARNING", "node port not found");
		if (hash_empty(nip->hashtable))
			node_ip_del(tree_ip_passive, nip);
	}
}

/**
 * route_manager_add - добавляет по правилу узлы в деревья
 * поиска ip и узелы в хэш таблицы портов для него.
 *
 * @rule: указатель на правило.
 *
 * Возвращает -ENOMEM при ошибке и 0 в случае успешного выполнения.
 */

int route_manager_add(struct node_rule *rule)
{
	int err;

	mutex_lock(&mutex_modify);
	node_rule_add(&head_list_rule, rule);
	err = __route_manager_add(rule);
	if (err)
		goto abort;
	route_manager_switch();
	err = __route_manager_add(rule);
	if (err)
		goto abort;
	mutex_unlock(&mutex_modify);
	return err;

abort:
	__route_manager_del(rule);
	route_manager_switch();
	__route_manager_del(rule);
	node_rule_del(rule);
	mutex_unlock(&mutex_modify);
	return err;
}

/**
 * __route_manager_add - добавляет по правилу узел в пассивное дерево
 * поиска ip и узел в хэш таблицу портов для него. 
 *
 * @rule: указатель на правило.
 *
 * Возвращает -ENOMEM при ошибке и 0 в случае успешного выполнения.
 */

int __route_manager_add(struct node_rule *rule)
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
			// DEBUG("ERROR", "invalid toute type");
			return -EINVAL;
	}

	/**
	 * В зависимости от того, какой тип правила добавление производится 
	 * 1 или 2 раза (при этом меняется только индекс правила в узле).
	 * Такое решение необходимо, чтобы можно было за один проход найти в
	 * правиле как по данным источника, так и по данным назначения.
	 */
	for (index = 0; index < count_parts; index++) {
		nip = node_ip_get(tree_ip_passive, &(rule->parts[index].ip));
		if (!nip) {
			nip = node_ip_new(rule, index);
			if (!nip) {
				DEBUG("ERROR", "failed to create a node ip");
				goto err;
			}
			node_ip_add(tree_ip_passive, nip);
		}
		else {
			nport = node_port_get(nip, rule->parts[index].port);
			if (nport) {
				DEBUG("ERROR", "node port already exists");
				goto err;
			}
		}
		nport = node_port_new(rule, index);
		if (!nport) {
			DEBUG("ERROR", "failed to create the a node port");
			goto err;
		}
		node_port_add(nip, nport);
	}
	return 0;

err:
	return -ENOMEM;
}


/**
 * route_manager_get - выполняет поиск узла, в котором содержится 
 * указатель на правило, по буферу сокета.
 *
 * @skb: указатель на буфер сокета.
 *
 * Возвращает указатель на узел порта, а в случае его отсутствия NULL.
 */
struct node_port *route_manager_get(struct sk_buff *skb)
{
	struct node_port *nport_finded;
	struct in6_addr ip;
	u16 port;

	u8 transport_protocol;
	u16 network_protocol;

	network_protocol = skb->protocol;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	network_protocol = ntohs(network_protocol);
#endif
	switch (network_protocol) {
		case ETH_P_IP: {
			struct iphdr *iph = ip_hdr(skb);
			transport_protocol = iph->protocol;
			convert_ipv4_to_ipv6(&(iph->daddr), &ip);
			break;
		}
		default:
			goto not_found;
	}
	switch (transport_protocol) {
		case IPPROTO_TCP: {
			struct tcphdr *tcph = tcp_hdr(skb);
			port = (u16)tcph->dest;
			break;
		}
		case IPPROTO_UDP: {
			struct udphdr *udph = udp_hdr(skb);
			port = (u16)udph->dest;
			break;
		}
		default:
			goto not_found;
	}

	spin_lock(&spin_search);
	nport_finded = __route_manager_get(&ip, port);
	spin_unlock(&spin_search);
	return nport_finded;

not_found:
	return NULL;
}

/**
 * __route_manager_get - выполняет поиск узла, в котором содержится 
 * указатель на правило, по ip и порту. Данная функция не выполняет 
 * спин блокировку для поиска, о чем следует помнить.
 * 
 * @ip: искомый ip.
 * @port: искомый порт.
 *
 * Возвращает указатель на узел порта, а в случае его отсутствия NULL.
 */

struct node_port *__route_manager_get(struct in6_addr *ip, u16 port)
{
	struct node_ip *nip;
	struct node_port *nport = NULL;

	nip = node_ip_get(tree_ip_active, ip);
	if (nip)
		nport = node_port_get(nip, port);
	return nport;
}

/**
 * route_manager_redirect - производит перенаправление пакета в соответствии
 * с правилом. Отлинковать полученный буфер сокета не удается, поэтому
 * происходит его копирование, модификация копии и ее отправка.
 * 
 * @skb: указатель на полученый буфер сокета.
 * @state: указатель на структуру с дополнительной информацией о буфере.
 * @nport: указатель на узел порта (результат поиска правила).
 *
 * Возвращает -ENOMEM при ошибке и 0 при успешном завершении.
 */

int route_manager_redirect(struct sk_buff *skb,
			   const struct nf_hook_state *state,
			   struct node_port *nport)
{
	struct net *net;
	struct rtable *rtable;
	//struct sk_buff *newskb;
	struct iphdr *iph;

	u8 transport_protocol;
	u16 network_protocol;

	/*newskb = skb_copy(skb, GFP_ATOMIC);
	if (!newskb) {
		// DEBUG("FATAL ERROR", "Unable to allocate memory");
		goto skip_packet;
	}*/
	skb->dev = state->in;
	//newskb->dev = state->in;
	net = dev_net(skb->dev);
	/*if (skb_dst(skb)->error) {
		// DEBUG("FATAL ERROR", "skb_dst failed");
		goto abort;
	}*/

	/**
	 * Для сетевого уровня обязательно необходимо заменить ip источника,
	 * т.к будут проблемы с дальнейшей маршрутезацией пакета в сети.
	 */
	network_protocol = skb->protocol;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	network_protocol = ntohs(network_protocol);
#endif
	switch (network_protocol) {
		case ETH_P_IP: {
			struct flowi4 fl4;
			struct in6_addr *ipv6;

			iph = ip_hdr(skb);
			ipv6 = &(nport->rule->parts[!(nport->index_part)].ip);
			transport_protocol = iph->protocol;

			fl4.saddr = iph->daddr;
			convert_ipv6_to_ipv4(ipv6, &(fl4.daddr));
			fl4.flowi4_oif = skb->dev->ifindex;

			iph->saddr = fl4.saddr;
			iph->daddr = fl4.daddr;

			rtable = ip_route_output_key(net, &fl4);
			skb_dst_set(skb, &rtable->dst);
			break;
		}
	}

	/**
	 * Для транспортного уровня замену порта источника производить не, 
	 * нужно, поскольку тогда, в случае дуплексного правла, обратно
	 * пакет приложение не получит.
	 */
	skb->csum = 0;
	switch (transport_protocol) {
		case IPPROTO_TCP: {
			struct tcphdr *tcph = tcp_hdr(skb);
			tcph->dest = nport->rule->parts[!(nport->index_part)].port;
			break;
		}
		case IPPROTO_UDP: {
			struct udphdr *udph = udp_hdr(skb);
			unsigned int offset = skb_transport_offset(skb);
			
			udph->check = 0;
			udph->dest = nport->rule->parts[!(nport->index_part)].port;
			skb->csum = skb_checksum(skb, offset, 
						    skb->len - offset, 0);
			udph->check = csum_tcpudp_magic(iph->saddr,
							iph->daddr,
							skb->len - offset, 
							IPPROTO_UDP,
							skb->csum);
			break;
		}
	}
	ip_local_out(net, skb->sk, skb);
	return 0;

/*abort:
	kfree_skb(newskb);
	*/
skip_packet:
	return -ENOMEM;
}

/**
 * route_manager_sniffer - вызывается при каждом получении пакета.
 * В функции происходится поиск правила и перенаправления пакета.
 * Список параметров см. linux/netfilter.h
 */
unsigned int route_manager_sniffer(void *priv, struct sk_buff *skb,
				   const struct nf_hook_state *state)
{
	struct node_port *nport = route_manager_get(skb);
	if (!nport)
		goto skip_packet;
	// DEBUG("INFO", "rule finded");
	if(route_manager_redirect(skb, state, nport))
		goto skip_packet;
	// else
	// 	DEBUG("INFO", "redirect sucsess");
	return NF_STOLEN;

skip_packet:
	return NF_ACCEPT;
}
