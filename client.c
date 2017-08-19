#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "my_netlink.h"

int main(int argc, char *argv[])
{
	int nl_sock = socket(PF_NETLINK, SOCK_RAW, MY_NETLINK);
	if (nl_sock < 0) {
		perror("socket");
		return 1;
	}
	struct sockaddr_nl my;
	memset(&my, 0, sizeof(struct sockaddr_nl));
	my.nl_family = AF_NETLINK;
	my.nl_pid = getpid();
	my.nl_groups = 0;

	bind(nl_sock, (struct sockaddr*) &my, sizeof(my));

	struct sockaddr_nl dest;
	memset(&dest, 0, sizeof(struct sockaddr_nl));
	dest.nl_family = AF_NETLINK;
	dest.nl_pid = 0;
	dest.nl_groups = 0;

	struct nlmsghdr *nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(sizeof(struct my_struct)));
	memset(nlh, 0, NLMSG_SPACE(sizeof(struct my_struct)));
	nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct my_struct));
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	struct my_struct my_s;
	strcpy(my_s.mess, "Hello from client");

	//strcpy(NLMSG_DATA(nlh), "Hello from client");
	memcpy(NLMSG_DATA(nlh), &my_s, sizeof(my_s));

	struct iovec iov;
	iov.iov_base = nlh;
	iov.iov_len = nlh->nlmsg_len;

	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &dest;
	msg.msg_namelen = sizeof(dest);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if(sendmsg(nl_sock, &msg, 0) < 0)
	{
		perror("sendmsg");
		return 1;
	}


	memset(nlh, 0, NLMSG_SPACE(sizeof(struct my_struct)));
	if(recvmsg(nl_sock, &msg, 0) < 0)
	{
		perror("recvmsg");
		return 1;
	}


	printf("recieved message: %s\n", (char*)NLMSG_DATA(nlh));//((struct my_struct*)NLMSG_DATA(nlh))->mess);
	close(nl_sock);
	free(nlh);

	return 0;
}
