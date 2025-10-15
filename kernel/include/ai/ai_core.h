/*
 * LimitlessOS AI & Machine Learning Integration
 * Native AI frameworks, GPU computing, and intelligent system optimization
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/* AI Framework Types */
typedef enum {
    AI_FRAMEWORK_TENSORFLOW = 0,
    AI_FRAMEWORK_PYTORCH,
    AI_FRAMEWORK_ONNX,
    AI_FRAMEWORK_TENSORRT,
    AI_FRAMEWORK_OPENVINO,
    AI_FRAMEWORK_NCNN,
    AI_FRAMEWORK_MNN,
    AI_FRAMEWORK_TFLITE,
    AI_FRAMEWORK_MAX
} ai_framework_t;

/* GPU Computing APIs */
typedef enum {
    GPU_API_CUDA = 0,
    GPU_API_OPENCL,
    GPU_API_VULKAN_COMPUTE,
    GPU_API_METAL,
    GPU_API_ROCM,
    GPU_API_ONEAPI,
    GPU_API_MAX
} gpu_api_t;

/* Model Types */
typedef enum {
    MODEL_TYPE_CLASSIFICATION = 0,
    MODEL_TYPE_DETECTION,
    MODEL_TYPE_SEGMENTATION,
    MODEL_TYPE_NLP,
    MODEL_TYPE_SPEECH,
    MODEL_TYPE_RECOMMENDATION,
    MODEL_TYPE_REINFORCEMENT,
    MODEL_TYPE_GENERATIVE,
    MODEL_TYPE_CUSTOM,
    MODEL_TYPE_MAX
} model_type_t;

/* Data Types */
typedef enum {
    DATA_TYPE_FLOAT32 = 0,
    DATA_TYPE_FLOAT16,
    DATA_TYPE_INT32,
    DATA_TYPE_INT16,
    DATA_TYPE_INT8,
    DATA_TYPE_UINT8,
    DATA_TYPE_BOOL,
    DATA_TYPE_MAX
} ai_data_type_t;

/* Device Types */
typedef enum {
    AI_DEVICE_CPU = 0,
    AI_DEVICE_GPU,
    AI_DEVICE_TPU,
    AI_DEVICE_NPU,
    AI_DEVICE_VPU,
    AI_DEVICE_FPGA,
    AI_DEVICE_MAX
} ai_device_type_t;

/* Tensor shape */
typedef struct tensor_shape {
    int dimensions[8];           /* Tensor dimensions */
    int rank;                   /* Number of dimensions */
} tensor_shape_t;

/* Tensor descriptor */
typedef struct tensor {
    char name[128];             /* Tensor name */
    tensor_shape_t shape;       /* Tensor shape */
    ai_data_type_t data_type;   /* Data type */
    void *data;                 /* Tensor data */
    size_t size;                /* Data size in bytes */
    bool on_device;             /* Data on GPU/accelerator */
    void *device_ptr;           /* Device memory pointer */
} tensor_t;

/* Model metadata */
typedef struct ai_model {
    char name[128];             /* Model name */
    char path[512];             /* Model file path */
    char version[32];           /* Model version */
    model_type_t type;          /* Model type */
    ai_framework_t framework;   /* AI framework */
    
    /* Model architecture */
    struct {
        char architecture[64];  /* Model architecture (ResNet, BERT, etc.) */
        uint64_t parameters;    /* Number of parameters */
        uint64_t flops;         /* FLOPs estimate */
        size_t memory_usage;    /* Memory usage estimate */
    } architecture;
    
    /* Input/Output specifications */
    struct {
        tensor_t inputs[16];    /* Input tensors */
        tensor_t outputs[16];   /* Output tensors */
        int input_count;        /* Number of inputs */
        int output_count;       /* Number of outputs */
    } io_spec;
    
    /* Performance characteristics */
    struct {
        float latency_ms;       /* Inference latency */
        float throughput_fps;   /* Throughput (frames/sec) */
        size_t memory_footprint; /* Memory footprint */
        float accuracy;         /* Model accuracy */
    } performance;
    
    /* Runtime information */
    struct {
        bool loaded;            /* Model is loaded */
        ai_device_type_t device; /* Target device */
        void *runtime_handle;   /* Framework-specific handle */
        uint64_t load_time;     /* Load timestamp */
        uint64_t inference_count; /* Number of inferences */
    } runtime;
    
    /* Preprocessing/Postprocessing */
    struct {
        struct {
            bool normalize;     /* Normalize inputs */
            float mean[3];      /* Mean values (RGB) */
            float std[3];       /* Standard deviation */
            bool resize;        /* Resize inputs */
            int target_width;   /* Target width */
            int target_height;  /* Target height */
        } preprocessing;
        
        struct {
            bool softmax;       /* Apply softmax */
            bool argmax;        /* Apply argmax */
            float threshold;    /* Classification threshold */
        } postprocessing;
    } processing;
    
} ai_model_t;

/* GPU Device Information */
typedef struct gpu_device {
    char name[128];             /* GPU name */
    char vendor[64];            /* Vendor (NVIDIA, AMD, Intel) */
    char driver_version[32];    /* Driver version */
    
    /* Hardware specifications */
    struct {
        uint32_t compute_units;  /* Compute units/SMs */
        uint32_t cores;         /* CUDA cores/Stream processors */
        uint32_t tensor_cores;  /* Tensor cores */
        uint32_t rt_cores;      /* RT cores */
        uint64_t memory_size;   /* VRAM size */
        uint32_t memory_bandwidth; /* Memory bandwidth GB/s */
        uint32_t base_clock;    /* Base clock MHz */
        uint32_t boost_clock;   /* Boost clock MHz */
    } specs;
    
    /* Compute capabilities */
    struct {
        float compute_capability; /* CUDA compute capability */
        bool fp16_support;      /* Half precision support */
        bool int8_support;      /* INT8 support */
        bool tensor_ops;        /* Tensor operations */
        bool mixed_precision;   /* Mixed precision support */
    } capabilities;
    
    /* Current state */
    struct {
        bool available;         /* Device available */
        float utilization;      /* GPU utilization % */
        uint64_t memory_used;   /* Used VRAM */
        uint64_t memory_free;   /* Free VRAM */
        uint32_t temperature;   /* Temperature °C */
        uint32_t power_usage;   /* Power usage W */
    } state;
    
    /* Supported APIs */
    struct {
        bool cuda;              /* CUDA support */
        bool opencl;            /* OpenCL support */
        bool vulkan;            /* Vulkan support */
        bool directx;           /* DirectX support */
        char cuda_version[16];  /* CUDA version */
        char opencl_version[16]; /* OpenCL version */
    } apis;
    
} gpu_device_t;

/* AI Workload */
typedef struct ai_workload {
    char id[64];                /* Workload ID */
    char name[128];             /* Workload name */
    
    /* Workload specification */
    struct {
        ai_model_t *model;      /* Associated model */
        ai_device_type_t device; /* Target device */
        int priority;           /* Priority (0-100) */
        bool realtime;          /* Real-time requirement */
        float max_latency_ms;   /* Maximum latency */
        float min_throughput;   /* Minimum throughput */
    } spec;
    
    /* Resource allocation */
    struct {
        uint32_t gpu_memory_mb; /* GPU memory allocation */
        uint32_t cpu_cores;     /* CPU cores allocation */
        uint32_t ram_mb;        /* RAM allocation */
        float gpu_utilization;  /* Target GPU utilization */
    } resources;
    
    /* Execution state */
    struct {
        enum {
            WORKLOAD_IDLE = 0,
            WORKLOAD_QUEUED,
            WORKLOAD_RUNNING,
            WORKLOAD_COMPLETED,
            WORKLOAD_ERROR
        } state;
        
        uint64_t start_time;    /* Start timestamp */
        uint64_t end_time;      /* End timestamp */
        uint64_t queue_time;    /* Queue time */
        uint64_t execution_time; /* Execution time */
        
        float actual_latency;   /* Actual latency */
        float actual_throughput; /* Actual throughput */
        
        char error_message[256]; /* Error message */
        
    } execution;
    
    /* Statistics */
    struct {
        uint64_t total_inferences;
        uint64_t successful_inferences;
        uint64_t failed_inferences;
        float average_latency;
        float average_throughput;
        uint64_t total_gpu_time;
        uint64_t total_cpu_time;
    } stats;
    
} ai_workload_t;

/* Neural Network Accelerator */
typedef struct npu_device {
    char name[128];             /* NPU name */
    char vendor[64];            /* Vendor */
    char arch[64];              /* Architecture */
    
    /* Specifications */
    struct {
        uint32_t tops_int8;     /* INT8 TOPS */
        uint32_t tops_fp16;     /* FP16 TOPS */
        uint32_t memory_size;   /* On-chip memory */
        uint32_t frequency;     /* Operating frequency */
        uint32_t power_consumption; /* Power consumption */
    } specs;
    
    /* Supported operations */
    struct {
        bool convolution;       /* Convolution layers */
        bool pooling;          /* Pooling layers */
        bool activation;       /* Activation functions */
        bool batch_norm;       /* Batch normalization */
        bool attention;        /* Attention mechanisms */
        bool lstm;             /* LSTM layers */
        bool transformer;      /* Transformer blocks */
    } operations;
    
    /* Runtime state */
    struct {
        bool available;        /* Device available */
        float utilization;     /* Utilization percentage */
        uint32_t temperature;  /* Temperature */
        uint32_t active_models; /* Number of active models */
    } state;
    
} npu_device_t;

/* System Intelligence Agent */
typedef struct system_ai_agent {
    char name[128];             /* Agent name */
    bool enabled;               /* Agent enabled */
    
    /* Learning models */
    struct {
        ai_model_t *workload_predictor; /* Workload prediction model */
        ai_model_t *resource_optimizer; /* Resource optimization model */
        ai_model_t *anomaly_detector;   /* Anomaly detection model */
        ai_model_t *performance_tuner;  /* Performance tuning model */
    } models;
    
    /* Collected metrics */
    struct {
        struct {
            float cpu_usage[256];    /* CPU usage history */
            float memory_usage[256]; /* Memory usage history */
            float gpu_usage[256];    /* GPU usage history */
            float network_usage[256]; /* Network usage history */
            int history_size;        /* History buffer size */
            int history_index;       /* Current index */
        } metrics;
        
        struct {
            char process_names[64][128]; /* Process names */
            float process_cpu[64];       /* Process CPU usage */
            float process_memory[64];    /* Process memory usage */
            int process_count;           /* Number of processes */
        } processes;
        
        struct {
            float inference_latency[128]; /* Inference latency history */
            float model_accuracy[128];    /* Model accuracy history */
            float resource_efficiency[128]; /* Resource efficiency */
            int ai_history_size;         /* AI metrics history size */
        } ai_metrics;
        
    } metrics;
    
    /* Optimization decisions */
    struct {
        struct {
            bool frequency_scaling;  /* CPU frequency scaling */
            bool core_parking;       /* CPU core parking */
            bool memory_compression; /* Memory compression */
            bool gpu_boost;         /* GPU boost */
        } current_optimizations;
        
        struct {
            uint64_t decisions_made; /* Number of decisions */
            uint64_t optimizations_applied; /* Applied optimizations */
            float performance_improvement; /* Performance gain */
            float power_savings;     /* Power savings */
        } effectiveness;
        
    } optimization;
    
} system_ai_agent_t;

/* AI & ML Subsystem */
typedef struct ai_ml_system {
    bool initialized;
    
    /* Frameworks */
    struct {
        struct {
            ai_framework_t type;
            char name[64];
            char version[32];
            bool available;
            bool initialized;
            void *handle;           /* Library handle */
        } frameworks[AI_FRAMEWORK_MAX];
        int framework_count;
    } frameworks;
    
    /* Compute devices */
    struct {
        gpu_device_t gpus[16];
        npu_device_t npus[8];
        int gpu_count;
        int npu_count;
        
        /* Device selection policy */
        enum {
            DEVICE_POLICY_AUTO = 0,
            DEVICE_POLICY_CPU_ONLY,
            DEVICE_POLICY_GPU_PREFERRED,
            DEVICE_POLICY_NPU_PREFERRED,
            DEVICE_POLICY_LOAD_BALANCE
        } device_policy;
        
    } devices;
    
    /* Model management */
    struct {
        ai_model_t models[128];
        int model_count;
        
        /* Model registry */
        struct {
            char model_store_path[512];
            char cache_path[512];
            uint64_t cache_size_limit;
            bool auto_download;
        } registry;
        
    } models;
    
    /* Workload management */
    struct {
        ai_workload_t workloads[256];
        int workload_count;
        
        /* Scheduler */
        struct {
            enum {
                SCHEDULER_FIFO = 0,
                SCHEDULER_PRIORITY,
                SCHEDULER_FAIR_SHARE,
                SCHEDULER_DEADLINE,
                SCHEDULER_AI_OPTIMIZED
            } policy;
            
            bool preemption_enabled;
            int quantum_ms;
            
        } scheduler;
        
        /* Queue management */
        struct {
            ai_workload_t *queue[512];
            int queue_size;
            int queue_head;
            int queue_tail;
        } queue;
        
    } workloads;
    
    /* System intelligence */
    system_ai_agent_t intelligence_agent;
    
    /* Performance monitoring */
    struct {
        struct {
            uint64_t total_inferences;
            uint64_t total_gpu_time_ms;
            uint64_t total_cpu_time_ms;
            float average_latency_ms;
            float average_throughput;
        } global_stats;
        
        struct {
            float cpu_ai_utilization;
            float gpu_ai_utilization;
            float memory_ai_usage;
            uint32_t active_models;
            uint32_t queued_workloads;
        } current_state;
        
    } monitoring;
    
    /* Configuration */
    struct {
        bool auto_optimization;     /* Automatic optimization */
        bool telemetry_enabled;     /* Performance telemetry */
        bool model_caching;         /* Model caching */
        bool mixed_precision;       /* Mixed precision training */
        int max_concurrent_inferences; /* Max concurrent inferences */
        uint64_t memory_pool_size;  /* Memory pool size */
    } config;
    
} ai_ml_system_t;

/* Global AI/ML system instance */
extern ai_ml_system_t ai_system;

/* Core AI/ML functions */
int ai_ml_init(void);
void ai_ml_exit(void);
int ai_detect_devices(void);
int ai_enumerate_frameworks(void);

/* Framework management */
int ai_framework_init(ai_framework_t framework);
int ai_framework_load(const char *library_path);
bool ai_framework_available(ai_framework_t framework);
const char *ai_framework_name(ai_framework_t framework);
const char *ai_framework_version(ai_framework_t framework);

/* Model management */
int ai_model_load(const char *model_path, ai_framework_t framework, ai_model_t *model);
int ai_model_unload(ai_model_t *model);
int ai_model_optimize(ai_model_t *model, ai_device_type_t device);
int ai_model_quantize(ai_model_t *model, ai_data_type_t target_type);
int ai_model_validate(const ai_model_t *model);
int ai_model_get_info(const char *model_path, ai_model_t *info);
ai_model_t *ai_model_find(const char *name);

/* Model inference */
int ai_inference_sync(ai_model_t *model, const tensor_t *inputs, tensor_t *outputs);
int ai_inference_async(ai_model_t *model, const tensor_t *inputs, tensor_t *outputs, void (*callback)(void *), void *user_data);
int ai_inference_batch(ai_model_t *model, const tensor_t *batch_inputs, tensor_t *batch_outputs, int batch_size);
int ai_inference_stream(ai_model_t *model, const tensor_t *inputs, tensor_t *outputs);

/* Tensor operations */
int tensor_create(tensor_t *tensor, const tensor_shape_t *shape, ai_data_type_t type);
int tensor_destroy(tensor_t *tensor);
int tensor_copy_to_device(tensor_t *tensor, ai_device_type_t device);
int tensor_copy_from_device(tensor_t *tensor);
int tensor_reshape(tensor_t *tensor, const tensor_shape_t *new_shape);
int tensor_convert_type(tensor_t *tensor, ai_data_type_t new_type);
size_t tensor_size_bytes(const tensor_shape_t *shape, ai_data_type_t type);

/* GPU computing */
int gpu_init(void);
int gpu_enumerate_devices(gpu_device_t *devices, int max_devices);
gpu_device_t *gpu_get_device(int device_id);
int gpu_allocate_memory(int device_id, size_t size, void **ptr);
int gpu_free_memory(int device_id, void *ptr);
int gpu_copy_to_device(int device_id, const void *host_ptr, void *device_ptr, size_t size);
int gpu_copy_from_device(int device_id, const void *device_ptr, void *host_ptr, size_t size);
int gpu_synchronize(int device_id);
int gpu_get_utilization(int device_id, float *utilization);
int gpu_get_memory_info(int device_id, uint64_t *total, uint64_t *free, uint64_t *used);

/* CUDA specific functions */
#ifdef CUDA_AVAILABLE
int cuda_init(void);
int cuda_create_context(int device_id);
int cuda_destroy_context(int device_id);
int cuda_launch_kernel(int device_id, const char *kernel_name, void *params);
int cuda_compile_ptx(const char *ptx_code, void **module);
#endif

/* OpenCL specific functions */
#ifdef OPENCL_AVAILABLE
int opencl_init(void);
int opencl_create_context(int device_id);
int opencl_compile_program(const char *source_code, void **program);
int opencl_execute_kernel(void *program, const char *kernel_name, void *params);
#endif

/* Workload management */
int ai_workload_create(const char *name, ai_model_t *model, ai_workload_t *workload);
int ai_workload_submit(ai_workload_t *workload);
int ai_workload_cancel(const char *workload_id);
int ai_workload_wait(const char *workload_id);
int ai_workload_get_status(const char *workload_id, void *status);
ai_workload_t *ai_workload_find(const char *workload_id);

/* Scheduler functions */
int ai_scheduler_init(void);
int ai_scheduler_set_policy(int policy);
int ai_scheduler_run(void);
ai_workload_t *ai_scheduler_next_workload(void);
int ai_scheduler_add_workload(ai_workload_t *workload);
int ai_scheduler_remove_workload(const char *workload_id);

/* System intelligence */
int system_ai_init(void);
int system_ai_collect_metrics(void);
int system_ai_analyze_performance(void);
int system_ai_optimize_system(void);
int system_ai_predict_workload(void);
int system_ai_detect_anomalies(void);
float system_ai_get_cpu_prediction(int seconds_ahead);
float system_ai_get_memory_prediction(int seconds_ahead);
int system_ai_recommend_optimizations(void);

/* Model optimization */
int ai_model_profile(ai_model_t *model, ai_device_type_t device);
int ai_model_benchmark(ai_model_t *model, int iterations);
int ai_model_auto_tune(ai_model_t *model, ai_device_type_t device);
int ai_model_compress(ai_model_t *model, float compression_ratio);
int ai_model_prune(ai_model_t *model, float pruning_ratio);

/* Training support */
int ai_training_init(void);
int ai_training_create_session(const char *config_path);
int ai_training_load_dataset(const char *dataset_path);
int ai_training_start(const char *session_id);
int ai_training_pause(const char *session_id);
int ai_training_resume(const char *session_id);
int ai_training_stop(const char *session_id);
int ai_training_get_progress(const char *session_id, float *progress);

/* Model serving */
int ai_serving_start_server(int port, const char *model_path);
int ai_serving_stop_server(int port);
int ai_serving_register_model(const char *model_name, const char *model_path);
int ai_serving_unregister_model(const char *model_name);
int ai_serving_get_model_list(char models[][128], int max_models);

/* Distributed inference */
int ai_distributed_init(const char *cluster_config);
int ai_distributed_register_node(const char *node_address);
int ai_distributed_submit_job(const char *job_config);
int ai_distributed_get_job_status(const char *job_id);
int ai_distributed_cancel_job(const char *job_id);

/* Performance profiling */
int ai_profiler_start(void);
int ai_profiler_stop(void);
int ai_profiler_get_report(char *report, size_t size);
int ai_profiler_trace_inference(ai_model_t *model, const char *trace_file);

/* Utility functions */
const char *ai_device_type_name(ai_device_type_t device);
const char *ai_data_type_name(ai_data_type_t type);
const char *model_type_name(model_type_t type);
size_t ai_data_type_size(ai_data_type_t type);
int ai_get_optimal_device(const ai_model_t *model);
bool ai_device_supports_type(ai_device_type_t device, ai_data_type_t type);
float ai_estimate_inference_time(const ai_model_t *model, ai_device_type_t device);
uint64_t ai_estimate_memory_usage(const ai_model_t *model, ai_device_type_t device);