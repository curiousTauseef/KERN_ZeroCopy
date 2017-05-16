#include "./../header/common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivannikov Igor");

static struct in6_addr convert_ip4_to_ip6(u32 ip4)
{
	struct in6_addr ip6;

	memset(&ip6, 0, sizeof(struct in6_addr));
	memcpy(&ip6, &ip4, sizeof(u32));
	return ip6;
}

static void generate_rand_port(u16 *port)
{
	get_random_bytes(port, sizeof(u16));
}

static void generate_rand_ip(struct in6_addr *ip)
{
	memset(ip, 0, sizeof(struct in6_addr));
	get_random_bytes(ip, sizeof(int)); // Only for IPv4.
}
/*
static void generate_rand_rules(u32 count_ip, u32 count_port)
{
	struct node_rule *rule     = NULL;
	struct node_rule *rule_old = NULL;
	struct node_ip   *nip;
	struct node_port *nport;
	int num_ip;
	int num_port;

	int circle = 0;
	for (num_ip = 0; num_ip < count_ip; num_ip++) {
		if (!rule) {
			rule = kmalloc(sizeof(struct node_rule), GFP_KERNEL);
			if (!rule) {
				DEBUG("FATAL ERROR", "memory error");
				return;
			}
			rule->route_type = ROUTE_TYPE_SIMPLEX;
			//printk("INFO: create new rule by ip %p\n", rule);
		}
		generate_rand_ip(&(rule->parts[0].ip));
		generate_rand_ip(&(rule->parts[1].ip));
		//rule->parts[0].ip = convert_ip4_to_ip6(htonl(0xC0A8124E)); // 192.168.18.78
		//rule->parts[1].ip = convert_ip4_to_ip6(htonl(0xC0A8124D)); // 192.168.18.77

		nip = node_ip_get(tree_ip_active, &(rule->parts[0].ip));
		if (nip) {
			DEBUG("INFO", "ip is exist");
			num_ip--;
			continue;
		}
		rule_old = rule;
		for (num_port = 0; num_port < count_port; num_port++) {
			if (circle > 10) return;
			if (!rule) {
				rule = kmalloc(sizeof(struct node_rule), GFP_KERNEL);
				memcpy(rule, rule_old, sizeof(struct node_rule));
				generate_rand_ip(&(rule->parts[1].ip));
				//printk("INFO: create new rule by port %p\n", rule);
			}
			generate_rand_port(&(rule->parts[0].port));
			generate_rand_port(&(rule->parts[1].port));
			if (nip) {
				nport = node_port_get(nip, rule->parts[0].port);
				if (nport) {
					printk("INFO: port %d is exist for rule %p\n",rule->parts[0].port, rule);
					num_port--;
					circle++;
					continue;
				}
			}
			if (route_manager_add(rule)){
				DEBUG("FATAL ERROR", "error add rule");
				return;
			}
			if (!nip) {
				nip = node_ip_get(tree_ip_active, &(rule->parts[0].ip));
				if (!nip) {
					DEBUG("FATAL ERROR", "not fount node ip after add");
					return;
				}
			}
			rule = NULL;
			circle = 0;
		}
	}
}*/

int init_module(void)
{
	struct node_rule *rule, *rule_new;
	int count_port = 100000;

	route_manager_init();
	rule = kmalloc(sizeof(struct node_rule), GFP_KERNEL);
	rule->route_type = ROUTE_TYPE_SIMPLEX;
	rule->parts[0].ip = convert_ip4_to_ip6(htonl(0x7F000001));
	rule->parts[1].ip = convert_ip4_to_ip6(htonl(0x7F000001));

	for (int i = 0; i < count_port; i++) {
		rule_new = kmalloc(sizeof(struct node_rule), GFP_KERNEL);
		memmove(rule_new, rule, sizeof(struct node_rule));
		if (i == count_port / 2)
		{
			rule_new->parts[0].port = htons(20501);
			rule_new->parts[1].port = htons(20502);
		}
		else
		{
			generate_rand_port(&(rule_new->parts[0].port));
			generate_rand_port(&(rule_new->parts[1].port));
			if (rule_new->parts[0].port == htons(20501)) {
				i--;
				continue;
			}
		}
		if (route_manager_add(rule_new)) {
			DEBUG("WARNING", "error add rule");
		}
	}
	DEBUG("INFO", "\nmodule installed\n");
	return 0;
}
void cleanup_module()
{
	route_manager_destroy();
	DEBUG("INFO", "\nmodule removed\n");
}
