#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// == command options ==:
//
//  ip_addrs <interface>
//      outputs info about the given interface
//  ip_addrs -6
//      only show interfaces with IPv6 addresses
//  ip_addrs -4
//      only show interfaces with IPv4 addresses
//  ip_addrs <up/down>
//      only show interfaces that are <up/down>
//  ip_addrs brief
//      only show brief info about interfaces

// define colors for printing

#define IF_NAME_COLOR "\x1B[1;36m"
#define INET_ADDR_COLOR "\x1B[1;35m"
#define INET6_ADDR_COLOR "\x1B[1;34m"
#define IF_UP_COLOR ""
#define IF_DOWN_COLOR ""
#define RESET_COLOR "\x1b[0m"

typedef struct inet_addr {
    char *addr;
    struct inet_addr *next;
} inet_addr;

typedef struct inet6_addr {
    char *addr;
    struct inet6_addr *next;
} inet6_addr;

typedef struct Interface {
    char *if_name;
    char *family;
    char *ether;
    inet_addr *inet;
    inet6_addr *inet6;
    struct Interface *next;
} Interface;
Interface *if_list;

typedef struct {
    char *if_name;
    char *family;
    char *inet;
    char *ether;
    char *inet6;
} IfInfo;

void put_if_info(IfInfo *info) {
    Interface *temp;
    for (temp = if_list; temp != NULL; temp = temp->next) {
        // interface already exists, add new info
        if (strcmp(temp->if_name, info->if_name) == 0) {

            if (info->inet != NULL) {
                // add new inet address
                inet_addr *a = malloc(sizeof(inet_addr));
                a->addr = strdup(info->inet);
                a->next = temp->inet;
                temp->inet = a;
            }

            if (info->inet6 != NULL) {
                // add new inet6 address
                inet6_addr *a = malloc(sizeof(inet6_addr));
                a->addr = strdup(info->inet6);
                a->next = temp->inet6;
                temp->inet6 = a;
            }

            return;
        }
    }
    // interface does not exist, add a new one (insert at front)
    Interface *n = calloc(1, sizeof(Interface));
    // n->ether = strdup(info->ether);
    n->family = strdup(info->family);
    n->if_name = strdup(info->if_name);

    if (info->inet != NULL) {
        // add new inet address
        n->inet = malloc(sizeof(inet_addr));
        n->inet->addr = strdup(info->inet);
        n->inet->next = NULL;
    }

    if (info->inet6 != NULL) {
        // add new inet6 address
        n->inet6 = malloc(sizeof(inet6_addr));
        n->inet6->addr = strdup(info->inet6);
        n->inet6->next = NULL;
    }

    n->next = if_list;
    if_list = n;

    return;
}

// returns a linked list of the interfaces with all their info
void get_addrs() {
    // Interface *ifs = malloc(sizeof(Interface));

    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];

    // try to initialize ifaddr linked list
    if (getifaddrs(&ifaddr) == -1) {
        perror("could not getifaddrs");
        exit(EXIT_FAILURE);
    }

    // iterate through ifaddrs adding to ifs
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;
        // only show AF_INET* interfaces
        if (family == AF_INET || family == AF_INET6) {
            char *fam = family == AF_INET ? "inet" : "inet6";
            IfInfo new_info = {0};
            new_info.if_name = ifa->ifa_name;
            new_info.family = fam;

            size_t addr_size = (family == AF_INET)
                                   ? sizeof(struct sockaddr_in)
                                   : sizeof(struct sockaddr_in6);

            s = getnameinfo(ifa->ifa_addr, addr_size, host, NI_MAXHOST, NULL, 0,
                            NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if (family == AF_INET) {
                new_info.inet = host;
            } else {
                new_info.inet6 = host;
            }

            put_if_info(&new_info);
        }
    }

    freeifaddrs(ifaddr);
    return;
}

int main(int argc, char *argv[]) {
    // hi
    get_addrs();
    if (if_list == NULL)
        fprintf(stderr, "[error]: no interfaces found\n");
    // max bytes that the address char array will take up

    // print out each interface and its addresses

    Interface *tmp;
    int num = 1;
    for (tmp = if_list; tmp != NULL; tmp = tmp->next) {
        printf(IF_NAME_COLOR "%d: %s\n" RESET_COLOR, num, tmp->if_name);
        printf("    inet: \n");
        inet_addr *inet_addrs;
        for (inet_addrs = tmp->inet; inet_addrs != NULL;
             inet_addrs = inet_addrs->next) {
            printf(INET_ADDR_COLOR "\t%s\n" RESET_COLOR, inet_addrs->addr);
        }

        printf("    inet6: \n");
        inet6_addr *inet6_addrs;
        for (inet6_addrs = tmp->inet6; inet6_addrs != NULL;
             inet6_addrs = inet6_addrs->next) {
            printf(INET6_ADDR_COLOR "\t%s\n" RESET_COLOR, inet6_addrs->addr);
        }
        num++;
    }

    return EXIT_SUCCESS;
}
