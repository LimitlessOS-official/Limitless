/*
 * LimitlessOS Blockchain & Distributed Systems Header
 * Blockchain integration, smart contracts, decentralized identity, and crypto-currency support
 */

#ifndef BLOCKCHAIN_DISTRIBUTED_H
#define BLOCKCHAIN_DISTRIBUTED_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/ec.h>

/* Maximum limits */
#define MAX_BLOCKCHAINS 20
#define MAX_BLOCKS_PER_CHAIN 1000000
#define MAX_TRANSACTIONS_PER_BLOCK 10000
#define MAX_SMART_CONTRACTS 5000
#define MAX_WALLETS 10000
#define MAX_PEERS 1000
#define MAX_CONSENSUS_NODES 100
#define MAX_TOKENS 1000
#define MAX_IDENTITIES 50000

/* Hash sizes */
#define HASH_SIZE 32
#define ADDRESS_SIZE 20
#define SIGNATURE_SIZE 64
#define PUBLIC_KEY_SIZE 33
#define PRIVATE_KEY_SIZE 32

/* Blockchain types */
typedef enum {
    BLOCKCHAIN_BITCOIN = 0,
    BLOCKCHAIN_ETHEREUM,
    BLOCKCHAIN_LIMITLESS,
    BLOCKCHAIN_PRIVATE,
    BLOCKCHAIN_CONSORTIUM,
    BLOCKCHAIN_HYBRID,
    BLOCKCHAIN_MAX
} blockchain_type_t;

/* Consensus algorithms */
typedef enum {
    CONSENSUS_PROOF_OF_WORK = 0,
    CONSENSUS_PROOF_OF_STAKE,
    CONSENSUS_DELEGATED_PROOF_OF_STAKE,
    CONSENSUS_PRACTICAL_BYZANTINE_FAULT_TOLERANCE,
    CONSENSUS_RAFT,
    CONSENSUS_PROOF_OF_AUTHORITY,
    CONSENSUS_PROOF_OF_SPACE,
    CONSENSUS_PROOF_OF_HISTORY,
    CONSENSUS_MAX
} consensus_algorithm_t;

/* Smart contract languages */
typedef enum {
    CONTRACT_SOLIDITY = 0,
    CONTRACT_VYPER,
    CONTRACT_RUST,
    CONTRACT_WASM,
    CONTRACT_JAVASCRIPT,
    CONTRACT_PYTHON,
    CONTRACT_GO,
    CONTRACT_LIMITLESS_SCRIPT,
    CONTRACT_MAX
} contract_language_t;

/* Transaction types */
typedef enum {
    TX_TRANSFER = 0,
    TX_CONTRACT_CREATION,
    TX_CONTRACT_CALL,
    TX_MULTISIG,
    TX_ATOMIC_SWAP,
    TX_TOKEN_MINT,
    TX_TOKEN_BURN,
    TX_IDENTITY_REGISTRATION,
    TX_IDENTITY_UPDATE,
    TX_GOVERNANCE_VOTE,
    TX_MAX
} transaction_type_t;

/* Token standards */
typedef enum {
    TOKEN_ERC20 = 0,
    TOKEN_ERC721,
    TOKEN_ERC1155,
    TOKEN_BEP20,
    TOKEN_NATIVE,
    TOKEN_CUSTOM,
    TOKEN_MAX
} token_standard_t;

/* Identity verification levels */
typedef enum {
    IDENTITY_UNVERIFIED = 0,
    IDENTITY_BASIC,
    IDENTITY_ENHANCED,
    IDENTITY_FULL_KYC,
    IDENTITY_GOVERNMENT_ISSUED,
    IDENTITY_MAX
} identity_verification_t;

/* Block header */
typedef struct {
    uint32_t version;
    uint8_t previous_hash[HASH_SIZE];
    uint8_t merkle_root[HASH_SIZE];
    uint32_t timestamp;
    uint32_t difficulty;
    uint32_t nonce;
    uint64_t block_number;
    uint8_t hash[HASH_SIZE];
} block_header_t;

/* Transaction input */
typedef struct {
    uint8_t previous_tx_hash[HASH_SIZE];
    uint32_t output_index;
    uint8_t signature[SIGNATURE_SIZE];
    uint8_t public_key[PUBLIC_KEY_SIZE];
    uint32_t sequence;
} transaction_input_t;

/* Transaction output */
typedef struct {
    uint64_t value;
    uint8_t recipient_address[ADDRESS_SIZE];
    uint32_t script_length;
    uint8_t *script_data;
    bool spent;
    uint32_t spending_tx_index;
} transaction_output_t;

/* Smart contract */
typedef struct {
    uint8_t contract_address[ADDRESS_SIZE];
    char name[128];
    contract_language_t language;
    uint8_t creator_address[ADDRESS_SIZE];
    
    /* Contract code */
    uint32_t bytecode_size;
    uint8_t *bytecode;
    uint32_t source_size;
    char *source_code;
    
    /* Contract state */
    uint64_t storage_size;
    uint8_t *storage_data;
    uint64_t balance;
    
    /* Execution limits */
    uint64_t gas_limit;
    uint64_t gas_used;
    uint32_t call_depth;
    
    /* Metadata */
    time_t created;
    uint64_t creation_block;
    uint32_t version;
    bool active;
    
    pthread_mutex_t lock;
} smart_contract_t;

/* Transaction */
typedef struct {
    uint8_t tx_hash[HASH_SIZE];
    transaction_type_t type;
    uint32_t version;
    
    /* Inputs and outputs */
    transaction_input_t *inputs;
    uint32_t input_count;
    transaction_output_t *outputs;
    uint32_t output_count;
    
    /* Contract interaction */
    uint8_t contract_address[ADDRESS_SIZE];
    uint32_t function_selector;
    uint32_t call_data_size;
    uint8_t *call_data;
    
    /* Gas and fees */
    uint64_t gas_limit;
    uint64_t gas_price;
    uint64_t gas_used;
    uint64_t transaction_fee;
    
    /* Timing */
    uint32_t timestamp;
    uint32_t block_number;
    uint32_t transaction_index;
    
    /* Status */
    bool confirmed;
    bool executed;
    bool failed;
    char error_message[256];
    
    pthread_mutex_t lock;
} transaction_t;

/* Block */
typedef struct {
    block_header_t header;
    
    /* Transactions */
    transaction_t *transactions;
    uint32_t transaction_count;
    
    /* Metadata */
    uint64_t total_fees;
    uint64_t total_gas_used;
    uint32_t size_bytes;
    
    /* Mining info */
    uint8_t miner_address[ADDRESS_SIZE];
    uint64_t block_reward;
    uint32_t mining_time;
    
    /* Validation */
    bool validated;
    bool finalized;
    uint32_t confirmation_count;
    
    pthread_mutex_t lock;
} block_t;

/* Wallet */
typedef struct {
    uint32_t wallet_id;
    char name[128];
    
    /* Cryptographic keys */
    uint8_t public_key[PUBLIC_KEY_SIZE];
    uint8_t private_key[PRIVATE_KEY_SIZE]; /* Encrypted in storage */
    uint8_t address[ADDRESS_SIZE];
    
    /* Balances */
    uint64_t native_balance;
    uint64_t *token_balances;
    uint32_t token_count;
    
    /* Transaction history */
    uint8_t **transaction_hashes;
    uint32_t transaction_count;
    
    /* Security */
    bool encrypted;
    uint8_t encryption_key[32];
    bool hardware_wallet;
    bool multisig;
    uint32_t multisig_threshold;
    uint8_t multisig_addresses[10][ADDRESS_SIZE];
    uint32_t multisig_count;
    
    /* Metadata */
    time_t created;
    time_t last_used;
    bool active;
    
    pthread_mutex_t lock;
} wallet_t;

/* Blockchain network peer */
typedef struct {
    uint32_t peer_id;
    char ip_address[46]; /* IPv6 compatible */
    uint16_t port;
    char node_id[64];
    
    /* Capabilities */
    bool full_node;
    bool mining_node;
    bool validator_node;
    uint32_t protocol_version;
    
    /* Connection status */
    bool connected;
    time_t last_seen;
    time_t connection_time;
    uint32_t latency_ms;
    
    /* Statistics */
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint32_t blocks_shared;
    uint32_t transactions_shared;
    
    /* Reputation */
    double trust_score; /* 0.0 - 1.0 */
    uint32_t successful_interactions;
    uint32_t failed_interactions;
    
    pthread_mutex_t lock;
} blockchain_peer_t;

/* Consensus node */
typedef struct {
    uint32_t node_id;
    uint8_t validator_address[ADDRESS_SIZE];
    uint8_t public_key[PUBLIC_KEY_SIZE];
    
    /* Staking information */
    uint64_t stake_amount;
    uint64_t delegated_stake;
    double voting_power;
    
    /* Performance metrics */
    uint32_t blocks_produced;
    uint32_t blocks_missed;
    double uptime_percentage;
    uint32_t slash_count;
    
    /* Status */
    bool active;
    bool jailed;
    time_t jail_until;
    
    pthread_mutex_t lock;
} consensus_node_t;

/* Token definition */
typedef struct {
    uint32_t token_id;
    char name[64];
    char symbol[16];
    token_standard_t standard;
    
    /* Token properties */
    uint8_t decimals;
    uint64_t total_supply;
    uint64_t max_supply;
    bool mintable;
    bool burnable;
    bool pausable;
    
    /* Contract information */
    uint8_t contract_address[ADDRESS_SIZE];
    uint32_t deployment_block;
    
    /* Economics */
    uint64_t market_cap;
    uint64_t circulating_supply;
    uint64_t locked_supply;
    
    /* Metadata */
    char description[512];
    char website[256];
    char logo_url[256];
    
    bool active;
    time_t created;
    
    pthread_mutex_t lock;
} token_t;

/* Decentralized identity */
typedef struct {
    uint32_t identity_id;
    uint8_t did[64]; /* Decentralized Identifier */
    
    /* Identity information */
    char name[128];
    char email[256];
    char organization[128];
    identity_verification_t verification_level;
    
    /* Cryptographic proof */
    uint8_t public_key[PUBLIC_KEY_SIZE];
    uint8_t identity_hash[HASH_SIZE];
    
    /* Credentials */
    uint8_t **credential_hashes;
    uint32_t credential_count;
    
    /* Attestations */
    uint8_t **attestation_signatures;
    uint8_t **attestor_addresses;
    uint32_t attestation_count;
    
    /* Privacy settings */
    bool public_profile;
    bool kyc_verified;
    bool government_verified;
    
    /* Blockchain anchoring */
    uint8_t anchor_tx_hash[HASH_SIZE];
    uint32_t anchor_block;
    
    /* Status */
    bool active;
    bool revoked;
    time_t created;
    time_t last_updated;
    
    pthread_mutex_t lock;
} decentralized_identity_t;

/* Blockchain configuration */
typedef struct {
    blockchain_type_t type;
    consensus_algorithm_t consensus;
    
    /* Network parameters */
    uint32_t network_id;
    uint16_t default_port;
    uint32_t block_time_seconds;
    uint32_t max_block_size;
    uint32_t max_transactions_per_block;
    
    /* Economic parameters */
    uint64_t block_reward;
    uint64_t min_transaction_fee;
    uint64_t gas_limit_per_block;
    uint64_t gas_price;
    
    /* Consensus parameters */
    uint32_t confirmation_blocks;
    uint32_t difficulty_adjustment_blocks;
    double difficulty_adjustment_factor;
    
    /* Smart contract settings */
    bool contracts_enabled;
    uint64_t contract_creation_fee;
    uint64_t max_contract_size;
    
    /* Security settings */
    bool require_signature_verification;
    bool enable_multisig;
    uint32_t max_multisig_signers;
    
} blockchain_config_t;

/* Main blockchain system */
typedef struct {
    /* Blockchains */
    block_t **blockchain_data; /* Array of blockchain arrays */
    uint32_t *blockchain_lengths;
    blockchain_config_t *blockchain_configs;
    uint32_t blockchain_count;
    
    /* Smart contracts */
    smart_contract_t contracts[MAX_SMART_CONTRACTS];
    uint32_t contract_count;
    
    /* Wallets */
    wallet_t wallets[MAX_WALLETS];
    uint32_t wallet_count;
    
    /* Network */
    blockchain_peer_t peers[MAX_PEERS];
    uint32_t peer_count;
    
    /* Consensus */
    consensus_node_t consensus_nodes[MAX_CONSENSUS_NODES];
    uint32_t consensus_node_count;
    uint32_t local_node_id;
    
    /* Tokens */
    token_t tokens[MAX_TOKENS];
    uint32_t token_count;
    
    /* Identities */
    decentralized_identity_t identities[MAX_IDENTITIES];
    uint32_t identity_count;
    
    /* Mining/Validation */
    bool mining_enabled;
    bool validation_enabled;
    uint32_t mining_threads;
    uint64_t hashrate;
    
    /* Threading */
    pthread_t mining_threads_handles[16];
    pthread_t validation_thread;
    pthread_t network_thread;
    pthread_t consensus_thread;
    bool threads_running;
    
    /* Synchronization */
    pthread_mutex_t system_lock;
    pthread_cond_t new_block_available;
    pthread_cond_t new_transaction_available;
    
    /* Statistics */
    uint64_t total_blocks;
    uint64_t total_transactions;
    uint64_t total_contracts_deployed;
    uint64_t total_gas_consumed;
    time_t start_time;
    
    bool initialized;
} blockchain_system_t;

/* Core functions */
int blockchain_system_init(void);
int blockchain_system_cleanup(void);
int blockchain_create_network(blockchain_type_t type, consensus_algorithm_t consensus);

/* Block operations */
int block_create(uint32_t blockchain_id, block_t *block);
int block_add_transaction(uint32_t blockchain_id, uint32_t block_index, const transaction_t *transaction);
int block_validate(uint32_t blockchain_id, uint32_t block_index);
int block_mine(uint32_t blockchain_id, uint32_t block_index, uint8_t miner_address[ADDRESS_SIZE]);
int block_get_by_hash(uint32_t blockchain_id, const uint8_t hash[HASH_SIZE], block_t *block);

/* Transaction operations */
int transaction_create(transaction_type_t type, transaction_t *transaction);
int transaction_add_input(transaction_t *transaction, const uint8_t prev_hash[HASH_SIZE], uint32_t output_index);
int transaction_add_output(transaction_t *transaction, uint64_t value, const uint8_t recipient[ADDRESS_SIZE]);
int transaction_sign(transaction_t *transaction, const uint8_t private_key[PRIVATE_KEY_SIZE]);
int transaction_verify(const transaction_t *transaction);
int transaction_broadcast(uint32_t blockchain_id, const transaction_t *transaction);

/* Smart contract operations */
int contract_deploy(uint32_t blockchain_id, const char *source_code, contract_language_t language, const uint8_t creator[ADDRESS_SIZE]);
int contract_call(uint32_t blockchain_id, const uint8_t contract_address[ADDRESS_SIZE], const char *function_name, const uint8_t *call_data, uint32_t data_size);
int contract_get_storage(const uint8_t contract_address[ADDRESS_SIZE], uint64_t key, uint8_t *value);
int contract_set_storage(const uint8_t contract_address[ADDRESS_SIZE], uint64_t key, const uint8_t *value);

/* Wallet operations */
int wallet_create(const char *name, wallet_t *wallet);
int wallet_import_private_key(const uint8_t private_key[PRIVATE_KEY_SIZE], wallet_t *wallet);
int wallet_get_balance(uint32_t wallet_id, uint32_t blockchain_id, uint64_t *balance);
int wallet_send_transaction(uint32_t wallet_id, uint32_t blockchain_id, const uint8_t recipient[ADDRESS_SIZE], uint64_t amount);
int wallet_create_multisig(uint8_t addresses[][ADDRESS_SIZE], uint32_t address_count, uint32_t threshold, wallet_t *multisig_wallet);

/* Token operations */
int token_create(const char *name, const char *symbol, uint8_t decimals, uint64_t total_supply, token_standard_t standard);
int token_mint(uint32_t token_id, const uint8_t recipient[ADDRESS_SIZE], uint64_t amount);
int token_burn(uint32_t token_id, const uint8_t holder[ADDRESS_SIZE], uint64_t amount);
int token_transfer(uint32_t token_id, const uint8_t from[ADDRESS_SIZE], const uint8_t to[ADDRESS_SIZE], uint64_t amount);
int token_get_balance(uint32_t token_id, const uint8_t address[ADDRESS_SIZE], uint64_t *balance);

/* Identity operations */
int identity_create(const char *name, const char *email, decentralized_identity_t *identity);
int identity_add_credential(uint32_t identity_id, const uint8_t credential_hash[HASH_SIZE]);
int identity_add_attestation(uint32_t identity_id, const uint8_t attestor[ADDRESS_SIZE], const uint8_t signature[SIGNATURE_SIZE]);
int identity_verify_kyc(uint32_t identity_id, identity_verification_t level);
int identity_resolve_did(const uint8_t did[64], decentralized_identity_t *identity);

/* Network operations */
int network_add_peer(const char *ip_address, uint16_t port);
int network_remove_peer(uint32_t peer_id);
int network_broadcast_block(uint32_t blockchain_id, const block_t *block);
int network_broadcast_transaction(uint32_t blockchain_id, const transaction_t *transaction);
int network_sync_blockchain(uint32_t blockchain_id);

/* Consensus operations */
int consensus_add_validator(const uint8_t validator_address[ADDRESS_SIZE], uint64_t stake);
int consensus_remove_validator(uint32_t node_id);
int consensus_vote_block(uint32_t blockchain_id, uint32_t block_index, bool approve);
int consensus_finalize_block(uint32_t blockchain_id, uint32_t block_index);

/* Mining operations */
int mining_start(uint32_t blockchain_id, uint8_t miner_address[ADDRESS_SIZE]);
int mining_stop(void);
int mining_set_threads(uint32_t thread_count);
int mining_get_stats(uint64_t *hashrate, uint32_t *blocks_mined);

/* Cryptographic functions */
int crypto_generate_keypair(uint8_t public_key[PUBLIC_KEY_SIZE], uint8_t private_key[PRIVATE_KEY_SIZE]);
int crypto_sign_data(const uint8_t *data, uint32_t data_size, const uint8_t private_key[PRIVATE_KEY_SIZE], uint8_t signature[SIGNATURE_SIZE]);
int crypto_verify_signature(const uint8_t *data, uint32_t data_size, const uint8_t signature[SIGNATURE_SIZE], const uint8_t public_key[PUBLIC_KEY_SIZE]);
int crypto_hash_data(const uint8_t *data, uint32_t data_size, uint8_t hash[HASH_SIZE]);
int crypto_derive_address(const uint8_t public_key[PUBLIC_KEY_SIZE], uint8_t address[ADDRESS_SIZE]);

/* Utility functions */
const char *blockchain_type_name(blockchain_type_t type);
const char *consensus_algorithm_name(consensus_algorithm_t consensus);
const char *transaction_type_name(transaction_type_t type);
const char *token_standard_name(token_standard_t standard);
const char *contract_language_name(contract_language_t language);
void hash_to_string(const uint8_t hash[HASH_SIZE], char *str);
void address_to_string(const uint8_t address[ADDRESS_SIZE], char *str);

#endif /* BLOCKCHAIN_DISTRIBUTED_H */