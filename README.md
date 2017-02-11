# KERN_ZeroCopy
Zero Copy technology for the Linux operating system (TCP/IP). Realization on kernel space.

A rule consists of two pairs (IP, port) and service information.
It can be either a simplex (one-way directed) and duplex (sent in both directions).
In the future, it will be added the ability to set restrictions on the type of
protocol, traffic volume, time and number of packages.
* All the rules are stored in a list using container of kerne; linux/list.h
* For a quick search on the IP address rules are presented in the form of 
a red-black tree. Using a standard container of kernel linux/rbtree.h
* Because the limited number of ports 2^16, the search for their rules are represented as a hash table.
Using a standard container of kernel linux/hashtable.h

Example:
```
┌────────────────────────────────────────┐       ┌─────────────────┐
│ tree node                              |>>─┐   |  rule           |
│                                        |   |   | ┌──────┐        |
│ ┌────────────────────────┐             |   ├─>>| | ip   | x2     |
| |   hashtable            |             |   |   | | port |        |
| |   ┌────────────┐       |             |   |   | └──────┘        |
| | x | hash node  |>>───────────────────────┘   |  ...            |
| |   └────────────┘       |             |       └─────────────────┘
| └────────────────────────┘             |
└────────────────────────────────────────┘
```

# Structs #

```
    ┌───────────────────────────────────────┐           ┌───────────────────────────────────────┐
    │                                       │           │                                       │
┌─>>│         struct node_rule_part         │           │            struct node_ip             │
│   │                                       │           │                                       │
│   ├───────────────────────────────────────┤           ├───────────────────────────────────────┤
│   │ struct in6_addr ip                    │           │ struct rb_node tnode                  │
│   ├───────────────────────────────────────┤           ├───────────────────────────────────────┤
│   │ u16 port                              │           │ struct hlist_head hashtable[...]      │
│   └───────────────────────────────────────┘           ├───────────────────────────────────────┤
│                                                       │ u8 index_part                         │
│   ┌───────────────────────────────────────┐           ├───────────────────────────────────────┤
│   │                                       │     ┌───<<│ struct node_rule *rule                │
│   │           struct node_rule            │<<───┤     └───────────────────────────────────────┘
│   │                                       │     │
│   ├───────────────────────────────────────┤     │     ┌───────────────────────────────────────┐
│   │ struct list_head lnode                │     │     │                                       │
│   ├───────────────────────────────────────┤     │     │           struct node_port            │
└─<<│ struct node_rule_part parts[2]        │     │     │                                       │
    ├───────────────────────────────────────┤     │     ├───────────────────────────────────────┤
    │ pid_t pid                             │     │     │ struct hlist_node hnode               │
    ├───────────────────────────────────────┤     │     ├───────────────────────────────────────┤
    │ u8 route_type                         │     │     │ u8 index_part                         │
    ├───────────────────────────────────────┤     │     ├───────────────────────────────────────┤
    │ u32 last_check_pid                    │     └───<<│ struct node_rule *rule                │
    └───────────────────────────────────────┘           └───────────────────────────────────────┘
```
