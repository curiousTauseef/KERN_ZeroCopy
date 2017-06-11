#include "uzcopy.h"

int sock_fd;

int uzcopy_init(void)
{
	struct sockaddr_nl addr;

	sock_fd = socket(AF_NETLINK, SOCK_RAW, UZCOPY_NETLINK_PROTO);
	if (sock_fd < 0) {
		return UZCOPY_STATUS_NOMEM;
	}

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		uzcopy_destroy();
		return UZCOPY_STATUS_EVAL;
	}

	return UZCOPY_STATUS_SUCCESS;
}

void uzcopy_destroy(void) 
{
	close(sock_fd);
}

int uzcopy_make_event(struct user_rule *rule) {

	struct sockaddr_nl addr;
	struct nlmsghdr *nlh = NULL;
	struct msghdr msg;
	struct iovec iov;

	memset(&addr, 0, sizeof(struct sockaddr_nl));
	memset(&iov, 0, sizeof(struct iovec));
	memset(&msg, 0, sizeof(struct msghdr));

	addr.nl_family = AF_NETLINK;
	nlh = malloc(NLMSG_SPACE(sizeof(struct user_rule)));
	nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct user_rule));
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_type  = NLMSG_MIN_TYPE + 1;
	memmove(NLMSG_DATA(nlh), rule, sizeof(struct user_rule));

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	msg.msg_name = (void *)&addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	
	if (sendmsg(sock_fd, &msg, 0) < 0) {
		return UZCOPY_STATUS_DISCONNECT;
	}
	else {
		if (recvmsg(sock_fd, &msg, 0) < 0) {
			return UZCOPY_STATUS_DISCONNECT;
		}
		else {
			return *((int *)NLMSG_DATA(nlh));
		}
	}
}