/*
 * LimitlessOS Neuromorphic Computing Interface
 * Brain-inspired computing framework with spiking neural networks and adaptive learning
 */

#ifndef NEUROMORPHIC_COMPUTING_H
#define NEUROMORPHIC_COMPUTING_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

/* Maximum system limits */
#define MAX_NEUROMORPHIC_DEVICES    64
#define MAX_NEURAL_CORES           256
#define MAX_NEURONS_PER_CORE      8192
#define MAX_SYNAPSES_PER_NEURON    512
#define MAX_SPIKE_TRAINS           128
#define MAX_LEARNING_RULES          32
#define MAX_PLASTICITY_WINDOWS      16
#define MAX_NETWORK_LAYERS          64
#define MAX_ADAPTATION_POLICIES     16
#define MAX_MEMBRANE_MODELS         16
#define MAX_NEURAL_POPULATIONS    1024
#define MAX_HOMEOSTASIS_RULES       8
#define MAX_NEURAL_OSCILLATORS     64
#define MAX_SYNCHRONY_GROUPS       32
#define MAX_RESERVOIR_NETWORKS     16
#define MAX_NEURAL_INTERFACES       8

/* Neuromorphic device types */
typedef enum {
    NEURO_DEVICE_UNKNOWN = 0,
    NEURO_DEVICE_INTEL_LOIHI,        /* Intel Loihi neuromorphic chip */
    NEURO_DEVICE_IBM_TRUENORTH,      /* IBM TrueNorth */
    NEURO_DEVICE_SPINNAKER,          /* SpiNNaker system */
    NEURO_DEVICE_BRAINSCALES,        /* BrainScaleS platform */
    NEURO_DEVICE_DYNAPSE,            /* DYNAPse chip */
    NEURO_DEVICE_AKIDA,              /* BrainChip Akida */
    NEURO_DEVICE_MEMRISTOR_ARRAY,    /* Memristor-based hardware */
    NEURO_DEVICE_OPTICAL_NEURAL,     /* Optical neural processing */
    NEURO_DEVICE_QUANTUM_NEURAL,     /* Quantum neural networks */
    NEURO_DEVICE_FPGA_NEURAL,        /* FPGA-based implementation */
    NEURO_DEVICE_GPU_SPIKING,        /* GPU-accelerated spiking */
    NEURO_DEVICE_CUSTOM,             /* Custom implementation */
    NEURO_DEVICE_MAX
} neuromorphic_device_type_t;

/* Neuron models */
typedef enum {
    NEURON_MODEL_INTEGRATE_FIRE = 0,
    NEURON_MODEL_LEAKY_INTEGRATE_FIRE,
    NEURON_MODEL_EXPONENTIAL_INTEGRATE_FIRE,
    NEURON_MODEL_ADAPTIVE_EXPONENTIAL,
    NEURON_MODEL_IZHIKEVICH,
    NEURON_MODEL_HODGKIN_HUXLEY,
    NEURON_MODEL_FITZHUGH_NAGUMO,
    NEURON_MODEL_MORRIS_LECAR,
    NEURON_MODEL_HINDMARSH_ROSE,
    NEURON_MODEL_QUADRATIC_INTEGRATE_FIRE,
    NEURON_MODEL_RESONATE_FIRE,
    NEURON_MODEL_THETA_MODEL,
    NEURON_MODEL_SPIKING_NEURAL_GAS,
    NEURON_MODEL_LIQUID_STATE_MACHINE,
    NEURON_MODEL_ECHO_STATE_NETWORK,
    NEURON_MODEL_MAX
} neuron_model_t;

/* Synaptic plasticity types */
typedef enum {
    PLASTICITY_NONE = 0,
    PLASTICITY_STDP,                 /* Spike-timing dependent plasticity */
    PLASTICITY_ANTI_STDP,            /* Anti-STDP */
    PLASTICITY_TRIPLET_STDP,         /* Triplet STDP rule */
    PLASTICITY_BCM,                  /* Bienenstock-Cooper-Munro rule */
    PLASTICITY_HOMEOSTATIC,          /* Homeostatic plasticity */
    PLASTICITY_METAPLASTICITY,       /* Metaplasticity */
    PLASTICITY_STRUCTURAL,           /* Structural plasticity */
    PLASTICITY_SHORT_TERM,           /* Short-term plasticity */
    PLASTICITY_VOLTAGE_DEPENDENT,    /* Voltage-dependent plasticity */
    PLASTICITY_CALCIUM_DEPENDENT,    /* Calcium-dependent plasticity */
    PLASTICITY_DOPAMINE_MODULATED,   /* Dopamine-modulated plasticity */
    PLASTICITY_REWARD_MODULATED,     /* Reward-modulated plasticity */
    PLASTICITY_COMPETITIVE,          /* Competitive plasticity */
    PLASTICITY_COOPERATIVE,          /* Cooperative plasticity */
    PLASTICITY_MAX
} synaptic_plasticity_t;

/* Learning algorithms */
typedef enum {
    LEARNING_UNSUPERVISED = 0,
    LEARNING_SUPERVISED,
    LEARNING_REINFORCEMENT,
    LEARNING_SEMI_SUPERVISED,
    LEARNING_ACTIVE,
    LEARNING_ONLINE,
    LEARNING_OFFLINE,
    LEARNING_CONTINUAL,
    LEARNING_TRANSFER,
    LEARNING_META,
    LEARNING_FEDERATED,
    LEARNING_SELF_SUPERVISED,
    LEARNING_CONTRASTIVE,
    LEARNING_ADVERSARIAL,
    LEARNING_EVOLUTIONARY,
    LEARNING_MAX
} learning_algorithm_t;

/* Neural encoding schemes */
typedef enum {
    ENCODING_RATE = 0,               /* Rate coding */
    ENCODING_TEMPORAL,               /* Temporal coding */
    ENCODING_POPULATION,             /* Population coding */
    ENCODING_SPARSE,                 /* Sparse coding */
    ENCODING_RANK_ORDER,             /* Rank order coding */
    ENCODING_PHASE,                  /* Phase coding */
    ENCODING_BURST,                  /* Burst coding */
    ENCODING_LATENCY,                /* Latency coding */
    ENCODING_SYNCHRONY,              /* Synchrony coding */
    ENCODING_OSCILLATORY,            /* Oscillatory coding */
    ENCODING_DELTA,                  /* Delta coding */
    ENCODING_BEN_SPIKES,             /* Ben's spike coding */
    ENCODING_REAL_VALUED,            /* Real-valued encoding */
    ENCODING_STOCHASTIC,             /* Stochastic encoding */
    ENCODING_BERNOULLI,              /* Bernoulli encoding */
    ENCODING_MAX
} neural_encoding_t;

/* Spike data structure */
typedef struct {
    uint32_t neuron_id;              /* Source neuron ID */
    uint64_t timestamp;              /* Spike time in nanoseconds */
    uint32_t core_id;                /* Neural core ID */
    uint8_t spike_type;              /* Spike type/class */
    float amplitude;                 /* Spike amplitude */
    uint16_t flags;                  /* Additional spike flags */
} neural_spike_t;

/* Neuron parameters */
typedef struct {
    uint32_t neuron_id;              /* Unique neuron identifier */
    neuron_model_t model;            /* Neuron model type */
    
    /* Membrane parameters */
    float membrane_potential;        /* Current membrane potential (mV) */
    float resting_potential;         /* Resting potential (mV) */
    float threshold_potential;       /* Spike threshold (mV) */
    float reset_potential;           /* Reset potential after spike (mV) */
    float membrane_capacitance;      /* Membrane capacitance (pF) */
    float membrane_resistance;       /* Membrane resistance (MΩ) */
    float membrane_time_constant;    /* Membrane time constant (ms) */
    
    /* Adaptation parameters */
    float adaptation_current;        /* Adaptation current (nA) */
    float adaptation_time_constant;  /* Adaptation time constant (ms) */
    float spike_adaptation_increment;/* Spike-triggered adaptation */
    
    /* Noise and stochasticity */
    float noise_amplitude;           /* Noise amplitude */
    float background_current;        /* Background input current */
    uint32_t random_seed;            /* Random number seed */
    
    /* Refractory period */
    float absolute_refractory_period;/* Absolute refractory period (ms) */
    float relative_refractory_period;/* Relative refractory period (ms) */
    uint64_t last_spike_time;        /* Last spike timestamp */
    
    /* Synaptic integration */
    float excitatory_reversal;       /* Excitatory reversal potential */
    float inhibitory_reversal;       /* Inhibitory reversal potential */
    float synaptic_time_constant_exc;/* Excitatory synaptic time constant */
    float synaptic_time_constant_inh;/* Inhibitory synaptic time constant */
    
    /* State variables */
    float recovery_variable;         /* Recovery variable (Izhikevich) */
    float calcium_concentration;     /* Intracellular calcium */
    float sodium_current;            /* Sodium current state */
    float potassium_current;         /* Potassium current state */
    
    /* Plasticity state */
    float synaptic_trace_pre;        /* Pre-synaptic trace */
    float synaptic_trace_post;       /* Post-synaptic trace */
    uint64_t last_pre_spike;         /* Last pre-synaptic spike */
    uint64_t last_post_spike;        /* Last post-synaptic spike */
    
    /* Statistics */
    uint64_t total_spikes;           /* Total spike count */
    float firing_rate;               /* Current firing rate (Hz) */
    float average_isi;               /* Average inter-spike interval */
    float cv_isi;                    /* Coefficient of variation of ISI */
    
    pthread_mutex_t lock;            /* Thread safety */
} neural_neuron_t;

/* Synapse structure */
typedef struct {
    uint32_t synapse_id;             /* Unique synapse identifier */
    uint32_t pre_neuron_id;          /* Pre-synaptic neuron */
    uint32_t post_neuron_id;         /* Post-synaptic neuron */
    
    /* Synaptic parameters */
    float weight;                    /* Synaptic weight */
    float weight_min;                /* Minimum weight */
    float weight_max;                /* Maximum weight */
    float transmission_delay;        /* Transmission delay (ms) */
    
    /* Plasticity parameters */
    synaptic_plasticity_t plasticity_type;
    float learning_rate;             /* Learning rate */
    float eligibility_trace;         /* Eligibility trace */
    float plasticity_threshold;      /* Plasticity threshold */
    
    /* Short-term dynamics */
    float facilitation_factor;       /* Facilitation factor */
    float depression_factor;         /* Depression factor */
    float recovery_time_constant;    /* Recovery time constant */
    float utilization_factor;        /* Utilization factor */
    
    /* STDP parameters */
    float stdp_tau_plus;             /* STDP positive window */
    float stdp_tau_minus;            /* STDP negative window */
    float stdp_a_plus;               /* STDP positive amplitude */
    float stdp_a_minus;              /* STDP negative amplitude */
    
    /* Metaplasticity */
    float metaplasticity_factor;     /* Metaplasticity scaling */
    float activity_history;          /* Recent activity history */
    
    /* State variables */
    float current_efficacy;          /* Current synaptic efficacy */
    uint64_t last_activation;        /* Last activation time */
    uint32_t activation_count;       /* Activation count */
    
    /* Statistics */
    float average_weight_change;     /* Average weight change */
    float weight_variance;           /* Weight variance */
    uint64_t total_transmissions;    /* Total transmissions */
    
    pthread_mutex_t lock;            /* Thread safety */
} neural_synapse_t;

/* Neural population */
typedef struct {
    uint32_t population_id;          /* Unique population identifier */
    char name[64];                   /* Population name */
    uint32_t neuron_count;           /* Number of neurons */
    uint32_t *neuron_ids;            /* Array of neuron IDs */
    
    /* Population parameters */
    neuron_model_t default_model;    /* Default neuron model */
    neural_encoding_t encoding_scheme;/* Encoding scheme */
    
    /* Connectivity */
    float connection_probability;     /* Connection probability */
    float average_degree;            /* Average degree */
    uint32_t total_synapses;         /* Total synapses */
    
    /* Population dynamics */
    float population_rate;           /* Population firing rate */
    float synchrony_measure;         /* Synchrony measure */
    float burst_measure;             /* Burst measure */
    float oscillation_frequency;     /* Oscillation frequency */
    
    /* Adaptation and homeostasis */
    float target_rate;               /* Target firing rate */
    float homeostatic_gain;          /* Homeostatic gain */
    bool homeostasis_enabled;        /* Homeostasis enabled */
    
    /* Spatial organization */
    float position_x;                /* X position */
    float position_y;                /* Y position */
    float position_z;                /* Z position */
    float radius;                    /* Population radius */
    
    pthread_mutex_t lock;            /* Thread safety */
} neural_population_t;

/* Neural network structure */
typedef struct {
    uint32_t network_id;             /* Unique network identifier */
    char name[128];                  /* Network name */
    
    /* Network composition */
    uint32_t neuron_count;           /* Total neurons */
    uint32_t synapse_count;          /* Total synapses */
    uint32_t population_count;       /* Number of populations */
    uint32_t layer_count;            /* Number of layers */
    
    neural_neuron_t *neurons;        /* Array of neurons */
    neural_synapse_t *synapses;      /* Array of synapses */
    neural_population_t *populations;/* Array of populations */
    
    /* Network parameters */
    float simulation_timestep;       /* Simulation timestep (ms) */
    uint64_t simulation_time;        /* Current simulation time */
    bool real_time_simulation;       /* Real-time simulation flag */
    
    /* Learning configuration */
    learning_algorithm_t learning_algorithm;
    float global_learning_rate;      /* Global learning rate */
    bool learning_enabled;           /* Learning enabled */
    bool plasticity_enabled;         /* Plasticity enabled */
    
    /* Input/output */
    uint32_t input_neuron_count;     /* Input neurons */
    uint32_t output_neuron_count;    /* Output neurons */
    uint32_t *input_neurons;         /* Input neuron IDs */
    uint32_t *output_neurons;        /* Output neuron IDs */
    
    /* Network topology */
    bool feedforward_network;        /* Feedforward topology */
    bool recurrent_network;          /* Recurrent connections */
    bool small_world_network;        /* Small-world topology */
    bool scale_free_network;         /* Scale-free topology */
    
    /* Reservoir computing */
    bool is_reservoir;               /* Reservoir computing network */
    float reservoir_sparsity;        /* Reservoir sparsity */
    float spectral_radius;           /* Spectral radius */
    
    /* Statistics and monitoring */
    float network_activity;          /* Overall network activity */
    float synchrony_index;           /* Network synchrony */
    float complexity_measure;        /* Network complexity */
    uint64_t total_spikes;           /* Total spikes fired */
    
    pthread_mutex_t lock;            /* Thread safety */
} neural_network_t;

/* Neuromorphic core (hardware abstraction) */
typedef struct {
    uint32_t core_id;                /* Core identifier */
    neuromorphic_device_type_t device_type;
    
    /* Core capabilities */
    uint32_t max_neurons;            /* Maximum neurons per core */
    uint32_t max_synapses;           /* Maximum synapses per core */
    uint32_t current_neurons;        /* Current neuron count */
    uint32_t current_synapses;       /* Current synapse count */
    
    /* Hardware parameters */
    float clock_frequency;           /* Core clock frequency (MHz) */
    uint32_t memory_size;            /* Core memory (bytes) */
    bool supports_plasticity;        /* Plasticity support */
    bool supports_adaptation;        /* Adaptation support */
    bool supports_stochasticity;     /* Stochastic operations */
    
    /* Resource utilization */
    float cpu_utilization;           /* CPU utilization */
    float memory_utilization;        /* Memory utilization */
    float power_consumption;         /* Power consumption (mW) */
    float temperature;               /* Core temperature (°C) */
    
    /* Configuration */
    uint32_t assigned_network_id;    /* Assigned network */
    bool core_active;                /* Core active status */
    bool real_time_mode;             /* Real-time processing */
    
    pthread_mutex_t lock;            /* Thread safety */
} neuromorphic_core_t;

/* Neuromorphic device */
typedef struct {
    uint32_t device_id;              /* Device identifier */
    neuromorphic_device_type_t type; /* Device type */
    char name[128];                  /* Device name */
    char vendor[64];                 /* Vendor name */
    char version[32];                /* Hardware version */
    
    /* Device capabilities */
    uint32_t core_count;             /* Number of cores */
    uint32_t total_neurons;          /* Total neurons */
    uint32_t total_synapses;         /* Total synapses */
    neuromorphic_core_t *cores;      /* Array of cores */
    
    /* Device parameters */
    float max_frequency;             /* Maximum frequency (MHz) */
    uint32_t total_memory;           /* Total memory (bytes) */
    float max_power;                 /* Maximum power (W) */
    bool supports_learning;          /* Learning support */
    bool supports_inference;         /* Inference support */
    
    /* Communication interfaces */
    bool has_spi_interface;          /* SPI interface */
    bool has_i2c_interface;          /* I2C interface */
    bool has_pcie_interface;         /* PCIe interface */
    bool has_usb_interface;          /* USB interface */
    bool has_ethernet_interface;     /* Ethernet interface */
    
    /* Status */
    bool device_online;              /* Device online status */
    bool device_initialized;         /* Initialization status */
    float current_temperature;       /* Current temperature */
    float current_power;             /* Current power consumption */
    
    /* Statistics */
    uint64_t total_operations;       /* Total operations */
    uint64_t successful_operations;  /* Successful operations */
    uint64_t failed_operations;      /* Failed operations */
    double average_latency;          /* Average operation latency */
    
    pthread_mutex_t lock;            /* Thread safety */
} neuromorphic_device_t;

/* Spike train */
typedef struct {
    uint32_t train_id;               /* Spike train identifier */
    uint32_t neuron_count;           /* Number of neurons */
    uint32_t *neuron_ids;            /* Neuron IDs */
    
    /* Spike data */
    uint32_t max_spikes;             /* Maximum spikes */
    uint32_t spike_count;            /* Current spike count */
    neural_spike_t *spikes;          /* Spike array */
    
    /* Timing parameters */
    uint64_t start_time;             /* Start time */
    uint64_t end_time;               /* End time */
    uint64_t duration;               /* Duration */
    float average_rate;              /* Average firing rate */
    
    /* Analysis results */
    float isi_mean;                  /* Mean inter-spike interval */
    float isi_std;                   /* ISI standard deviation */
    float cv_isi;                    /* Coefficient of variation */
    float burst_index;               /* Burst index */
    float regularity_index;          /* Regularity index */
    
    pthread_mutex_t lock;            /* Thread safety */
} spike_train_t;

/* Learning rule */
typedef struct {
    uint32_t rule_id;                /* Rule identifier */
    char name[64];                   /* Rule name */
    synaptic_plasticity_t type;      /* Plasticity type */
    
    /* Rule parameters */
    float learning_rate;             /* Base learning rate */
    float decay_constant;            /* Decay constant */
    float threshold;                 /* Activation threshold */
    
    /* STDP parameters */
    float tau_plus;                  /* Positive time window */
    float tau_minus;                 /* Negative time window */
    float a_plus;                    /* Positive amplitude */
    float a_minus;                   /* Negative amplitude */
    
    /* Homeostatic parameters */
    float target_rate;               /* Target firing rate */
    float homeostatic_gain;          /* Homeostatic gain */
    float adaptation_rate;           /* Adaptation rate */
    
    /* Metaplasticity */
    float meta_learning_rate;        /* Meta-learning rate */
    float activity_threshold;        /* Activity threshold */
    
    /* Rule application */
    uint32_t synapse_count;          /* Number of synapses */
    uint32_t *synapse_ids;           /* Applied synapse IDs */
    bool rule_active;                /* Rule active status */
    
    pthread_mutex_t lock;            /* Thread safety */
} learning_rule_t;

/* Neuromorphic system */
typedef struct {
    /* System configuration */
    bool initialized;                /* System initialized */
    uint64_t start_time;             /* System start time */
    bool real_time_processing;       /* Real-time processing */
    
    /* Device management */
    uint32_t device_count;           /* Number of devices */
    neuromorphic_device_t devices[MAX_NEUROMORPHIC_DEVICES];
    
    /* Network management */
    uint32_t network_count;          /* Number of networks */
    neural_network_t networks[MAX_NEURAL_CORES];
    
    /* Spike processing */
    uint32_t spike_train_count;      /* Number of spike trains */
    spike_train_t spike_trains[MAX_SPIKE_TRAINS];
    
    /* Learning system */
    uint32_t learning_rule_count;    /* Number of learning rules */
    learning_rule_t learning_rules[MAX_LEARNING_RULES];
    
    /* System statistics */
    uint64_t total_neurons;          /* Total neurons in system */
    uint64_t total_synapses;         /* Total synapses in system */
    uint64_t total_spikes;           /* Total spikes processed */
    double average_spike_rate;       /* Average spike rate */
    
    /* Performance metrics */
    uint64_t processing_operations;  /* Processing operations */
    double average_latency;          /* Average processing latency */
    float system_utilization;        /* System utilization */
    float total_power_consumption;   /* Total power consumption */
    
    /* Threading */
    bool threads_running;            /* Threads running flag */
    pthread_t spike_processor_thread;/* Spike processing thread */
    pthread_t learning_thread;       /* Learning thread */
    pthread_t adaptation_thread;     /* Adaptation thread */
    pthread_mutex_t system_lock;     /* System-wide lock */
    
} neuromorphic_system_t;

/* Function prototypes */

/* System management */
int neuromorphic_system_init(void);
int neuromorphic_system_cleanup(void);
int neuromorphic_detect_devices(void);
int neuromorphic_device_init(uint32_t device_id);
int neuromorphic_device_cleanup(uint32_t device_id);

/* Network creation and management */
int neural_network_create(const char *name, uint32_t neuron_count, uint32_t synapse_count);
int neural_network_destroy(uint32_t network_id);
int neural_network_load_topology(uint32_t network_id, const char *topology_file);
int neural_network_save_topology(uint32_t network_id, const char *topology_file);
int neural_network_reset(uint32_t network_id);
int neural_network_configure_learning(uint32_t network_id, learning_algorithm_t algorithm, float learning_rate);

/* Neuron management */
int neural_neuron_create(uint32_t network_id, neuron_model_t model, float *parameters);
int neural_neuron_configure(uint32_t network_id, uint32_t neuron_id, neural_neuron_t *config);
int neural_neuron_set_parameters(uint32_t network_id, uint32_t neuron_id, float *parameters);
int neural_neuron_get_state(uint32_t network_id, uint32_t neuron_id, neural_neuron_t *state);
int neural_neuron_inject_current(uint32_t network_id, uint32_t neuron_id, float current, float duration);
int neural_neuron_generate_spike(uint32_t network_id, uint32_t neuron_id, uint64_t timestamp);

/* Synapse management */
int neural_synapse_create(uint32_t network_id, uint32_t pre_neuron, uint32_t post_neuron, float weight);
int neural_synapse_configure(uint32_t network_id, uint32_t synapse_id, neural_synapse_t *config);
int neural_synapse_set_weight(uint32_t network_id, uint32_t synapse_id, float weight);
int neural_synapse_set_plasticity(uint32_t network_id, uint32_t synapse_id, synaptic_plasticity_t type, float *parameters);
int neural_synapse_get_state(uint32_t network_id, uint32_t synapse_id, neural_synapse_t *state);

/* Population management */
int neural_population_create(uint32_t network_id, const char *name, uint32_t neuron_count, neuron_model_t model);
int neural_population_configure_connectivity(uint32_t network_id, uint32_t pop1_id, uint32_t pop2_id, float probability, float weight_mean, float weight_std);
int neural_population_set_encoding(uint32_t network_id, uint32_t population_id, neural_encoding_t encoding);
int neural_population_get_statistics(uint32_t network_id, uint32_t population_id, float *firing_rate, float *synchrony);

/* Simulation control */
int neural_simulation_start(uint32_t network_id, float duration);
int neural_simulation_stop(uint32_t network_id);
int neural_simulation_pause(uint32_t network_id);
int neural_simulation_resume(uint32_t network_id);
int neural_simulation_step(uint32_t network_id, float timestep);
int neural_simulation_set_timestep(uint32_t network_id, float timestep);

/* Spike processing */
int spike_train_create(uint32_t neuron_count, uint32_t *neuron_ids, uint64_t duration);
int spike_train_add_spike(uint32_t train_id, uint32_t neuron_id, uint64_t timestamp, float amplitude);
int spike_train_get_spikes(uint32_t train_id, uint64_t start_time, uint64_t end_time, neural_spike_t **spikes, uint32_t *count);
int spike_train_analyze(uint32_t train_id, float *firing_rate, float *isi_mean, float *cv_isi);
int spike_train_detect_bursts(uint32_t train_id, float threshold, uint32_t min_spikes);

/* Learning and adaptation */
int learning_rule_create(const char *name, synaptic_plasticity_t type, float *parameters);
int learning_rule_apply(uint32_t rule_id, uint32_t network_id, uint32_t *synapse_ids, uint32_t synapse_count);
int learning_update_weights(uint32_t network_id, uint32_t synapse_id, uint64_t pre_spike_time, uint64_t post_spike_time);
int homeostasis_enable(uint32_t network_id, uint32_t population_id, float target_rate, float gain);
int adaptation_configure(uint32_t network_id, float adaptation_rate, float target_activity);

/* Hardware interface */
int neuromorphic_hardware_configure(uint32_t device_id, uint32_t core_id, neural_network_t *network);
int neuromorphic_hardware_upload_network(uint32_t device_id, uint32_t network_id);
int neuromorphic_hardware_run_inference(uint32_t device_id, float *input, float *output, uint32_t input_size, uint32_t output_size);
int neuromorphic_hardware_run_learning(uint32_t device_id, uint32_t network_id, float *training_data, uint32_t epochs);

/* Analysis and visualization */
int neural_analysis_connectivity(uint32_t network_id, float *clustering_coefficient, float *path_length, float *small_worldness);
int neural_analysis_dynamics(uint32_t network_id, float *activity_measure, float *synchrony_index, float *complexity);
int neural_analysis_information_flow(uint32_t network_id, uint32_t source_pop, uint32_t target_pop, float *mutual_information);
int neural_visualization_raster_plot(uint32_t network_id, uint64_t start_time, uint64_t end_time, const char *output_file);
int neural_visualization_network_graph(uint32_t network_id, const char *output_file);

/* Utility functions */
const char *neuromorphic_device_type_name(neuromorphic_device_type_t type);
const char *neuron_model_name(neuron_model_t model);
const char *synaptic_plasticity_name(synaptic_plasticity_t plasticity);
const char *learning_algorithm_name(learning_algorithm_t algorithm);
const char *neural_encoding_name(neural_encoding_t encoding);
double neuromorphic_get_timestamp_seconds(void);
uint64_t neuromorphic_get_timestamp_nanoseconds(void);
float neural_random_gaussian(float mean, float std);
float neural_random_uniform(float min, float max);
int neural_random_poisson(float rate, float duration);

#endif /* NEUROMORPHIC_COMPUTING_H */