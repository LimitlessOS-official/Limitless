/*
 * storage_benchmark.c - LimitlessOS Advanced Storage Benchmarking
 * 
 * Comprehensive storage performance testing and analysis tool
 * with support for various I/O patterns and enterprise metrics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <math.h>

#define MAX_DEVICES         16
#define MAX_THREADS         64
#define MAX_BLOCK_SIZES     16
#define DEFAULT_TEST_SIZE   (1024 * 1024 * 1024)  /* 1GB */
#define DEFAULT_BLOCK_SIZE  4096
#define MIN_BLOCK_SIZE      512
#define MAX_BLOCK_SIZE      (1024 * 1024)         /* 1MB */
#define WARMUP_DURATION     5                      /* 5 seconds */
#define REPORT_INTERVAL     5                      /* 5 seconds */

/* Test types */
typedef enum {
    TEST_SEQUENTIAL_READ,
    TEST_SEQUENTIAL_WRITE,
    TEST_RANDOM_READ,
    TEST_RANDOM_WRITE,
    TEST_MIXED_RANDOM,
    TEST_QUEUE_DEPTH,
    TEST_LATENCY,
    TEST_ENDURANCE
} test_type_t;

/* I/O pattern */
typedef enum {
    IO_PATTERN_SEQUENTIAL,
    IO_PATTERN_RANDOM,
    IO_PATTERN_MIXED
} io_pattern_t;

/* Benchmark configuration */
typedef struct benchmark_config {
    char device_path[256];          /* Device to test */
    test_type_t test_type;          /* Type of test */
    io_pattern_t io_pattern;        /* I/O pattern */
    
    /* Test parameters */
    uint64_t test_size;             /* Total test size in bytes */
    uint32_t block_size;            /* Block size for I/O */
    uint32_t queue_depth;           /* Queue depth for async I/O */
    uint32_t thread_count;          /* Number of threads */
    uint32_t duration;              /* Test duration in seconds */
    uint32_t warmup_time;           /* Warmup time in seconds */
    
    /* I/O patterns */
    uint32_t read_percentage;       /* Read percentage for mixed workload */
    uint32_t random_percentage;     /* Random I/O percentage */
    
    /* Advanced options */
    bool direct_io;                 /* Use O_DIRECT */
    bool sync_io;                   /* Use O_SYNC */
    bool verify_data;               /* Verify written data */
    bool continuous;                /* Continuous benchmarking */
    
} benchmark_config_t;

/* Performance metrics */
typedef struct perf_metrics {
    /* Throughput */
    double read_mbps;               /* Read throughput (MB/s) */
    double write_mbps;              /* Write throughput (MB/s) */
    double total_mbps;              /* Total throughput (MB/s) */
    
    /* IOPS */
    double read_iops;               /* Read IOPS */
    double write_iops;              /* Write IOPS */
    double total_iops;              /* Total IOPS */
    
    /* Latency (microseconds) */
    double avg_latency;             /* Average latency */
    double min_latency;             /* Minimum latency */
    double max_latency;             /* Maximum latency */
    double p50_latency;             /* 50th percentile */
    double p95_latency;             /* 95th percentile */
    double p99_latency;             /* 99th percentile */
    
    /* Counts */
    uint64_t read_ops;              /* Total read operations */
    uint64_t write_ops;             /* Total write operations */
    uint64_t read_bytes;            /* Total bytes read */
    uint64_t write_bytes;           /* Total bytes written */
    uint64_t error_count;           /* Number of errors */
    
    /* Timing */
    double test_duration;           /* Actual test duration */
    
} perf_metrics_t;

/* Latency histogram */
typedef struct latency_histogram {
    uint32_t buckets[64];           /* Latency buckets (log scale) */
    uint32_t bucket_count;
    double min_latency;
    double max_latency;
} latency_histogram_t;

/* Thread data */
typedef struct thread_data {
    int thread_id;
    benchmark_config_t* config;
    perf_metrics_t metrics;
    latency_histogram_t histogram;
    
    /* Thread-specific state */
    int fd;                         /* File descriptor */
    uint8_t* buffer;                /* I/O buffer */
    uint64_t offset;                /* Current offset */
    uint64_t bytes_remaining;       /* Bytes remaining for this thread */
    
    /* Timing */
    struct timespec start_time;
    struct timespec end_time;
    
    /* Control */
    volatile bool running;
    volatile bool stop_requested;
    
} thread_data_t;

/* Global benchmark state */
typedef struct benchmark_state {
    benchmark_config_t config;
    thread_data_t threads[MAX_THREADS];
    perf_metrics_t total_metrics;
    latency_histogram_t total_histogram;
    
    /* Device information */
    uint64_t device_size;
    uint32_t logical_block_size;
    uint32_t physical_block_size;
    bool is_rotational;
    
    /* Control */
    volatile bool stop_all;
    struct timespec test_start;
    struct timespec test_end;
    
    /* Progress reporting */
    pthread_t report_thread;
    volatile bool report_running;
    
} benchmark_state_t;

static benchmark_state_t g_benchmark = {0};

/* Function prototypes */
static int benchmark_init(const char* device);
static int run_benchmark(benchmark_config_t* config);
static void* thread_worker(void* arg);
static void* progress_reporter(void* arg);
static int get_device_info(const char* device);
static void generate_test_data(uint8_t* buffer, size_t size, uint64_t offset);
static bool verify_test_data(const uint8_t* buffer, size_t size, uint64_t offset);
static double timespec_diff_ms(const struct timespec* start, const struct timespec* end);
static void update_latency_histogram(latency_histogram_t* hist, double latency);
static void merge_metrics(perf_metrics_t* total, const perf_metrics_t* thread_metrics);
static void print_results(const perf_metrics_t* metrics);
static void print_latency_distribution(const latency_histogram_t* hist);
static void signal_handler(int sig);

/* Initialize benchmarking */
int benchmark_init(const char* device) {
    printf("Initializing LimitlessOS Storage Benchmark\n");
    printf("Target device: %s\n", device);
    
    memset(&g_benchmark, 0, sizeof(benchmark_state_t));
    
    /* Get device information */
    if (get_device_info(device) != 0) {
        printf("Failed to get device information\n");
        return -1;
    }
    
    printf("Device size: %lu bytes (%.2f GB)\n", 
           g_benchmark.device_size, g_benchmark.device_size / (1024.0 * 1024.0 * 1024.0));
    printf("Logical block size: %u bytes\n", g_benchmark.logical_block_size);
    printf("Physical block size: %u bytes\n", g_benchmark.physical_block_size);
    printf("Rotational device: %s\n", g_benchmark.is_rotational ? "Yes" : "No");
    
    /* Setup signal handler for graceful shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Benchmark initialized\n");
    return 0;
}

/* Run sequential read test */
int benchmark_sequential_read(const char* device, uint64_t size, uint32_t block_size, uint32_t threads) {
    benchmark_config_t config = {0};
    
    strncpy(config.device_path, device, sizeof(config.device_path) - 1);
    config.test_type = TEST_SEQUENTIAL_READ;
    config.io_pattern = IO_PATTERN_SEQUENTIAL;
    config.test_size = size;
    config.block_size = block_size;
    config.thread_count = threads;
    config.queue_depth = 32;
    config.duration = 60;
    config.warmup_time = WARMUP_DURATION;
    config.direct_io = true;
    
    printf("\n=== Sequential Read Test ===\n");
    printf("Size: %lu bytes, Block size: %u bytes, Threads: %u\n",
           size, block_size, threads);
    
    return run_benchmark(&config);
}

/* Run sequential write test */
int benchmark_sequential_write(const char* device, uint64_t size, uint32_t block_size, uint32_t threads) {
    benchmark_config_t config = {0};
    
    strncpy(config.device_path, device, sizeof(config.device_path) - 1);
    config.test_type = TEST_SEQUENTIAL_WRITE;
    config.io_pattern = IO_PATTERN_SEQUENTIAL;
    config.test_size = size;
    config.block_size = block_size;
    config.thread_count = threads;
    config.queue_depth = 32;
    config.duration = 60;
    config.warmup_time = WARMUP_DURATION;
    config.direct_io = true;
    config.verify_data = true;
    
    printf("\n=== Sequential Write Test ===\n");
    printf("Size: %lu bytes, Block size: %u bytes, Threads: %u\n",
           size, block_size, threads);
    
    return run_benchmark(&config);
}

/* Run random I/O test */
int benchmark_random_io(const char* device, uint64_t size, uint32_t block_size, 
                       uint32_t threads, uint32_t read_pct) {
    benchmark_config_t config = {0};
    
    strncpy(config.device_path, device, sizeof(config.device_path) - 1);
    config.test_type = TEST_MIXED_RANDOM;
    config.io_pattern = IO_PATTERN_RANDOM;
    config.test_size = size;
    config.block_size = block_size;
    config.thread_count = threads;
    config.queue_depth = 32;
    config.duration = 60;
    config.warmup_time = WARMUP_DURATION;
    config.read_percentage = read_pct;
    config.direct_io = true;
    
    printf("\n=== Random I/O Test ===\n");
    printf("Size: %lu bytes, Block size: %u bytes, Threads: %u, Read: %u%%\n",
           size, block_size, threads, read_pct);
    
    return run_benchmark(&config);
}

/* Run latency test */
int benchmark_latency(const char* device, uint32_t block_size) {
    benchmark_config_t config = {0};
    
    strncpy(config.device_path, device, sizeof(config.device_path) - 1);
    config.test_type = TEST_LATENCY;
    config.io_pattern = IO_PATTERN_RANDOM;
    config.test_size = 1024 * 1024 * 100;  /* 100MB */
    config.block_size = block_size;
    config.thread_count = 1;               /* Single-threaded for latency */
    config.queue_depth = 1;                /* Queue depth 1 for latency */
    config.duration = 30;
    config.warmup_time = WARMUP_DURATION;
    config.read_percentage = 70;           /* 70% reads */
    config.direct_io = true;
    
    printf("\n=== Latency Test ===\n");
    printf("Block size: %u bytes, Single-threaded, QD=1\n", block_size);
    
    return run_benchmark(&config);
}

/* Run comprehensive benchmark suite */
int benchmark_comprehensive(const char* device) {
    printf("Starting comprehensive storage benchmark suite...\n");
    
    if (benchmark_init(device) != 0) {
        return -1;
    }
    
    uint32_t block_sizes[] = {4096, 8192, 16384, 65536, 262144};
    uint32_t num_block_sizes = sizeof(block_sizes) / sizeof(block_sizes[0]);
    
    uint64_t test_size = g_benchmark.device_size / 10;  /* Use 10% of device */
    if (test_size > DEFAULT_TEST_SIZE) {
        test_size = DEFAULT_TEST_SIZE;
    }
    
    /* Sequential read tests */
    for (uint32_t i = 0; i < num_block_sizes; i++) {
        benchmark_sequential_read(device, test_size, block_sizes[i], 1);
        if (g_benchmark.stop_all) break;
        
        benchmark_sequential_read(device, test_size, block_sizes[i], 4);
        if (g_benchmark.stop_all) break;
    }
    
    /* Sequential write tests */
    for (uint32_t i = 0; i < num_block_sizes; i++) {
        benchmark_sequential_write(device, test_size, block_sizes[i], 1);
        if (g_benchmark.stop_all) break;
        
        benchmark_sequential_write(device, test_size, block_sizes[i], 4);
        if (g_benchmark.stop_all) break;
    }
    
    /* Random I/O tests */
    uint32_t random_block_sizes[] = {4096, 8192};
    for (uint32_t i = 0; i < 2; i++) {
        benchmark_random_io(device, test_size, random_block_sizes[i], 4, 100);  /* Random read */
        if (g_benchmark.stop_all) break;
        
        benchmark_random_io(device, test_size, random_block_sizes[i], 4, 0);    /* Random write */
        if (g_benchmark.stop_all) break;
        
        benchmark_random_io(device, test_size, random_block_sizes[i], 4, 70);   /* Mixed 70/30 */
        if (g_benchmark.stop_all) break;
    }
    
    /* Latency tests */
    benchmark_latency(device, 4096);
    if (g_benchmark.stop_all) return 0;
    
    benchmark_latency(device, 8192);
    
    printf("\nComprehensive benchmark completed!\n");
    return 0;
}

/* Run benchmark */
static int run_benchmark(benchmark_config_t* config) {
    g_benchmark.config = *config;
    g_benchmark.stop_all = false;
    
    /* Open device */
    int flags = O_RDWR;
    if (config->direct_io) flags |= O_DIRECT;
    if (config->sync_io) flags |= O_SYNC;
    
    int test_fd = open(config->device_path, flags);
    if (test_fd < 0) {
        printf("Failed to open device: %s\n", config->device_path);
        return -1;
    }
    close(test_fd);
    
    /* Calculate per-thread work size */
    uint64_t work_per_thread = config->test_size / config->thread_count;
    
    printf("Starting test with %u threads, %u second duration\n", 
           config->thread_count, config->duration);
    
    /* Initialize threads */
    for (uint32_t i = 0; i < config->thread_count; i++) {
        thread_data_t* td = &g_benchmark.threads[i];
        memset(td, 0, sizeof(thread_data_t));
        
        td->thread_id = i;
        td->config = &g_benchmark.config;
        td->offset = i * work_per_thread;
        td->bytes_remaining = work_per_thread;
        td->running = false;
        td->stop_requested = false;
        
        /* Allocate aligned I/O buffer */
        if (posix_memalign((void**)&td->buffer, 4096, config->block_size) != 0) {
            printf("Failed to allocate I/O buffer for thread %u\n", i);
            return -1;
        }
    }
    
    /* Start progress reporter */
    g_benchmark.report_running = true;
    pthread_create(&g_benchmark.report_thread, NULL, progress_reporter, NULL);
    
    /* Record start time */
    clock_gettime(CLOCK_MONOTONIC, &g_benchmark.test_start);
    
    /* Start worker threads */
    pthread_t worker_threads[MAX_THREADS];
    for (uint32_t i = 0; i < config->thread_count; i++) {
        pthread_create(&worker_threads[i], NULL, thread_worker, &g_benchmark.threads[i]);
    }
    
    /* Wait for test completion or timeout */
    struct timespec timeout;
    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_sec += config->warmup_time + config->duration;
    
    for (uint32_t i = 0; i < config->thread_count; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    
    /* Stop progress reporter */
    g_benchmark.report_running = false;
    pthread_cancel(g_benchmark.report_thread);
    pthread_join(g_benchmark.report_thread, NULL);
    
    /* Record end time */
    clock_gettime(CLOCK_MONOTONIC, &g_benchmark.test_end);
    
    /* Calculate total metrics */
    memset(&g_benchmark.total_metrics, 0, sizeof(perf_metrics_t));
    g_benchmark.total_metrics.min_latency = 1e9;  /* Initialize to large value */
    
    for (uint32_t i = 0; i < config->thread_count; i++) {
        merge_metrics(&g_benchmark.total_metrics, &g_benchmark.threads[i].metrics);
        
        /* Cleanup */
        free(g_benchmark.threads[i].buffer);
    }
    
    /* Calculate final metrics */
    double total_time = timespec_diff_ms(&g_benchmark.test_start, &g_benchmark.test_end) / 1000.0;
    g_benchmark.total_metrics.test_duration = total_time;
    
    if (total_time > 0) {
        g_benchmark.total_metrics.read_mbps = (g_benchmark.total_metrics.read_bytes / (1024.0 * 1024.0)) / total_time;
        g_benchmark.total_metrics.write_mbps = (g_benchmark.total_metrics.write_bytes / (1024.0 * 1024.0)) / total_time;
        g_benchmark.total_metrics.total_mbps = g_benchmark.total_metrics.read_mbps + g_benchmark.total_metrics.write_mbps;
        
        g_benchmark.total_metrics.read_iops = g_benchmark.total_metrics.read_ops / total_time;
        g_benchmark.total_metrics.write_iops = g_benchmark.total_metrics.write_ops / total_time;
        g_benchmark.total_metrics.total_iops = g_benchmark.total_metrics.read_iops + g_benchmark.total_metrics.write_iops;
    }
    
    /* Print results */
    print_results(&g_benchmark.total_metrics);
    
    return 0;
}

/* Worker thread function */
static void* thread_worker(void* arg) {
    thread_data_t* td = (thread_data_t*)arg;
    benchmark_config_t* config = td->config;
    
    /* Open device for this thread */
    int flags = O_RDWR;
    if (config->direct_io) flags |= O_DIRECT;
    if (config->sync_io) flags |= O_SYNC;
    
    td->fd = open(config->device_path, flags);
    if (td->fd < 0) {
        printf("Thread %d: Failed to open device\n", td->thread_id);
        return NULL;
    }
    
    /* Initialize metrics */
    memset(&td->metrics, 0, sizeof(perf_metrics_t));
    td->metrics.min_latency = 1e9;
    
    /* Warmup period */
    clock_gettime(CLOCK_MONOTONIC, &td->start_time);
    struct timespec warmup_end = td->start_time;
    warmup_end.tv_sec += config->warmup_time;
    
    td->running = true;
    
    /* Warmup loop */
    while (!td->stop_requested && !g_benchmark.stop_all) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        if (now.tv_sec >= warmup_end.tv_sec) {
            break;
        }
        
        /* Perform I/O without recording metrics */
        bool is_read = (config->test_type == TEST_SEQUENTIAL_READ ||
                       config->test_type == TEST_RANDOM_READ ||
                       (config->test_type == TEST_MIXED_RANDOM && (rand() % 100) < config->read_percentage));
        
        uint64_t offset;
        if (config->io_pattern == IO_PATTERN_SEQUENTIAL) {
            offset = td->offset;
            td->offset += config->block_size;
            if (td->offset >= td->offset + td->bytes_remaining) {
                td->offset = td->thread_id * (td->bytes_remaining);
            }
        } else {
            offset = (rand() % (g_benchmark.device_size / config->block_size)) * config->block_size;
        }
        
        if (is_read) {
            pread(td->fd, td->buffer, config->block_size, offset);
        } else {
            generate_test_data(td->buffer, config->block_size, offset);
            pwrite(td->fd, td->buffer, config->block_size, offset);
        }
        
        usleep(1000);  /* Small delay to avoid overwhelming */
    }
    
    printf("Thread %d: Warmup complete, starting measurement\n", td->thread_id);
    
    /* Reset metrics for actual test */
    memset(&td->metrics, 0, sizeof(perf_metrics_t));
    td->metrics.min_latency = 1e9;
    
    /* Test period */
    clock_gettime(CLOCK_MONOTONIC, &td->start_time);
    struct timespec test_end = td->start_time;
    test_end.tv_sec += config->duration;
    
    /* Main test loop */
    while (!td->stop_requested && !g_benchmark.stop_all) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        if (now.tv_sec >= test_end.tv_sec) {
            break;
        }
        
        /* Determine I/O type */
        bool is_read = (config->test_type == TEST_SEQUENTIAL_READ ||
                       config->test_type == TEST_RANDOM_READ ||
                       (config->test_type == TEST_MIXED_RANDOM && (rand() % 100) < config->read_percentage));
        
        /* Calculate offset */
        uint64_t offset;
        if (config->io_pattern == IO_PATTERN_SEQUENTIAL) {
            offset = td->offset;
            td->offset += config->block_size;
            if (td->offset >= td->offset + td->bytes_remaining) {
                td->offset = td->thread_id * (td->bytes_remaining);
            }
        } else {
            offset = (rand() % (g_benchmark.device_size / config->block_size)) * config->block_size;
        }
        
        /* Record start time for latency */
        struct timespec io_start, io_end;
        clock_gettime(CLOCK_MONOTONIC, &io_start);
        
        /* Perform I/O */
        ssize_t result;
        if (is_read) {
            result = pread(td->fd, td->buffer, config->block_size, offset);
            if (result > 0) {
                td->metrics.read_ops++;
                td->metrics.read_bytes += result;
                
                /* Verify data if requested */
                if (config->verify_data && !verify_test_data(td->buffer, result, offset)) {
                    td->metrics.error_count++;
                }
            }
        } else {
            generate_test_data(td->buffer, config->block_size, offset);
            result = pwrite(td->fd, td->buffer, config->block_size, offset);
            if (result > 0) {
                td->metrics.write_ops++;
                td->metrics.write_bytes += result;
            }
        }
        
        /* Record end time and calculate latency */
        clock_gettime(CLOCK_MONOTONIC, &io_end);
        double latency = timespec_diff_ms(&io_start, &io_end) * 1000.0;  /* Convert to microseconds */
        
        /* Update latency statistics */
        if (latency < td->metrics.min_latency) td->metrics.min_latency = latency;
        if (latency > td->metrics.max_latency) td->metrics.max_latency = latency;
        
        uint64_t total_ops = td->metrics.read_ops + td->metrics.write_ops;
        td->metrics.avg_latency = ((td->metrics.avg_latency * (total_ops - 1)) + latency) / total_ops;
        
        update_latency_histogram(&td->histogram, latency);
        
        if (result < 0) {
            td->metrics.error_count++;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &td->end_time);
    td->running = false;
    
    close(td->fd);
    
    printf("Thread %d: Test complete\n", td->thread_id);
    return NULL;
}

/* Progress reporter thread */
static void* progress_reporter(void* arg) {
    (void)arg;
    
    while (g_benchmark.report_running) {
        sleep(REPORT_INTERVAL);
        
        if (!g_benchmark.report_running) break;
        
        /* Calculate current metrics */
        perf_metrics_t current = {0};
        current.min_latency = 1e9;
        
        for (uint32_t i = 0; i < g_benchmark.config.thread_count; i++) {
            if (g_benchmark.threads[i].running) {
                merge_metrics(&current, &g_benchmark.threads[i].metrics);
            }
        }
        
        /* Calculate rates */
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = timespec_diff_ms(&g_benchmark.test_start, &now) / 1000.0;
        
        if (elapsed > 0) {
            double read_mbps = (current.read_bytes / (1024.0 * 1024.0)) / elapsed;
            double write_mbps = (current.write_bytes / (1024.0 * 1024.0)) / elapsed;
            double read_iops = current.read_ops / elapsed;
            double write_iops = current.write_ops / elapsed;
            
            printf("Progress: %.1fs - Read: %.1f MB/s (%.0f IOPS), Write: %.1f MB/s (%.0f IOPS), Latency: %.1f μs\n",
                   elapsed, read_mbps, read_iops, write_mbps, write_iops, current.avg_latency);
        }
    }
    
    return NULL;
}

/* Utility functions */
static int get_device_info(const char* device) {
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    /* Get device size */
    if (ioctl(fd, BLKGETSIZE64, &g_benchmark.device_size) != 0) {
        close(fd);
        return -1;
    }
    
    /* Get block sizes */
    int logical_block_size, physical_block_size;
    if (ioctl(fd, BLKSSZGET, &logical_block_size) == 0) {
        g_benchmark.logical_block_size = logical_block_size;
    } else {
        g_benchmark.logical_block_size = 512;
    }
    
    if (ioctl(fd, BLKPBSZGET, &physical_block_size) == 0) {
        g_benchmark.physical_block_size = physical_block_size;
    } else {
        g_benchmark.physical_block_size = g_benchmark.logical_block_size;
    }
    
    /* Check if rotational */
    int rotational;
    if (ioctl(fd, BLKROTATIONAL, &rotational) == 0) {
        g_benchmark.is_rotational = (rotational != 0);
    } else {
        g_benchmark.is_rotational = true;  /* Assume rotational if unknown */
    }
    
    close(fd);
    return 0;
}

static void generate_test_data(uint8_t* buffer, size_t size, uint64_t offset) {
    /* Generate deterministic test pattern based on offset */
    uint32_t* words = (uint32_t*)buffer;
    size_t word_count = size / sizeof(uint32_t);
    
    uint32_t seed = (uint32_t)(offset / size);
    
    for (size_t i = 0; i < word_count; i++) {
        words[i] = seed + i;
    }
}

static bool verify_test_data(const uint8_t* buffer, size_t size, uint64_t offset) {
    /* Verify test pattern */
    const uint32_t* words = (const uint32_t*)buffer;
    size_t word_count = size / sizeof(uint32_t);
    
    uint32_t expected_seed = (uint32_t)(offset / size);
    
    for (size_t i = 0; i < word_count; i++) {
        if (words[i] != expected_seed + i) {
            return false;
        }
    }
    
    return true;
}

static double timespec_diff_ms(const struct timespec* start, const struct timespec* end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1e6;
}

static void update_latency_histogram(latency_histogram_t* hist, double latency) {
    /* Log-scale histogram buckets */
    int bucket = 0;
    if (latency > 1.0) {
        bucket = (int)(log2(latency)) + 1;
    }
    
    if (bucket >= 0 && bucket < 64) {
        hist->buckets[bucket]++;
        hist->bucket_count++;
        
        if (latency < hist->min_latency || hist->bucket_count == 1) {
            hist->min_latency = latency;
        }
        if (latency > hist->max_latency) {
            hist->max_latency = latency;
        }
    }
}

static void merge_metrics(perf_metrics_t* total, const perf_metrics_t* thread_metrics) {
    total->read_ops += thread_metrics->read_ops;
    total->write_ops += thread_metrics->write_ops;
    total->read_bytes += thread_metrics->read_bytes;
    total->write_bytes += thread_metrics->write_bytes;
    total->error_count += thread_metrics->error_count;
    
    if (thread_metrics->min_latency < total->min_latency) {
        total->min_latency = thread_metrics->min_latency;
    }
    if (thread_metrics->max_latency > total->max_latency) {
        total->max_latency = thread_metrics->max_latency;
    }
    
    /* Weighted average of latencies */
    uint64_t total_ops = total->read_ops + total->write_ops;
    uint64_t thread_ops = thread_metrics->read_ops + thread_metrics->write_ops;
    
    if (total_ops > 0) {
        total->avg_latency = ((total->avg_latency * (total_ops - thread_ops)) + 
                             (thread_metrics->avg_latency * thread_ops)) / total_ops;
    }
}

static void print_results(const perf_metrics_t* metrics) {
    printf("\n" "=" "=" "=" " Benchmark Results " "=" "=" "=" "\n");
    
    printf("Test Duration: %.2f seconds\n", metrics->test_duration);
    printf("Total Operations: %lu reads, %lu writes\n", metrics->read_ops, metrics->write_ops);
    printf("Total Data: %.2f MB read, %.2f MB written\n",
           metrics->read_bytes / (1024.0 * 1024.0),
           metrics->write_bytes / (1024.0 * 1024.0));
    
    printf("\nThroughput:\n");
    printf("  Read:  %.2f MB/s\n", metrics->read_mbps);
    printf("  Write: %.2f MB/s\n", metrics->write_mbps);
    printf("  Total: %.2f MB/s\n", metrics->total_mbps);
    
    printf("\nIOPS:\n");
    printf("  Read:  %.0f IOPS\n", metrics->read_iops);
    printf("  Write: %.0f IOPS\n", metrics->write_iops);
    printf("  Total: %.0f IOPS\n", metrics->total_iops);
    
    printf("\nLatency (microseconds):\n");
    printf("  Average: %.1f μs\n", metrics->avg_latency);
    printf("  Minimum: %.1f μs\n", metrics->min_latency);
    printf("  Maximum: %.1f μs\n", metrics->max_latency);
    
    if (metrics->error_count > 0) {
        printf("\nErrors: %lu\n", metrics->error_count);
    }
    
    printf("\n");
}

static void signal_handler(int sig) {
    (void)sig;
    printf("\nReceived signal, stopping benchmark...\n");
    g_benchmark.stop_all = true;
}

/* Main CLI interface */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <device> <command> [options]\n", argv[0]);
        printf("Commands:\n");
        printf("  seq-read [size] [block_size] [threads]   - Sequential read test\n");
        printf("  seq-write [size] [block_size] [threads]  - Sequential write test\n");
        printf("  random [size] [block_size] [threads] [read_pct] - Random I/O test\n");
        printf("  latency [block_size]                     - Latency test\n");
        printf("  comprehensive                            - Full benchmark suite\n");
        printf("\nExample:\n");
        printf("  %s /dev/nvme0n1 seq-read 1073741824 4096 4\n", argv[0]);
        printf("  %s /dev/sda comprehensive\n", argv[0]);
        return 1;
    }
    
    const char* device = argv[1];
    const char* command = argv[2];
    
    if (benchmark_init(device) != 0) {
        return 1;
    }
    
    if (strcmp(command, "seq-read") == 0) {
        uint64_t size = (argc > 3) ? strtoull(argv[3], NULL, 10) : DEFAULT_TEST_SIZE;
        uint32_t block_size = (argc > 4) ? atoi(argv[4]) : DEFAULT_BLOCK_SIZE;
        uint32_t threads = (argc > 5) ? atoi(argv[5]) : 1;
        
        return benchmark_sequential_read(device, size, block_size, threads);
        
    } else if (strcmp(command, "seq-write") == 0) {
        uint64_t size = (argc > 3) ? strtoull(argv[3], NULL, 10) : DEFAULT_TEST_SIZE;
        uint32_t block_size = (argc > 4) ? atoi(argv[4]) : DEFAULT_BLOCK_SIZE;
        uint32_t threads = (argc > 5) ? atoi(argv[5]) : 1;
        
        return benchmark_sequential_write(device, size, block_size, threads);
        
    } else if (strcmp(command, "random") == 0) {
        uint64_t size = (argc > 3) ? strtoull(argv[3], NULL, 10) : DEFAULT_TEST_SIZE;
        uint32_t block_size = (argc > 4) ? atoi(argv[4]) : DEFAULT_BLOCK_SIZE;
        uint32_t threads = (argc > 5) ? atoi(argv[5]) : 4;
        uint32_t read_pct = (argc > 6) ? atoi(argv[6]) : 70;
        
        return benchmark_random_io(device, size, block_size, threads, read_pct);
        
    } else if (strcmp(command, "latency") == 0) {
        uint32_t block_size = (argc > 3) ? atoi(argv[3]) : DEFAULT_BLOCK_SIZE;
        
        return benchmark_latency(device, block_size);
        
    } else if (strcmp(command, "comprehensive") == 0) {
        return benchmark_comprehensive(device);
        
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
}