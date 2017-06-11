#ifndef _HEADER_UZCOPY_H
#define _HEADER_UZCOPY_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>

#define UZCOPY_NETLINK_PROTO 17

#define UZCOPY_ROUTE_SIMPLEX 0x1
#define UZCOPY_ROUTE_DUPLEX  0x2

#define UZCOPY_ACTION_ADD    0x1
#define UZCOPY_ACTION_DELETE 0x2

// Error status
#define UZCOPY_STATUS_SUCCESS     0
#define UZCOPY_STATUS_NOMEM      -2
#define UZCOPY_STATUS_NOTFOUND   -3
#define UZCOPY_STATUS_EVAL       -4
#define UZCOPY_STATUS_DISCONNECT -12
#define UZCOPY_STATUS_DUPLICATE  -12

struct user_rule
{
	struct in6_addr ip_for;
	struct in6_addr ip_to;
	unsigned short port_for;
	unsigned short port_to;
	unsigned char route_type;
	unsigned char action;
};


int uzcopy_init(void);
int uzcopy_make_event(struct user_rule *);
void uzcopy_destroy(void);

#endif  /* _HEADER_UZCOPY_H */