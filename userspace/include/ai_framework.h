/**
 * LimitlessOS AI/ML/Quantum Computing Framework
 * Advanced artificial intelligence with privacy-preserving features and quantum computing support
 */

#ifndef LIMITLESS_AI_FRAMEWORK_H
#define LIMITLESS_AI_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "limitless_types.h"

/* AI Framework Version */
#define LIMITLESS_AI_VERSION_MAJOR 2
#define LIMITLESS_AI_VERSION_MINOR 0

/* Maximum limits */
#define MAX_AI_MODELS               1024
#define MAX_NEURAL_NETWORKS         512
#define MAX_ML_DATASETS             256
#define MAX_AI_ACCELERATORS         64
#define MAX_QUANTUM_CIRCUITS        128
#define MAX_AI_SESSIONS             256
#define MAX_INFERENCE_CONTEXTS      1024
#define MAX_TRAINING_JOBS           64

/* AI Model Types */
typedef enum {
    AI_MODEL_UNKNOWN,
    /* Classical machine learning */
    AI_MODEL_LINEAR_REGRESSION,
    AI_MODEL_LOGISTIC_REGRESSION,
    AI_MODEL_DECISION_TREE,
    AI_MODEL_RANDOM_FOREST,
    AI_MODEL_GRADIENT_BOOSTING,
    AI_MODEL_SVM,                   /* Support Vector Machine */
    AI_MODEL_K_MEANS,
    AI_MODEL_K_NN,                  /* K-Nearest Neighbors */
    AI_MODEL_NAIVE_BAYES,
    AI_MODEL_PCA,                   /* Principal Component Analysis */
    AI_MODEL_ICA,                   /* Independent Component Analysis */
    /* Deep learning */
    AI_MODEL_FEEDFORWARD_NN,        /* Feedforward Neural Network */
    AI_MODEL_CONVOLUTIONAL_NN,      /* CNN */
    AI_MODEL_RECURRENT_NN,          /* RNN */
    AI_MODEL_LSTM,                  /* Long Short-Term Memory */
    AI_MODEL_GRU,                   /* Gated Recurrent Unit */
    AI_MODEL_TRANSFORMER,           /* Transformer architecture */
    AI_MODEL_BERT,                  /* BERT language model */
    AI_MODEL_GPT,                   /* GPT language model */
    AI_MODEL_VISION_TRANSFORMER,    /* Vision Transformer */
    AI_MODEL_AUTOENCODER,
    AI_MODEL_VAE,                   /* Variational Autoencoder */
    AI_MODEL_GAN,                   /* Generative Adversarial Network */
    AI_MODEL_DIFFUSION,             /* Diffusion models */
    AI_MODEL_NERF,                  /* Neural Radiance Fields */
    /* Reinforcement learning */
    AI_MODEL_DQN,                   /* Deep Q-Network */
    AI_MODEL_A3C,                   /* Asynchronous Actor-Critic */
    AI_MODEL_PPO,                   /* Proximal Policy Optimization */
    AI_MODEL_SAC,                   /* Soft Actor-Critic */
    AI_MODEL_DDPG,                  /* Deep Deterministic Policy Gradient */
    /* Quantum machine learning */
    AI_MODEL_QUANTUM_SVM,
    AI_MODEL_QUANTUM_NN,            /* Quantum Neural Network */
    AI_MODEL_VARIATIONAL_QUANTUM_EIGENSOLVER,
    AI_MODEL_QUANTUM_APPROXIMATE_OPTIMIZATION,
    AI_MODEL_QUANTUM_REINFORCEMENT_LEARNING,
    /* Specialized models */
    AI_MODEL_OBJECT_DETECTION,      /* YOLO, R-CNN, etc. */
    AI_MODEL_SEMANTIC_SEGMENTATION,
    AI_MODEL_SPEECH_RECOGNITION,
    AI_MODEL_SPEECH_SYNTHESIS,
    AI_MODEL_MACHINE_TRANSLATION,
    AI_MODEL_SENTIMENT_ANALYSIS,
    AI_MODEL_ANOMALY_DETECTION,
    AI_MODEL_TIME_SERIES_FORECASTING,
    AI_MODEL_RECOMMENDATION_SYSTEM,
    AI_MODEL_FEDERATED_LEARNING,
    AI_MODEL_DIFFERENTIAL_PRIVACY,
    AI_MODEL_HOMOMORPHIC_ENCRYPTION_ML
} ai_model_type_t;

/* AI Hardware Accelerators */
typedef enum {
    AI_ACCEL_NONE,
    AI_ACCEL_CPU,                   /* CPU-based inference */
    AI_ACCEL_GPU_CUDA,              /* NVIDIA CUDA */
    AI_ACCEL_GPU_OPENCL,            /* OpenCL */
    AI_ACCEL_GPU_VULKAN,            /* Vulkan Compute */
    AI_ACCEL_GPU_METAL,             /* Apple Metal */
    AI_ACCEL_TPU,                   /* Google Tensor Processing Unit */
    AI_ACCEL_INTEL_NEURAL_COMPUTE,  /* Intel Neural Compute Stick */
    AI_ACCEL_ARM_NPU,               /* ARM Neural Processing Unit */
    AI_ACCEL_QUALCOMM_DSP,          /* Qualcomm Hexagon DSP */
    AI_ACCEL_HUAWEI_ASCEND,         /* Huawei Ascend */
    AI_ACCEL_XILINX_FPGA,           /* Xilinx FPGA */
    AI_ACCEL_INTEL_FPGA,            /* Intel FPGA */
    AI_ACCEL_GRAPHCORE_IPU,         /* Graphcore Intelligence Processing Unit */
    AI_ACCEL_CEREBRAS_WSE,          /* Cerebras Wafer Scale Engine */
    AI_ACCEL_NEUROMORPHIC_LOIHI,    /* Intel Loihi */
    AI_ACCEL_NEUROMORPHIC_SPINNAKER,/* SpiNNaker */
    AI_ACCEL_QUANTUM_PROCESSOR,     /* Quantum computing processor */
    AI_ACCEL_LIMITLESS_NPU          /* LimitlessOS Neural Processing Unit */
} ai_accelerator_type_t;

/* Data Types */
typedef enum {
    AI_DTYPE_FLOAT32,
    AI_DTYPE_FLOAT64,
    AI_DTYPE_FLOAT16,
    AI_DTYPE_BFLOAT16,
    AI_DTYPE_INT32,
    AI_DTYPE_INT64,
    AI_DTYPE_INT16,
    AI_DTYPE_INT8,
    AI_DTYPE_UINT32,
    AI_DTYPE_UINT64,
    AI_DTYPE_UINT16,
    AI_DTYPE_UINT8,
    AI_DTYPE_BOOL,
    AI_DTYPE_COMPLEX64,
    AI_DTYPE_COMPLEX128,
    AI_DTYPE_QUANTUM_STATE          /* Quantum state representation */
} ai_data_type_t;

/* Optimization Algorithms */
typedef enum {
    AI_OPTIMIZER_SGD,               /* Stochastic Gradient Descent */
    AI_OPTIMIZER_MOMENTUM,
    AI_OPTIMIZER_NESTEROV,
    AI_OPTIMIZER_ADAGRAD,
    AI_OPTIMIZER_ADADELTA,
    AI_OPTIMIZER_RMSPROP,
    AI_OPTIMIZER_ADAM,
    AI_OPTIMIZER_ADAMW,
    AI_OPTIMIZER_ADAMAX,
    AI_OPTIMIZER_NADAM,
    AI_OPTIMIZER_FTRL,
    AI_OPTIMIZER_LAMB,
    AI_OPTIMIZER_QUANTUM_GRADIENT_DESCENT
} ai_optimizer_t;

/* Privacy Preservation Methods */
typedef enum {
    PRIVACY_NONE,
    PRIVACY_DIFFERENTIAL_PRIVACY,
    PRIVACY_FEDERATED_LEARNING,
    PRIVACY_HOMOMORPHIC_ENCRYPTION,
    PRIVACY_SECURE_MULTIPARTY_COMPUTATION,
    PRIVACY_TRUSTED_EXECUTION_ENVIRONMENT,
    PRIVACY_ZERO_KNOWLEDGE_PROOFS,
    PRIVACY_QUANTUM_PRIVACY,
    PRIVACY_K_ANONYMITY,
    PRIVACY_L_DIVERSITY,
    PRIVACY_T_CLOSENESS,
    PRIVACY_SYNTHETIC_DATA_GENERATION
} privacy_method_t;

/* Quantum Computing Types */
typedef enum {
    QUANTUM_TYPE_GATE_BASED,        /* Gate-based quantum computers */
    QUANTUM_TYPE_ANNEALING,         /* Quantum annealing */
    QUANTUM_TYPE_TOPOLOGICAL,       /* Topological quantum computers */
    QUANTUM_TYPE_PHOTONIC,          /* Photonic quantum computers */
    QUANTUM_TYPE_ION_TRAP,          /* Ion trap quantum computers */
    QUANTUM_TYPE_SUPERCONDUCTING,   /* Superconducting quantum computers */
    QUANTUM_TYPE_NEUTRAL_ATOM,      /* Neutral atom quantum computers */
    QUANTUM_TYPE_SILICON_SPIN,      /* Silicon spin quantum computers */
    QUANTUM_TYPE_SIMULATOR          /* Quantum simulator */
} quantum_computer_type_t;

/* AI Tensor */
typedef struct ai_tensor {
    uint32_t id;                    /* Tensor ID */
    char name[64];                  /* Tensor name */
    
    /* Shape and dimensions */
    uint32_t ndim;                  /* Number of dimensions */
    uint64_t shape[8];              /* Dimension sizes */
    uint64_t strides[8];            /* Strides for each dimension */
    uint64_t size;                  /* Total number of elements */
    
    /* Data properties */
    ai_data_type_t dtype;           /* Data type */
    size_t element_size;            /* Size of each element in bytes */
    uint64_t total_bytes;           /* Total memory size */
    
    /* Memory management */
    void* data;                     /* Tensor data */
    bool owns_data;                 /* Whether tensor owns the data */
    bool is_contiguous;             /* Memory layout is contiguous */
    
    /* Device and acceleration */
    ai_accelerator_type_t device;   /* Device where tensor resides */
    void* device_ptr;               /* Device-specific pointer */
    
    /* Gradient information (for automatic differentiation) */
    bool requires_grad;             /* Whether gradients are required */
    struct ai_tensor* grad;         /* Gradient tensor */
    
    /* Privacy protection */
    privacy_method_t privacy_method;
    bool is_encrypted;
    void* privacy_metadata;
    
    struct ai_tensor* next;
} ai_tensor_t;

/* Neural Network Layer */
typedef struct ai_layer {
    uint32_t id;                    /* Layer ID */
    char name[64];                  /* Layer name */
    
    enum {
        AI_LAYER_DENSE,             /* Fully connected layer */
        AI_LAYER_CONV2D,            /* 2D Convolution */
        AI_LAYER_CONV3D,            /* 3D Convolution */
        AI_LAYER_DEPTHWISE_CONV2D,  /* Depthwise separable convolution */
        AI_LAYER_TRANSPOSE_CONV2D,  /* Transposed convolution */
        AI_LAYER_MAXPOOL2D,         /* Max pooling */
        AI_LAYER_AVGPOOL2D,         /* Average pooling */
        AI_LAYER_GLOBALPOOL,        /* Global pooling */
        AI_LAYER_DROPOUT,           /* Dropout regularization */
        AI_LAYER_BATCHNORM,         /* Batch normalization */
        AI_LAYER_LAYERNORM,         /* Layer normalization */
        AI_LAYER_GROUPNORM,         /* Group normalization */
        AI_LAYER_ACTIVATION,        /* Activation function */
        AI_LAYER_ATTENTION,         /* Attention mechanism */
        AI_LAYER_MULTIHEAD_ATTENTION, /* Multi-head attention */
        AI_LAYER_POSITIONAL_ENCODING, /* Positional encoding */
        AI_LAYER_EMBEDDING,         /* Embedding layer */
        AI_LAYER_LSTM,              /* LSTM layer */
        AI_LAYER_GRU,               /* GRU layer */
        AI_LAYER_RNN,               /* Basic RNN layer */
        AI_LAYER_TRANSFORMER_BLOCK, /* Transformer block */
        AI_LAYER_RESIDUAL_BLOCK,    /* Residual connection block */
        AI_LAYER_QUANTUM_LAYER      /* Quantum layer */
    } type;
    
    /* Layer parameters */
    ai_tensor_t* weights;           /* Layer weights */
    ai_tensor_t* biases;            /* Layer biases */
    ai_tensor_t* running_mean;      /* For batch normalization */
    ai_tensor_t* running_var;       /* For batch normalization */
    
    /* Layer configuration */
    union {
        struct {
            uint32_t input_size;
            uint32_t output_size;
        } dense;
        
        struct {
            uint32_t filters;
            uint32_t kernel_size[2];
            uint32_t strides[2];
            uint32_t padding[2];
            uint32_t dilation[2];
        } conv2d;
        
        struct {
            uint32_t pool_size[2];
            uint32_t strides[2];
            uint32_t padding[2];
        } pool2d;
        
        struct {
            float dropout_rate;
        } dropout;
        
        struct {
            float epsilon;
            float momentum;
        } batch_norm;
        
        struct {
            uint32_t num_heads;
            uint32_t d_model;
            uint32_t d_k;
            uint32_t d_v;
        } attention;
        
        struct {
            uint32_t hidden_size;
            uint32_t num_layers;
            bool bidirectional;
        } lstm;
        
        struct {
            uint32_t num_qubits;
            void* quantum_circuit;
        } quantum;
    } config;
    
    /* Activation function */
    enum {
        AI_ACTIVATION_NONE,
        AI_ACTIVATION_RELU,
        AI_ACTIVATION_LEAKY_RELU,
        AI_ACTIVATION_ELU,
        AI_ACTIVATION_SELU,
        AI_ACTIVATION_GELU,
        AI_ACTIVATION_SWISH,
        AI_ACTIVATION_MISH,
        AI_ACTIVATION_SIGMOID,
        AI_ACTIVATION_TANH,
        AI_ACTIVATION_SOFTMAX,
        AI_ACTIVATION_SOFTPLUS,
        AI_ACTIVATION_SOFTSIGN,
        AI_ACTIVATION_QUANTUM
    } activation;
    
    /* Forward and backward pass functions */
    status_t (*forward)(struct ai_layer* layer, ai_tensor_t* input, ai_tensor_t** output);
    status_t (*backward)(struct ai_layer* layer, ai_tensor_t* grad_output, ai_tensor_t** grad_input);
    
    struct ai_layer* next;
} ai_layer_t;

/* AI Model */
typedef struct ai_model {
    uint32_t id;                    /* Model ID */
    char name[128];                 /* Model name */
    char description[512];          /* Model description */
    ai_model_type_t type;           /* Model type */
    
    /* Model architecture */
    ai_layer_t* layers;             /* Model layers */
    uint32_t layer_count;           /* Number of layers */
    ai_tensor_t* input_tensor;      /* Input tensor specification */
    ai_tensor_t* output_tensor;     /* Output tensor specification */
    
    /* Training configuration */
    ai_optimizer_t optimizer;       /* Optimization algorithm */
    float learning_rate;            /* Learning rate */
    float weight_decay;             /* Weight decay (L2 regularization) */
    uint32_t batch_size;            /* Training batch size */
    uint32_t num_epochs;            /* Number of training epochs */
    
    /* Model metadata */
    char framework[64];             /* ML framework (TensorFlow, PyTorch, etc.) */
    char version[32];               /* Model version */
    uint64_t creation_time;         /* Model creation timestamp */
    uint64_t last_modified;         /* Last modification timestamp */
    uint64_t model_size_bytes;      /* Model size in bytes */
    uint32_t parameter_count;       /* Number of parameters */
    
    /* Performance metrics */
    struct {
        float accuracy;             /* Model accuracy */
        float precision;            /* Precision metric */
        float recall;               /* Recall metric */
        float f1_score;             /* F1 score */
        float loss;                 /* Training loss */
        float validation_loss;      /* Validation loss */
        float inference_time_ms;    /* Average inference time */
        uint64_t training_time_total; /* Total training time */
        uint32_t flops;             /* Floating point operations */
    } metrics;
    
    /* Privacy and security */
    privacy_method_t privacy_method;
    bool is_encrypted;
    bool differential_privacy_enabled;
    float privacy_budget;           /* Differential privacy budget */
    void* privacy_metadata;
    
    /* Hardware acceleration */
    ai_accelerator_type_t preferred_device;
    bool supports_quantization;
    bool supports_pruning;
    bool supports_distillation;
    
    /* Model state */
    enum {
        AI_MODEL_STATE_CREATED,
        AI_MODEL_STATE_TRAINING,
        AI_MODEL_STATE_TRAINED,
        AI_MODEL_STATE_DEPLOYED,
        AI_MODEL_STATE_RETIRED
    } state;
    
    bool is_loaded;
    void* model_data;               /* Loaded model data */
    
    struct ai_model* next;
} ai_model_t;

/* Dataset */
typedef struct ai_dataset {
    uint32_t id;                    /* Dataset ID */
    char name[128];                 /* Dataset name */
    char path[512];                 /* Dataset file path */
    
    /* Dataset properties */
    uint64_t num_samples;           /* Number of samples */
    uint64_t num_features;          /* Number of features */
    uint32_t num_classes;           /* Number of classes (for classification) */
    ai_data_type_t dtype;           /* Data type */
    
    /* Data splits */
    float train_split;              /* Training data percentage */
    float validation_split;         /* Validation data percentage */
    float test_split;               /* Test data percentage */
    
    /* Privacy protection */
    privacy_method_t privacy_method;
    bool is_anonymized;
    bool contains_pii;              /* Contains personally identifiable information */
    void* privacy_metadata;
    
    /* Data preprocessing */
    struct {
        bool normalize;
        bool standardize;
        bool shuffle;
        bool augment;
        float noise_level;          /* Data augmentation noise */
    } preprocessing;
    
    /* Memory management */
    bool is_loaded;
    void* data_ptr;                 /* Loaded data pointer */
    uint64_t memory_usage_bytes;
    
    struct ai_dataset* next;
} ai_dataset_t;

/* AI Accelerator Device */
typedef struct ai_accelerator {
    uint32_t id;                    /* Accelerator ID */
    ai_accelerator_type_t type;     /* Accelerator type */
    char name[128];                 /* Device name */
    char vendor[64];                /* Device vendor */
    char driver_version[32];        /* Driver version */
    
    /* Compute capabilities */
    uint32_t compute_units;         /* Number of compute units */
    uint32_t clock_speed_mhz;       /* Clock speed in MHz */
    uint64_t memory_size_bytes;     /* Device memory size */
    uint64_t memory_bandwidth_gbps; /* Memory bandwidth */
    uint32_t tensor_cores;          /* Number of tensor cores (if applicable) */
    
    /* Supported operations */
    struct {
        bool fp32;                  /* 32-bit floating point */
        bool fp16;                  /* 16-bit floating point */
        bool bf16;                  /* BFloat16 */
        bool int8;                  /* 8-bit integer */
        bool int4;                  /* 4-bit integer */
        bool sparse;                /* Sparse tensor operations */
        bool mixed_precision;       /* Mixed precision training */
        bool tensor_ops;            /* Tensor operations */
        bool conv2d;                /* 2D convolution */
        bool conv3d;                /* 3D convolution */
        bool attention;             /* Attention mechanisms */
        bool transformer;           /* Transformer operations */
        bool quantum_ops;           /* Quantum operations */
    } capabilities;
    
    /* Performance metrics */
    uint32_t peak_tflops_fp32;      /* Peak TFLOPS (FP32) */
    uint32_t peak_tflops_fp16;      /* Peak TFLOPS (FP16) */
    uint32_t peak_tops_int8;        /* Peak TOPS (INT8) */
    
    /* Power management */
    uint32_t power_consumption_watts; /* Power consumption */
    uint32_t thermal_design_power;  /* TDP */
    uint32_t current_temperature;   /* Current temperature */
    
    /* Device status */
    bool is_available;
    bool is_busy;
    uint32_t utilization_percent;
    uint64_t memory_used_bytes;
    uint64_t memory_free_bytes;
    
    /* Device interface */
    void* device_context;           /* Device-specific context */
    status_t (*initialize)(struct ai_accelerator* accel);
    status_t (*allocate_memory)(struct ai_accelerator* accel, uint64_t size, void** ptr);
    status_t (*free_memory)(struct ai_accelerator* accel, void* ptr);
    status_t (*copy_to_device)(struct ai_accelerator* accel, const void* host_ptr, void* device_ptr, uint64_t size);
    status_t (*copy_from_device)(struct ai_accelerator* accel, const void* device_ptr, void* host_ptr, uint64_t size);
    status_t (*execute_kernel)(struct ai_accelerator* accel, void* kernel, void** args, uint32_t arg_count);
    
    struct ai_accelerator* next;
} ai_accelerator_t;

/* Quantum Circuit */
typedef struct quantum_circuit {
    uint32_t id;                    /* Circuit ID */
    char name[128];                 /* Circuit name */
    uint32_t num_qubits;            /* Number of qubits */
    uint32_t num_classical_bits;    /* Number of classical bits */
    
    /* Quantum gates */
    struct {
        enum {
            QUANTUM_GATE_I,         /* Identity */
            QUANTUM_GATE_X,         /* Pauli-X */
            QUANTUM_GATE_Y,         /* Pauli-Y */
            QUANTUM_GATE_Z,         /* Pauli-Z */
            QUANTUM_GATE_H,         /* Hadamard */
            QUANTUM_GATE_S,         /* Phase */
            QUANTUM_GATE_T,         /* T gate */
            QUANTUM_GATE_RX,        /* Rotation around X */
            QUANTUM_GATE_RY,        /* Rotation around Y */
            QUANTUM_GATE_RZ,        /* Rotation around Z */
            QUANTUM_GATE_CNOT,      /* Controlled NOT */
            QUANTUM_GATE_CZ,        /* Controlled Z */
            QUANTUM_GATE_SWAP,      /* SWAP gate */
            QUANTUM_GATE_TOFFOLI,   /* Toffoli gate */
            QUANTUM_GATE_FREDKIN,   /* Fredkin gate */
            QUANTUM_GATE_MEASUREMENT /* Measurement */
        } type;
        uint32_t target_qubits[4];  /* Target qubit indices */
        uint32_t control_qubits[4]; /* Control qubit indices */
        float parameters[4];        /* Gate parameters */
    }* gates;
    uint32_t gate_count;
    
    /* Circuit properties */
    uint32_t depth;                 /* Circuit depth */
    uint32_t two_qubit_gate_count;  /* Number of two-qubit gates */
    
    struct quantum_circuit* next;
} quantum_circuit_t;

/* Quantum Computer */
typedef struct quantum_computer {
    uint32_t id;                    /* Quantum computer ID */
    char name[128];                 /* Quantum computer name */
    quantum_computer_type_t type;   /* Type of quantum computer */
    char vendor[64];                /* Vendor */
    
    /* Hardware specifications */
    uint32_t num_qubits;            /* Number of physical qubits */
    uint32_t num_logical_qubits;    /* Number of logical qubits */
    float coherence_time_us;        /* Qubit coherence time */
    float gate_fidelity;            /* Average gate fidelity */
    float measurement_fidelity;     /* Measurement fidelity */
    
    /* Connectivity */
    struct {
        uint32_t qubit1;
        uint32_t qubit2;
        float coupling_strength;
    }* connectivity_map;
    uint32_t connection_count;
    
    /* Error rates */
    float single_qubit_error_rate;
    float two_qubit_error_rate;
    float readout_error_rate;
    
    /* Quantum error correction */
    bool supports_error_correction;
    char error_correction_code[64];
    
    /* Status */
    bool is_available;
    bool is_calibrated;
    uint64_t last_calibration_time;
    uint32_t queue_length;          /* Number of pending jobs */
    
    /* Interface */
    void* quantum_context;
    status_t (*initialize)(struct quantum_computer* qc);
    status_t (*submit_circuit)(struct quantum_computer* qc, quantum_circuit_t* circuit, void** job_handle);
    status_t (*get_results)(struct quantum_computer* qc, void* job_handle, void* results);
    status_t (*calibrate)(struct quantum_computer* qc);
    
    struct quantum_computer* next;
} quantum_computer_t;

/* AI Inference Session */
typedef struct ai_inference_session {
    uint32_t id;                    /* Session ID */
    ai_model_t* model;              /* Associated model */
    ai_accelerator_t* accelerator;  /* Hardware accelerator */
    
    /* Session configuration */
    uint32_t batch_size;            /* Inference batch size */
    ai_data_type_t input_dtype;     /* Input data type */
    ai_data_type_t output_dtype;    /* Output data type */
    
    /* Performance optimization */
    bool enable_quantization;
    bool enable_pruning;
    bool enable_tensorrt;           /* NVIDIA TensorRT optimization */
    bool enable_onnx_runtime;       /* ONNX Runtime optimization */
    
    /* Privacy settings */
    privacy_method_t privacy_method;
    bool enable_differential_privacy;
    float privacy_budget;
    
    /* Session state */
    bool is_active;
    uint64_t inference_count;
    float avg_inference_time_ms;
    uint64_t total_input_bytes;
    uint64_t total_output_bytes;
    
    /* Memory management */
    void* input_buffer;
    void* output_buffer;
    size_t input_buffer_size;
    size_t output_buffer_size;
    
} ai_inference_session_t;

/* AI Training Job */
typedef struct ai_training_job {
    uint32_t id;                    /* Job ID */
    ai_model_t* model;              /* Model being trained */
    ai_dataset_t* dataset;          /* Training dataset */
    ai_accelerator_t* accelerator;  /* Hardware accelerator */
    
    /* Training configuration */
    uint32_t epochs;                /* Number of epochs */
    uint32_t batch_size;            /* Training batch size */
    float learning_rate;            /* Learning rate */
    ai_optimizer_t optimizer;       /* Optimizer algorithm */
    
    /* Regularization */
    float weight_decay;             /* L2 regularization */
    float dropout_rate;             /* Dropout rate */
    bool early_stopping;            /* Early stopping enabled */
    uint32_t patience;              /* Early stopping patience */
    
    /* Privacy settings */
    privacy_method_t privacy_method;
    bool enable_differential_privacy;
    float privacy_budget;
    
    /* Job status */
    enum {
        AI_JOB_PENDING,
        AI_JOB_RUNNING,
        AI_JOB_COMPLETED,
        AI_JOB_FAILED,
        AI_JOB_CANCELLED
    } status;
    
    uint32_t current_epoch;
    float progress_percent;
    uint64_t start_time;
    uint64_t end_time;
    uint64_t estimated_completion_time;
    
    /* Training metrics */
    struct {
        float* training_loss;       /* Loss per epoch */
        float* validation_loss;     /* Validation loss per epoch */
        float* training_accuracy;   /* Training accuracy per epoch */
        float* validation_accuracy; /* Validation accuracy per epoch */
        uint32_t metric_count;      /* Number of recorded metrics */
    } metrics;
    
    /* Checkpointing */
    bool enable_checkpointing;
    uint32_t checkpoint_frequency;  /* Epochs between checkpoints */
    char checkpoint_path[512];
    
    struct ai_training_job* next;
} ai_training_job_t;

/* Privacy Settings */
typedef struct ai_privacy_settings {
    bool ai_system_enabled;         /* Master AI system enable/disable */
    bool data_collection_enabled;   /* Data collection enabled */
    bool telemetry_enabled;         /* Telemetry enabled */
    bool usage_analytics_enabled;   /* Usage analytics enabled */
    bool model_improvement_enabled; /* Model improvement data sharing */
    
    /* Data retention */
    uint32_t data_retention_days;   /* Data retention period */
    bool auto_delete_data;          /* Automatically delete old data */
    
    /* Privacy techniques */
    bool differential_privacy_default; /* Use differential privacy by default */
    float default_privacy_budget;   /* Default privacy budget */
    bool federated_learning_preferred; /* Prefer federated learning */
    bool homomorphic_encryption_enabled; /* Use homomorphic encryption */
    
    /* User consent */
    bool explicit_consent_required; /* Require explicit consent */
    bool opt_in_by_default;         /* Opt-in by default */
    bool granular_permissions;      /* Allow granular permissions */
    
    /* Data minimization */
    bool minimize_data_collection;  /* Minimize data collection */
    bool anonymize_data;            /* Anonymize collected data */
    bool pseudonymize_data;         /* Pseudonymize collected data */
    
    /* User control */
    bool allow_data_export;         /* Allow users to export their data */
    bool allow_data_deletion;       /* Allow users to delete their data */
    bool allow_model_opt_out;       /* Allow opting out of model training */
    
} ai_privacy_settings_t;

/* AI Framework Manager */
typedef struct ai_framework {
    bool initialized;
    uint32_t version;
    
    /* Privacy settings */
    ai_privacy_settings_t privacy_settings;
    
    /* Models */
    ai_model_t* models;
    uint32_t model_count;
    
    /* Datasets */
    ai_dataset_t* datasets;
    uint32_t dataset_count;
    
    /* Hardware accelerators */
    ai_accelerator_t* accelerators;
    uint32_t accelerator_count;
    ai_accelerator_t* default_accelerator;
    
    /* Quantum computers */
    quantum_computer_t* quantum_computers;
    uint32_t quantum_computer_count;
    
    /* Active sessions */
    ai_inference_session_t* inference_sessions[MAX_INFERENCE_CONTEXTS];
    uint32_t active_inference_sessions;
    
    /* Training jobs */
    ai_training_job_t* training_jobs;
    uint32_t active_training_jobs;
    
    /* Framework libraries */
    struct {
        bool tensorflow_available;
        bool pytorch_available;
        bool onnx_available;
        bool scikit_learn_available;
        bool xgboost_available;
        bool lightgbm_available;
        bool catboost_available;
        bool huggingface_available;
        bool qiskit_available;      /* Quantum computing */
        bool cirq_available;        /* Google Cirq */
        bool pennylane_available;   /* PennyLane quantum ML */
    } libraries;
    
    /* System statistics */
    struct {
        uint64_t total_inferences;
        uint64_t total_training_jobs;
        uint64_t total_models_trained;
        uint64_t compute_time_hours;
        uint64_t data_processed_gb;
        float average_model_accuracy;
        uint32_t active_users;
    } statistics;
    
    /* Performance settings */
    struct {
        bool auto_mixed_precision;
        bool gradient_checkpointing;
        bool model_parallelism;
        bool data_parallelism;
        uint32_t max_batch_size;
        uint32_t memory_limit_gb;
    } performance_settings;
    
} ai_framework_t;

/* Global AI framework */
extern ai_framework_t ai_framework;

/* Core AI API */
status_t ai_init(void);
void ai_shutdown(void);
bool ai_is_enabled(void);
status_t ai_enable_system(bool enable);

/* Privacy Management */
status_t ai_set_privacy_settings(const ai_privacy_settings_t* settings);
ai_privacy_settings_t ai_get_privacy_settings(void);
status_t ai_request_user_consent(const char* purpose, bool* granted);
status_t ai_anonymize_data(void* data, size_t size, void** anonymized_data);
status_t ai_apply_differential_privacy(ai_tensor_t* tensor, float epsilon);

/* Hardware Detection and Management */
status_t ai_detect_accelerators(void);
ai_accelerator_t* ai_get_accelerator_by_type(ai_accelerator_type_t type);
ai_accelerator_t* ai_get_best_accelerator_for_model(ai_model_t* model);
status_t ai_benchmark_accelerator(ai_accelerator_t* accelerator);

/* Tensor Operations */
ai_tensor_t* ai_create_tensor(const uint64_t* shape, uint32_t ndim, ai_data_type_t dtype);
void ai_destroy_tensor(ai_tensor_t* tensor);
status_t ai_tensor_copy(ai_tensor_t* src, ai_tensor_t* dst);
status_t ai_tensor_reshape(ai_tensor_t* tensor, const uint64_t* new_shape, uint32_t new_ndim);
status_t ai_tensor_to_device(ai_tensor_t* tensor, ai_accelerator_t* device);
status_t ai_tensor_to_host(ai_tensor_t* tensor);

/* Model Management */
ai_model_t* ai_create_model(const char* name, ai_model_type_t type);
void ai_destroy_model(ai_model_t* model);
status_t ai_load_model(const char* model_path, ai_model_t** model);
status_t ai_save_model(ai_model_t* model, const char* model_path);
status_t ai_compile_model(ai_model_t* model, ai_accelerator_t* target_device);
ai_model_t* ai_get_model_by_name(const char* name);

/* Layer Operations */
ai_layer_t* ai_create_layer(const char* name, uint32_t layer_type);
void ai_destroy_layer(ai_layer_t* layer);
status_t ai_add_layer_to_model(ai_model_t* model, ai_layer_t* layer);
status_t ai_remove_layer_from_model(ai_model_t* model, ai_layer_t* layer);

/* Dataset Management */
ai_dataset_t* ai_create_dataset(const char* name, const char* path);
void ai_destroy_dataset(ai_dataset_t* dataset);
status_t ai_load_dataset(ai_dataset_t* dataset);
status_t ai_preprocess_dataset(ai_dataset_t* dataset);
status_t ai_split_dataset(ai_dataset_t* dataset, float train_split, float val_split, float test_split);

/* Training */
ai_training_job_t* ai_create_training_job(ai_model_t* model, ai_dataset_t* dataset);
void ai_destroy_training_job(ai_training_job_t* job);
status_t ai_start_training(ai_training_job_t* job);
status_t ai_stop_training(ai_training_job_t* job);
status_t ai_pause_training(ai_training_job_t* job);
status_t ai_resume_training(ai_training_job_t* job);
float ai_get_training_progress(ai_training_job_t* job);

/* Inference */
ai_inference_session_t* ai_create_inference_session(ai_model_t* model);
void ai_destroy_inference_session(ai_inference_session_t* session);
status_t ai_run_inference(ai_inference_session_t* session, ai_tensor_t* input, ai_tensor_t** output);
status_t ai_run_batch_inference(ai_inference_session_t* session, ai_tensor_t** inputs, uint32_t batch_size, ai_tensor_t*** outputs);

/* Model Optimization */
status_t ai_quantize_model(ai_model_t* model, ai_data_type_t target_dtype);
status_t ai_prune_model(ai_model_t* model, float pruning_ratio);
status_t ai_distill_model(ai_model_t* teacher_model, ai_model_t* student_model, ai_dataset_t* dataset);
status_t ai_optimize_model_for_device(ai_model_t* model, ai_accelerator_t* device);

/* Federated Learning */
status_t ai_create_federated_learning_session(ai_model_t* global_model, const char* session_name);
status_t ai_join_federated_learning(const char* session_name, ai_dataset_t* local_data);
status_t ai_update_federated_model(ai_model_t* local_model, ai_model_t* global_model);

/* Quantum Machine Learning */
quantum_circuit_t* ai_create_quantum_circuit(uint32_t num_qubits);
void ai_destroy_quantum_circuit(quantum_circuit_t* circuit);
status_t ai_add_quantum_gate(quantum_circuit_t* circuit, uint32_t gate_type, const uint32_t* qubits, const float* parameters);
status_t ai_run_quantum_circuit(quantum_computer_t* qc, quantum_circuit_t* circuit, void** results);
status_t ai_create_quantum_model(ai_model_t* classical_model, quantum_circuit_t* quantum_circuit, ai_model_t** hybrid_model);

/* AutoML */
status_t ai_auto_model_selection(ai_dataset_t* dataset, ai_model_t** best_model);
status_t ai_hyperparameter_optimization(ai_model_t* model, ai_dataset_t* dataset);
status_t ai_neural_architecture_search(ai_dataset_t* dataset, ai_model_t** optimized_model);

/* Model Interpretability */
status_t ai_explain_prediction(ai_model_t* model, ai_tensor_t* input, void** explanation);
status_t ai_generate_feature_importance(ai_model_t* model, ai_dataset_t* dataset, float** importance_scores);
status_t ai_visualize_model_attention(ai_model_t* model, ai_tensor_t* input, ai_tensor_t** attention_map);

/* Privacy-Preserving ML */
status_t ai_enable_differential_privacy_training(ai_training_job_t* job, float epsilon, float delta);
status_t ai_create_homomorphic_encryption_model(ai_model_t* plaintext_model, ai_model_t** encrypted_model);
status_t ai_secure_multiparty_computation_inference(ai_model_t** party_models, uint32_t party_count, ai_tensor_t* input, ai_tensor_t** output);

/* Model Monitoring and MLOps */
status_t ai_monitor_model_drift(ai_model_t* model, ai_dataset_t* new_data, float* drift_score);
status_t ai_validate_model_fairness(ai_model_t* model, ai_dataset_t* dataset, void** fairness_metrics);
status_t ai_audit_model_performance(ai_model_t* model, ai_dataset_t* test_data, void** audit_report);

/* Utilities */
const char* ai_get_model_type_name(ai_model_type_t type);
const char* ai_get_accelerator_type_name(ai_accelerator_type_t type);
const char* ai_get_data_type_name(ai_data_type_t dtype);
size_t ai_get_data_type_size(ai_data_type_t dtype);
bool ai_is_quantum_model_type(ai_model_type_t type);

/* Statistics and Reporting */
void ai_print_framework_statistics(void);
void ai_print_model_summary(ai_model_t* model);
void ai_print_training_progress(ai_training_job_t* job);
status_t ai_export_model_metrics(ai_model_t* model, const char* export_path);

#endif /* LIMITLESS_AI_FRAMEWORK_H */