#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>

#include "my_netlink.h"

MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Roman Ponomarenko <r.e.p@yandex.ru>");

static struct sock *nl_sock;

static void input(struct sk_buff *sk);

const static struct netlink_kernel_cfg net_cfg = {
	.input = input,
};

static void input(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	int pid;
	struct sk_buff *skb_out;
	int msg_size;
	struct my_struct data;
	int res;

	strcpy(data.mess, "Hello from kernel!");

	msg_size = sizeof(data);

	nlh = (struct nlmsghdr *)skb->data;
	pr_info("[my_netlink] received msg: %s\n",
			((struct my_struct *)NLMSG_DATA(nlh))->mess);
	pid = nlh->nlmsg_pid;

	skb_out = nlmsg_new(msg_size, 0);
	if (!skb_out) {
		pr_err("[my_netlink]Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0;
	memcpy(NLMSG_DATA(nlh), &data, msg_size);
	res = nlmsg_unicast(nl_sock, skb_out, pid);
	if (res < 0)
	pr_info("Error while sending back to user\n");
}

static int __init nl_init(void)
{
	nl_sock = netlink_kernel_create(&init_net, MY_NETLINK, &net_cfg);
	pr_info("[my_netlink] init\n");
	return 0;
}

static void __exit nl_exit(void)
{
	netlink_kernel_release(nl_sock);
	pr_info("[my_netlink] exit\n");
}

module_init(nl_init);
module_exit(nl_exit);
