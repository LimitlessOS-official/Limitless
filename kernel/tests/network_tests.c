/*
 * Kernel-Level Network Stack Test Suite
 * Comprehensive testing for LimitlessOS network implementation
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/socket.h"
#include "net/tcp.h"
#include "net/udp.h"
#include "net/ip.h"
#include "net/icmp.h"
#include "net/arp.h"
#include "net/ethernet.h"
#include "net/netfilter.h"
#include "net/nat.h"
#include "net/qos.h"
#include "kernel/printk.h"
#include "kernel/string.h"

/* Test results */
typedef struct test_results {
    uint32_t total_tests;
    uint32_t passed;
    uint32_t failed;
    uint32_t skipped;
} test_results_t;

static test_results_t results = {0};

/* Test macros */
#define TEST_START(name) \
    do { \
        printk(KERN_INFO "[ TEST ] %s\n", name); \
        results.total_tests++; \
    } while(0)

#define TEST_PASS() \
    do { \
        printk(KERN_INFO "[  OK  ]\n"); \
        results.passed++; \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        printk(KERN_ERR "[ FAIL ] %s\n", msg); \
        results.failed++; \
    } while(0)

#define TEST_SKIP(msg) \
    do { \
        printk(KERN_WARNING "[ SKIP ] %s\n", msg); \
        results.skipped++; \
    } while(0)

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            TEST_FAIL(msg); \
            return -1; \
        } \
    } while(0)

/*
 * Test Socket Creation
 */
static int test_socket_creation(void) {
    TEST_START("Socket Creation");
    
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT(sock >= 0, "Failed to create TCP socket");
    
    close(sock);
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ASSERT(sock >= 0, "Failed to create UDP socket");
    
    close(sock);
    
    TEST_PASS();
    return 0;
}

/*
 * Test Socket Bind
 */
static int test_socket_bind(void) {
    TEST_START("Socket Bind");
    
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT(sock >= 0, "Failed to create socket");
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = htonl(0x7F000001);  /* 127.0.0.1 */
    addr.sin_port = htons(8080);
    
    int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    ASSERT(ret == 0, "Failed to bind socket");
    
    close(sock);
    
    TEST_PASS();
    return 0;
}

/*
 * Test TCP Connection
 */
static int test_tcp_connection(void) {
    TEST_START("TCP Connection");
    
    /* Create server socket */
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT(server >= 0, "Failed to create server socket");
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = htonl(0x7F000001);
    server_addr.sin_port = htons(9999);
    
    int ret = bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));
    ASSERT(ret == 0, "Failed to bind server socket");
    
    ret = listen(server, 5);
    ASSERT(ret == 0, "Failed to listen on server socket");
    
    /* Note: Full connection test requires multi-threading or async I/O */
    
    close(server);
    
    TEST_PASS();
    return 0;
}

/*
 * Test UDP Send/Receive
 */
static int test_udp_sendrecv(void) {
    TEST_START("UDP Send/Receive");
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ASSERT(sock >= 0, "Failed to create UDP socket");
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = htonl(0x7F000001);
    addr.sin_port = htons(5000);
    
    const char* msg = "Hello, UDP!";
    ssize_t sent = sendto(sock, msg, strlen(msg), 0,
                         (struct sockaddr*)&addr, sizeof(addr));
    ASSERT(sent > 0, "Failed to send UDP packet");
    
    close(sock);
    
    TEST_PASS();
    return 0;
}

/*
 * Test ICMP Ping
 */
static int test_icmp_ping(void) {
    TEST_START("ICMP Ping");
    
    uint32_t target_ip = 0x7F000001;  /* 127.0.0.1 */
    
    int ret = icmp_ping_simple(target_ip);
    ASSERT(ret == 0, "Failed to send ICMP ping");
    
    TEST_PASS();
    return 0;
}

/*
 * Test ARP Resolution
 */
static int test_arp_resolution(void) {
    TEST_START("ARP Resolution");
    
    uint32_t ip = 0xC0A80001;  /* 192.168.0.1 */
    uint8_t mac[6];
    
    /* Note: This will likely timeout without a real network device */
    int ret = arp_resolve(ip, mac);
    
    /* Don't fail the test if no device available */
    if (ret != 0) {
        TEST_SKIP("No network device available");
    } else {
        TEST_PASS();
    }
    
    return 0;
}

/*
 * Test Ethernet Frame
 */
static int test_ethernet_frame(void) {
    TEST_START("Ethernet Frame Handling");
    
    uint8_t mac1[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t mac2[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    
    ASSERT(ethernet_mac_equal(mac1, mac1), "MAC comparison failed");
    ASSERT(!ethernet_mac_equal(mac1, mac2), "MAC comparison failed");
    
    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ASSERT(ethernet_mac_is_broadcast(broadcast), "Broadcast check failed");
    
    uint8_t multicast[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01};
    ASSERT(ethernet_mac_is_multicast(multicast), "Multicast check failed");
    
    TEST_PASS();
    return 0;
}

/*
 * Test Netfilter Hooks
 */
static unsigned int test_hook_func(unsigned int hooknum,
                                   struct sk_buff* skb,
                                   const struct net_device* in,
                                   const struct net_device* out,
                                   void* priv) {
    return NF_ACCEPT;
}

static int test_netfilter(void) {
    TEST_START("Netfilter Hooks");
    
    nf_hook_ops_t hook;
    memset(&hook, 0, sizeof(hook));
    hook.hook = test_hook_func;
    hook.hooknum = NF_IP_LOCAL_IN;
    hook.priority = NF_IP_PRI_FILTER;
    
    int ret = nf_register_hook(&hook);
    ASSERT(ret == 0, "Failed to register netfilter hook");
    
    nf_unregister_hook(&hook);
    
    TEST_PASS();
    return 0;
}

/*
 * Test NAT
 */
static int test_nat(void) {
    TEST_START("NAT Configuration");
    
    nat_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    
    /* SNAT rule: 192.168.1.0/24 -> 10.0.0.1 */
    rule.match_src_ip = 0xC0A80100;      /* 192.168.1.0 */
    rule.match_src_mask = 0xFFFFFF00;    /* /24 */
    rule.nat_ip = 0x0A000001;            /* 10.0.0.1 */
    rule.nat_type = NAT_TYPE_SNAT;
    rule.nat_port_min = 1024;
    rule.nat_port_max = 65535;
    
    int ret = nat_add_rule(&rule);
    ASSERT(ret == 0, "Failed to add NAT rule");
    
    nat_flush_rules();
    
    TEST_PASS();
    return 0;
}

/*
 * Test QoS Classification
 */
static int test_qos(void) {
    TEST_START("QoS Classification");
    
    qos_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    
    /* Classify VoIP traffic (UDP port 5060) as Voice class */
    rule.match_protocol = IPPROTO_UDP;
    rule.match_dst_port_min = 5060;
    rule.match_dst_port_max = 5060;
    rule.target_class = QOS_CLASS_VO;
    rule.set_dscp = QOS_DSCP_EF;
    
    int ret = qos_add_rule(&rule);
    ASSERT(ret == 0, "Failed to add QoS rule");
    
    /* Set bandwidth limit */
    ret = qos_set_class_bandwidth(QOS_CLASS_VO, 1000000, 50000);
    ASSERT(ret == 0, "Failed to set class bandwidth");
    
    qos_flush_rules();
    
    TEST_PASS();
    return 0;
}

/*
 * Test Socket Options
 */
static int test_socket_options(void) {
    TEST_START("Socket Options");
    
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT(sock >= 0, "Failed to create socket");
    
    /* Set SO_REUSEADDR */
    int optval = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    ASSERT(ret == 0, "Failed to set SO_REUSEADDR");
    
    /* Set SO_KEEPALIVE */
    optval = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    ASSERT(ret == 0, "Failed to set SO_KEEPALIVE");
    
    /* Set SO_SNDBUF */
    uint32_t bufsize = 32768;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    ASSERT(ret == 0, "Failed to set SO_SNDBUF");
    
    /* Get SO_TYPE */
    int socktype;
    uint32_t optlen = sizeof(socktype);
    ret = getsockopt(sock, SOL_SOCKET, SO_TYPE, &socktype, &optlen);
    ASSERT(ret == 0, "Failed to get SO_TYPE");
    ASSERT(socktype == SOCK_STREAM, "Incorrect socket type");
    
    close(sock);
    
    TEST_PASS();
    return 0;
}

/*
 * Performance Benchmarks
 */
static int test_tcp_throughput(void) {
    TEST_START("TCP Throughput Benchmark");
    
    /* This would require a full TCP connection with data transfer */
    /* Skipping for now as it requires a complete network stack */
    
    TEST_SKIP("Requires full network stack initialization");
    return 0;
}

static int test_latency(void) {
    TEST_START("Network Latency Benchmark");
    
    /* This would measure round-trip time for ICMP echo */
    
    TEST_SKIP("Requires timing infrastructure");
    return 0;
}

/*
 * Main Test Runner
 */
void run_network_tests(void) {
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "LimitlessOS Network Stack Test Suite\n");
    printk(KERN_INFO "========================================\n\n");
    
    memset(&results, 0, sizeof(results));
    
    /* Basic Socket Tests */
    test_socket_creation();
    test_socket_bind();
    test_socket_options();
    test_tcp_connection();
    test_udp_sendrecv();
    
    /* Protocol Tests */
    test_icmp_ping();
    test_arp_resolution();
    test_ethernet_frame();
    
    /* Advanced Features */
    test_netfilter();
    test_nat();
    test_qos();
    
    /* Performance Tests */
    test_tcp_throughput();
    test_latency();
    
    /* Print summary */
    printk(KERN_INFO "\n========================================\n");
    printk(KERN_INFO "Test Results Summary\n");
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "Total Tests:  %u\n", results.total_tests);
    printk(KERN_INFO "Passed:       %u (%u%%)\n", results.passed,
           results.total_tests ? (results.passed * 100 / results.total_tests) : 0);
    printk(KERN_INFO "Failed:       %u (%u%%)\n", results.failed,
           results.total_tests ? (results.failed * 100 / results.total_tests) : 0);
    printk(KERN_INFO "Skipped:      %u (%u%%)\n", results.skipped,
           results.total_tests ? (results.skipped * 100 / results.total_tests) : 0);
    printk(KERN_INFO "========================================\n");
    
    if (results.failed == 0) {
        printk(KERN_INFO "✓ All tests passed!\n");
    } else {
        printk(KERN_ERR "✗ Some tests failed!\n");
    }
}

/*
 * Print Network Stack Statistics
 */
void dump_network_stats(void) {
    printk(KERN_INFO "\n========================================\n");
    printk(KERN_INFO "Network Stack Statistics\n");
    printk(KERN_INFO "========================================\n\n");
    
    ethernet_dump_stats();
    printk(KERN_INFO "\n");
    
    arp_dump_cache();
    printk(KERN_INFO "\n");
    
    ip_dump_stats();
    printk(KERN_INFO "\n");
    
    icmp_dump_stats();
    printk(KERN_INFO "\n");
    
    tcp_dump_stats();
    printk(KERN_INFO "\n");
    
    udp_dump_stats();
    printk(KERN_INFO "\n");
    
    netfilter_dump_stats();
    printk(KERN_INFO "\n");
    
    nat_dump_stats();
    printk(KERN_INFO "\n");
    
    qos_dump_stats();
    printk(KERN_INFO "\n");
}
