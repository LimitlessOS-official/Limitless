/*
 * LimitlessOS Blockchain & Distributed Systems Implementation
 * Blockchain integration, smart contracts, decentralized identity, and crypto-currency support
 */

#include "blockchain_distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ecdsa.h>
#include <json-c/json.h>

/* Global blockchain system */
static blockchain_system_t blockchain_system = {0};

/* Thread functions */
static void *mining_thread_func(void *arg);
static void *validation_thread_func(void *arg);
static void *network_thread_func(void *arg);
static void *consensus_thread_func(void *arg);

/* Helper functions */
static int initialize_default_blockchain(void);
static int validate_transaction_inputs(const transaction_t *transaction);
static int validate_transaction_outputs(const transaction_t *transaction);
static int calculate_merkle_root(const transaction_t *transactions, uint32_t count, uint8_t root[HASH_SIZE]);
static int mine_block_pow(block_t *block, uint32_t difficulty);
static int execute_smart_contract(const smart_contract_t *contract, const uint8_t *call_data, uint32_t data_size);
static double calculate_peer_trust_score(const blockchain_peer_t *peer);

/* Initialize blockchain system */
int blockchain_system_init(void) {
    printf("Initializing Blockchain & Distributed Systems...\n");
    
    memset(&blockchain_system, 0, sizeof(blockchain_system_t));
    pthread_mutex_init(&blockchain_system.system_lock, NULL);
    pthread_cond_init(&blockchain_system.new_block_available, NULL);
    pthread_cond_init(&blockchain_system.new_transaction_available, NULL);
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Allocate blockchain storage */
    blockchain_system.blockchain_data = calloc(MAX_BLOCKCHAINS, sizeof(block_t*));
    blockchain_system.blockchain_lengths = calloc(MAX_BLOCKCHAINS, sizeof(uint32_t));
    blockchain_system.blockchain_configs = calloc(MAX_BLOCKCHAINS, sizeof(blockchain_config_t));
    
    if (!blockchain_system.blockchain_data || !blockchain_system.blockchain_lengths || 
        !blockchain_system.blockchain_configs) {
        printf("Failed to allocate blockchain storage\n");
        return -ENOMEM;
    }
    
    /* Initialize default blockchain */
    if (initialize_default_blockchain() != 0) {
        printf("Failed to initialize default blockchain\n");
        return -1;
    }
    
    /* Mining and validation settings */
    blockchain_system.mining_enabled = false;
    blockchain_system.validation_enabled = true;
    blockchain_system.mining_threads = 4;
    blockchain_system.hashrate = 0;
    
    /* Start background threads */
    blockchain_system.threads_running = true;
    blockchain_system.start_time = time(NULL);
    
    pthread_create(&blockchain_system.validation_thread, NULL, validation_thread_func, NULL);
    pthread_create(&blockchain_system.network_thread, NULL, network_thread_func, NULL);
    pthread_create(&blockchain_system.consensus_thread, NULL, consensus_thread_func, NULL);
    
    blockchain_system.initialized = true;
    
    printf("Blockchain system initialized successfully\n");
    printf("- Blockchains: %u\n", blockchain_system.blockchain_count);
    printf("- Mining: %s\n", blockchain_system.mining_enabled ? "Enabled" : "Disabled");
    printf("- Validation: %s\n", blockchain_system.validation_enabled ? "Enabled" : "Disabled");
    printf("- Smart contracts: Enabled\n");
    printf("- Decentralized identity: Enabled\n");
    
    return 0;
}

/* Cleanup blockchain system */
int blockchain_system_cleanup(void) {
    if (!blockchain_system.initialized) return 0;
    
    printf("Shutting down blockchain system...\n");
    
    /* Stop threads */
    blockchain_system.threads_running = false;
    pthread_cond_broadcast(&blockchain_system.new_block_available);
    pthread_cond_broadcast(&blockchain_system.new_transaction_available);
    
    pthread_join(blockchain_system.validation_thread, NULL);
    pthread_join(blockchain_system.network_thread, NULL);
    pthread_join(blockchain_system.consensus_thread, NULL);
    
    /* Stop mining if running */
    if (blockchain_system.mining_enabled) {
        mining_stop();
    }
    
    /* Cleanup blockchain data */
    for (uint32_t i = 0; i < blockchain_system.blockchain_count; i++) {
        if (blockchain_system.blockchain_data[i]) {
            free(blockchain_system.blockchain_data[i]);
        }
    }
    
    free(blockchain_system.blockchain_data);
    free(blockchain_system.blockchain_lengths);
    free(blockchain_system.blockchain_configs);
    
    /* Cleanup contracts */
    for (uint32_t i = 0; i < blockchain_system.contract_count; i++) {
        smart_contract_t *contract = &blockchain_system.contracts[i];
        if (contract->bytecode) free(contract->bytecode);
        if (contract->source_code) free(contract->source_code);
        if (contract->storage_data) free(contract->storage_data);
        pthread_mutex_destroy(&contract->lock);
    }
    
    pthread_mutex_destroy(&blockchain_system.system_lock);
    pthread_cond_destroy(&blockchain_system.new_block_available);
    pthread_cond_destroy(&blockchain_system.new_transaction_available);
    
    blockchain_system.initialized = false;
    
    printf("Blockchain system shutdown complete\n");
    
    return 0;
}

/* Initialize default LimitlessOS blockchain */
static int initialize_default_blockchain(void) {
    blockchain_config_t *config = &blockchain_system.blockchain_configs[0];
    
    config->type = BLOCKCHAIN_LIMITLESS;
    config->consensus = CONSENSUS_PROOF_OF_STAKE;
    config->network_id = 1;
    config->default_port = 8545;
    config->block_time_seconds = 12;
    config->max_block_size = 2 * 1024 * 1024; /* 2MB */
    config->max_transactions_per_block = 5000;
    config->block_reward = 2 * 1000000000000000000ULL; /* 2 LimitlessCoins */
    config->min_transaction_fee = 21000 * 1000000000ULL; /* 21k gas at 1 Gwei */
    config->gas_limit_per_block = 15000000;
    config->gas_price = 1000000000ULL; /* 1 Gwei */
    config->confirmation_blocks = 6;
    config->difficulty_adjustment_blocks = 2016;
    config->difficulty_adjustment_factor = 4.0;
    config->contracts_enabled = true;
    config->contract_creation_fee = 32000 * 1000000000ULL;
    config->max_contract_size = 24576; /* 24KB */
    config->require_signature_verification = true;
    config->enable_multisig = true;
    config->max_multisig_signers = 15;
    
    /* Allocate initial blockchain storage */
    blockchain_system.blockchain_data[0] = calloc(MAX_BLOCKS_PER_CHAIN, sizeof(block_t));
    if (!blockchain_system.blockchain_data[0]) {
        return -ENOMEM;
    }
    
    /* Create genesis block */
    block_t *genesis = &blockchain_system.blockchain_data[0][0];
    genesis->header.version = 1;
    memset(genesis->header.previous_hash, 0, HASH_SIZE);
    genesis->header.timestamp = (uint32_t)time(NULL);
    genesis->header.difficulty = 1;
    genesis->header.nonce = 0;
    genesis->header.block_number = 0;
    
    /* Genesis transaction (coinbase) */
    genesis->transactions = calloc(1, sizeof(transaction_t));
    if (!genesis->transactions) {
        return -ENOMEM;
    }
    
    transaction_t *coinbase = &genesis->transactions[0];
    coinbase->type = TX_TRANSFER;
    coinbase->version = 1;
    coinbase->input_count = 0;
    coinbase->output_count = 1;
    coinbase->outputs = calloc(1, sizeof(transaction_output_t));
    coinbase->outputs[0].value = config->block_reward;
    
    /* Generate a default miner address */
    uint8_t genesis_miner[ADDRESS_SIZE];
    memset(genesis_miner, 0x42, ADDRESS_SIZE); /* Placeholder address */
    memcpy(coinbase->outputs[0].recipient_address, genesis_miner, ADDRESS_SIZE);
    memcpy(genesis->miner_address, genesis_miner, ADDRESS_SIZE);
    
    genesis->transaction_count = 1;
    genesis->block_reward = config->block_reward;
    genesis->validated = true;
    genesis->finalized = true;
    
    /* Calculate genesis block hash */
    crypto_hash_data((uint8_t*)&genesis->header, sizeof(block_header_t) - HASH_SIZE, genesis->header.hash);
    
    blockchain_system.blockchain_lengths[0] = 1;
    blockchain_system.blockchain_count = 1;
    blockchain_system.total_blocks = 1;
    
    printf("Created genesis block for LimitlessOS blockchain\n");
    
    return 0;
}

/* Create blockchain network */
int blockchain_create_network(blockchain_type_t type, consensus_algorithm_t consensus) {
    if (blockchain_system.blockchain_count >= MAX_BLOCKCHAINS) {
        return -ENOSPC;
    }
    
    uint32_t blockchain_id = blockchain_system.blockchain_count;
    blockchain_config_t *config = &blockchain_system.blockchain_configs[blockchain_id];
    
    config->type = type;
    config->consensus = consensus;
    config->network_id = blockchain_id + 1;
    
    /* Set default parameters based on type */
    switch (type) {
        case BLOCKCHAIN_BITCOIN:
            config->block_time_seconds = 600; /* 10 minutes */
            config->max_block_size = 1024 * 1024; /* 1MB */
            config->consensus = CONSENSUS_PROOF_OF_WORK;
            break;
            
        case BLOCKCHAIN_ETHEREUM:
            config->block_time_seconds = 15;
            config->max_block_size = 128 * 1024; /* 128KB */
            config->consensus = CONSENSUS_PROOF_OF_STAKE;
            config->contracts_enabled = true;
            break;
            
        case BLOCKCHAIN_LIMITLESS:
            config->block_time_seconds = 12;
            config->max_block_size = 2 * 1024 * 1024; /* 2MB */
            config->consensus = CONSENSUS_PROOF_OF_STAKE;
            config->contracts_enabled = true;
            break;
            
        default:
            config->block_time_seconds = 30;
            config->max_block_size = 512 * 1024; /* 512KB */
            break;
    }
    
    /* Allocate blockchain storage */
    blockchain_system.blockchain_data[blockchain_id] = calloc(MAX_BLOCKS_PER_CHAIN, sizeof(block_t));
    if (!blockchain_system.blockchain_data[blockchain_id]) {
        return -ENOMEM;
    }
    
    blockchain_system.blockchain_lengths[blockchain_id] = 0;
    blockchain_system.blockchain_count++;
    
    printf("Created %s blockchain (ID: %u) with %s consensus\n",
           blockchain_type_name(type), blockchain_id, consensus_algorithm_name(consensus));
    
    return blockchain_id;
}

/* Create wallet */
int wallet_create(const char *name, wallet_t *wallet) {
    if (!name || !wallet || blockchain_system.wallet_count >= MAX_WALLETS) {
        return -EINVAL;
    }
    
    wallet->wallet_id = blockchain_system.wallet_count;
    strncpy(wallet->name, name, sizeof(wallet->name) - 1);
    
    /* Generate cryptographic keys */
    if (crypto_generate_keypair(wallet->public_key, wallet->private_key) != 0) {
        printf("Failed to generate keypair for wallet\n");
        return -1;
    }
    
    /* Derive address from public key */
    if (crypto_derive_address(wallet->public_key, wallet->address) != 0) {
        printf("Failed to derive address for wallet\n");
        return -1;
    }
    
    /* Initialize balances */
    wallet->native_balance = 0;
    wallet->token_balances = NULL;
    wallet->token_count = 0;
    
    /* Initialize transaction history */
    wallet->transaction_hashes = NULL;
    wallet->transaction_count = 0;
    
    /* Security settings */
    wallet->encrypted = false;
    wallet->hardware_wallet = false;
    wallet->multisig = false;
    wallet->multisig_threshold = 0;
    wallet->multisig_count = 0;
    
    /* Metadata */
    wallet->created = time(NULL);
    wallet->last_used = wallet->created;
    wallet->active = true;
    
    pthread_mutex_init(&wallet->lock, NULL);
    
    blockchain_system.wallet_count++;
    
    char address_str[41];
    address_to_string(wallet->address, address_str);
    printf("Created wallet '%s' with address: %s\n", name, address_str);
    
    return wallet->wallet_id;
}

/* Deploy smart contract */
int contract_deploy(uint32_t blockchain_id, const char *source_code, 
                   contract_language_t language, const uint8_t creator[ADDRESS_SIZE]) {
    if (blockchain_id >= blockchain_system.blockchain_count || !source_code || !creator ||
        blockchain_system.contract_count >= MAX_SMART_CONTRACTS) {
        return -EINVAL;
    }
    
    blockchain_config_t *config = &blockchain_system.blockchain_configs[blockchain_id];
    if (!config->contracts_enabled) {
        return -ENOTSUP;
    }
    
    smart_contract_t *contract = &blockchain_system.contracts[blockchain_system.contract_count];
    
    contract->contract_address[0] = 0xCA; /* Contract Address prefix */
    
    /* Generate contract address from creator + nonce */
    uint8_t creation_data[ADDRESS_SIZE + 8];
    memcpy(creation_data, creator, ADDRESS_SIZE);
    uint64_t nonce = blockchain_system.contract_count; /* Simplified nonce */
    memcpy(creation_data + ADDRESS_SIZE, &nonce, 8);
    
    crypto_hash_data(creation_data, sizeof(creation_data), contract->contract_address);
    
    strcpy(contract->name, "Smart Contract");
    contract->language = language;
    memcpy(contract->creator_address, creator, ADDRESS_SIZE);
    
    /* Store source code */
    contract->source_size = strlen(source_code) + 1;
    contract->source_code = malloc(contract->source_size);
    if (!contract->source_code) {
        return -ENOMEM;
    }
    strcpy(contract->source_code, source_code);
    
    /* Compile to bytecode (simplified) */
    contract->bytecode_size = contract->source_size; /* 1:1 for demo */
    contract->bytecode = malloc(contract->bytecode_size);
    if (!contract->bytecode) {
        free(contract->source_code);
        return -ENOMEM;
    }
    memcpy(contract->bytecode, source_code, contract->source_size);
    
    /* Initialize contract storage */
    contract->storage_size = 1024; /* 1KB initial storage */
    contract->storage_data = calloc(contract->storage_size, 1);
    if (!contract->storage_data) {
        free(contract->source_code);
        free(contract->bytecode);
        return -ENOMEM;
    }
    
    /* Initialize state */
    contract->balance = 0;
    contract->gas_limit = 1000000;
    contract->gas_used = 0;
    contract->call_depth = 0;
    contract->created = time(NULL);
    contract->creation_block = blockchain_system.blockchain_lengths[blockchain_id];
    contract->version = 1;
    contract->active = true;
    
    pthread_mutex_init(&contract->lock, NULL);
    
    blockchain_system.contract_count++;
    blockchain_system.total_contracts_deployed++;
    
    char address_str[41];
    address_to_string(contract->contract_address, address_str);
    printf("Deployed smart contract (%s) at address: %s\n", 
           contract_language_name(language), address_str);
    
    return blockchain_system.contract_count - 1;
}

/* Create transaction */
int transaction_create(transaction_type_t type, transaction_t *transaction) {
    if (!transaction) {
        return -EINVAL;
    }
    
    memset(transaction, 0, sizeof(transaction_t));
    
    transaction->type = type;
    transaction->version = 1;
    transaction->timestamp = (uint32_t)time(NULL);
    transaction->gas_limit = 21000; /* Default gas limit */
    transaction->gas_price = 1000000000ULL; /* 1 Gwei */
    transaction->confirmed = false;
    transaction->executed = false;
    transaction->failed = false;
    
    pthread_mutex_init(&transaction->lock, NULL);
    
    /* Generate transaction hash */
    crypto_hash_data((uint8_t*)transaction, sizeof(transaction_t), transaction->tx_hash);
    
    return 0;
}

/* Create token */
int token_create(const char *name, const char *symbol, uint8_t decimals, 
                uint64_t total_supply, token_standard_t standard) {
    if (!name || !symbol || blockchain_system.token_count >= MAX_TOKENS) {
        return -EINVAL;
    }
    
    token_t *token = &blockchain_system.tokens[blockchain_system.token_count];
    
    token->token_id = blockchain_system.token_count;
    strncpy(token->name, name, sizeof(token->name) - 1);
    strncpy(token->symbol, symbol, sizeof(token->symbol) - 1);
    token->standard = standard;
    token->decimals = decimals;
    token->total_supply = total_supply;
    token->max_supply = total_supply;
    token->mintable = true;
    token->burnable = true;
    token->pausable = false;
    
    /* Generate contract address for token */
    uint8_t token_data[64];
    snprintf((char*)token_data, sizeof(token_data), "token_%s_%u", symbol, token->token_id);
    crypto_hash_data(token_data, strlen((char*)token_data), token->contract_address);
    
    token->deployment_block = blockchain_system.total_blocks;
    token->market_cap = 0;
    token->circulating_supply = total_supply;
    token->locked_supply = 0;
    
    strcpy(token->description, "LimitlessOS native token");
    strcpy(token->website, "https://limitlessos.org");
    strcpy(token->logo_url, "https://limitlessos.org/logo.png");
    
    token->active = true;
    token->created = time(NULL);
    
    pthread_mutex_init(&token->lock, NULL);
    
    blockchain_system.token_count++;
    
    printf("Created token: %s (%s) - %lu supply with %u decimals\n", 
           name, symbol, total_supply, decimals);
    
    return token->token_id;
}

/* Create decentralized identity */
int identity_create(const char *name, const char *email, decentralized_identity_t *identity) {
    if (!name || !email || !identity || blockchain_system.identity_count >= MAX_IDENTITIES) {
        return -EINVAL;
    }
    
    identity->identity_id = blockchain_system.identity_count;
    
    /* Generate DID (Decentralized Identifier) */
    snprintf((char*)identity->did, sizeof(identity->did), "did:limitless:%08x", identity->identity_id);
    
    strncpy(identity->name, name, sizeof(identity->name) - 1);
    strncpy(identity->email, email, sizeof(identity->email) - 1);
    identity->verification_level = IDENTITY_UNVERIFIED;
    
    /* Generate cryptographic proof */
    crypto_generate_keypair(identity->public_key, NULL); /* Only store public key */
    
    /* Create identity hash */
    uint8_t identity_data[512];
    snprintf((char*)identity_data, sizeof(identity_data), "%s%s%s", 
             identity->did, name, email);
    crypto_hash_data(identity_data, strlen((char*)identity_data), identity->identity_hash);
    
    /* Initialize credentials and attestations */
    identity->credential_hashes = NULL;
    identity->credential_count = 0;
    identity->attestation_signatures = NULL;
    identity->attestor_addresses = NULL;
    identity->attestation_count = 0;
    
    /* Privacy settings */
    identity->public_profile = false;
    identity->kyc_verified = false;
    identity->government_verified = false;
    
    /* Blockchain anchoring (would be done in a real transaction) */
    memset(identity->anchor_tx_hash, 0, HASH_SIZE);
    identity->anchor_block = 0;
    
    /* Status */
    identity->active = true;
    identity->revoked = false;
    identity->created = time(NULL);
    identity->last_updated = identity->created;
    
    pthread_mutex_init(&identity->lock, NULL);
    
    blockchain_system.identity_count++;
    
    printf("Created decentralized identity: %s (%s) - DID: %s\n", 
           name, email, identity->did);
    
    return identity->identity_id;
}

/* Background thread functions */
static void *mining_thread_func(void *arg) {
    int thread_id = *(int*)arg;
    
    while (blockchain_system.threads_running && blockchain_system.mining_enabled) {
        /* Simple mining simulation */
        for (uint32_t blockchain_id = 0; blockchain_id < blockchain_system.blockchain_count; blockchain_id++) {
            blockchain_config_t *config = &blockchain_system.blockchain_configs[blockchain_id];
            
            if (config->consensus == CONSENSUS_PROOF_OF_WORK) {
                /* Look for unmined blocks */
                uint32_t block_count = blockchain_system.blockchain_lengths[blockchain_id];
                
                if (block_count > 0) {
                    block_t *block = &blockchain_system.blockchain_data[blockchain_id][block_count - 1];
                    
                    if (!block->validated) {
                        printf("Thread %d mining block %u on blockchain %u\n", 
                               thread_id, block_count - 1, blockchain_id);
                        
                        mine_block_pow(block, config->difficulty_adjustment_blocks);
                        blockchain_system.hashrate += 1000; /* Simulate hashrate */
                    }
                }
            }
        }
        
        usleep(100000); /* 100ms */
    }
    
    return NULL;
}

static void *validation_thread_func(void *arg) {
    while (blockchain_system.threads_running) {
        /* Validate pending transactions and blocks */
        for (uint32_t blockchain_id = 0; blockchain_id < blockchain_system.blockchain_count; blockchain_id++) {
            uint32_t block_count = blockchain_system.blockchain_lengths[blockchain_id];
            
            for (uint32_t block_idx = 0; block_idx < block_count; block_idx++) {
                block_t *block = &blockchain_system.blockchain_data[blockchain_id][block_idx];
                
                if (!block->validated) {
                    printf("Validating block %u on blockchain %u\n", block_idx, blockchain_id);
                    
                    /* Validate all transactions in the block */
                    bool all_valid = true;
                    for (uint32_t tx_idx = 0; tx_idx < block->transaction_count; tx_idx++) {
                        if (transaction_verify(&block->transactions[tx_idx]) != 0) {
                            all_valid = false;
                            break;
                        }
                    }
                    
                    if (all_valid) {
                        block->validated = true;
                        block->confirmation_count = 1;
                        printf("Block %u validated successfully\n", block_idx);
                    }
                }
            }
        }
        
        sleep(5); /* Check every 5 seconds */
    }
    
    return NULL;
}

static void *network_thread_func(void *arg) {
    while (blockchain_system.threads_running) {
        /* Network maintenance and peer management */
        for (uint32_t i = 0; i < blockchain_system.peer_count; i++) {
            blockchain_peer_t *peer = &blockchain_system.peers[i];
            
            time_t now = time(NULL);
            if (peer->connected && (now - peer->last_seen) > 300) {
                /* Mark peer as disconnected after 5 minutes */
                peer->connected = false;
                printf("Peer %u marked as disconnected\n", peer->peer_id);
            }
            
            /* Update trust scores */
            peer->trust_score = calculate_peer_trust_score(peer);
        }
        
        sleep(60); /* Check every minute */
    }
    
    return NULL;
}

static void *consensus_thread_func(void *arg) {
    while (blockchain_system.threads_running) {
        /* Consensus protocol operations */
        for (uint32_t blockchain_id = 0; blockchain_id < blockchain_system.blockchain_count; blockchain_id++) {
            blockchain_config_t *config = &blockchain_system.blockchain_configs[blockchain_id];
            
            if (config->consensus == CONSENSUS_PROOF_OF_STAKE) {
                /* Proof of Stake consensus simulation */
                uint32_t block_count = blockchain_system.blockchain_lengths[blockchain_id];
                
                if (block_count > 0) {
                    block_t *latest_block = &blockchain_system.blockchain_data[blockchain_id][block_count - 1];
                    
                    /* Check if it's time for the next block */
                    time_t now = time(NULL);
                    if (now - latest_block->header.timestamp >= config->block_time_seconds) {
                        /* Create new block (simplified) */
                        printf("Creating new block via PoS consensus on blockchain %u\n", blockchain_id);
                    }
                }
            }
        }
        
        sleep(10); /* Check every 10 seconds */
    }
    
    return NULL;
}

/* Helper function implementations */
static int mine_block_pow(block_t *block, uint32_t difficulty) {
    uint32_t target = 0xFFFFFFFF >> difficulty;
    
    for (uint64_t nonce = 0; nonce < 0xFFFFFFFFULL; nonce++) {
        block->header.nonce = (uint32_t)nonce;
        
        /* Calculate block hash */
        crypto_hash_data((uint8_t*)&block->header, sizeof(block_header_t) - HASH_SIZE, 
                        block->header.hash);
        
        /* Check if hash meets difficulty target */
        uint32_t hash_value = *(uint32_t*)block->header.hash;
        if (hash_value < target) {
            block->validated = true;
            block->mining_time = (uint32_t)time(NULL);
            printf("Block mined! Nonce: %u, Hash: %08x\n", block->header.nonce, hash_value);
            return 0;
        }
    }
    
    return -1; /* Mining failed */
}

static double calculate_peer_trust_score(const blockchain_peer_t *peer) {
    if (peer->successful_interactions + peer->failed_interactions == 0) {
        return 0.5; /* Neutral score for new peers */
    }
    
    double success_rate = (double)peer->successful_interactions / 
                         (peer->successful_interactions + peer->failed_interactions);
    
    /* Factor in connection time and activity */
    time_t now = time(NULL);
    double connection_bonus = fmin(0.1, (now - peer->connection_time) / 86400.0 * 0.1); /* Max 0.1 for 1 day */
    
    return fmin(1.0, success_rate + connection_bonus);
}

/* Cryptographic functions */
int crypto_generate_keypair(uint8_t public_key[PUBLIC_KEY_SIZE], uint8_t private_key[PRIVATE_KEY_SIZE]) {
    /* Generate random private key */
    if (RAND_bytes(private_key, PRIVATE_KEY_SIZE) != 1) {
        return -1;
    }
    
    /* Derive public key (simplified - in reality would use ECC) */
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (EVP_DigestUpdate(ctx, private_key, PRIVATE_KEY_SIZE) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    unsigned int public_key_len = PUBLIC_KEY_SIZE;
    if (EVP_DigestFinal_ex(ctx, public_key, &public_key_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    
    /* Add compression flag */
    public_key[0] = 0x02; /* Compressed public key prefix */
    
    return 0;
}

int crypto_derive_address(const uint8_t public_key[PUBLIC_KEY_SIZE], uint8_t address[ADDRESS_SIZE]) {
    uint8_t hash[HASH_SIZE];
    
    /* Hash the public key */
    if (crypto_hash_data(public_key, PUBLIC_KEY_SIZE, hash) != 0) {
        return -1;
    }
    
    /* Take first 20 bytes as address */
    memcpy(address, hash, ADDRESS_SIZE);
    
    return 0;
}

int crypto_hash_data(const uint8_t *data, uint32_t data_size, uint8_t hash[HASH_SIZE]) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (EVP_DigestUpdate(ctx, data, data_size) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    unsigned int hash_len = HASH_SIZE;
    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    return 0;
}

int transaction_verify(const transaction_t *transaction) {
    if (!transaction) return -EINVAL;
    
    /* Verify transaction structure */
    if (transaction->input_count > 1000 || transaction->output_count > 1000) {
        return -1; /* Suspicious transaction */
    }
    
    /* Verify inputs have sufficient value (simplified) */
    uint64_t input_value = 0;
    uint64_t output_value = 0;
    
    for (uint32_t i = 0; i < transaction->output_count; i++) {
        output_value += transaction->outputs[i].value;
    }
    
    /* For demo purposes, assume inputs are valid if outputs are reasonable */
    if (output_value > 0 && output_value < 1000000000000000000ULL) { /* Less than 1B coins */
        return 0; /* Valid */
    }
    
    return -1; /* Invalid */
}

/* Utility function implementations */
const char *blockchain_type_name(blockchain_type_t type) {
    static const char *names[] = {
        "Bitcoin", "Ethereum", "LimitlessOS", "Private", "Consortium", "Hybrid"
    };
    
    if (type < BLOCKCHAIN_MAX) {
        return names[type];
    }
    return "Unknown";
}

const char *consensus_algorithm_name(consensus_algorithm_t consensus) {
    static const char *names[] = {
        "Proof of Work", "Proof of Stake", "Delegated Proof of Stake",
        "Practical Byzantine Fault Tolerance", "Raft", "Proof of Authority",
        "Proof of Space", "Proof of History"
    };
    
    if (consensus < CONSENSUS_MAX) {
        return names[consensus];
    }
    return "Unknown";
}

const char *contract_language_name(contract_language_t language) {
    static const char *names[] = {
        "Solidity", "Vyper", "Rust", "WebAssembly", "JavaScript", "Python", "Go", "LimitlessScript"
    };
    
    if (language < CONTRACT_MAX) {
        return names[language];
    }
    return "Unknown";
}

void address_to_string(const uint8_t address[ADDRESS_SIZE], char *str) {
    strcpy(str, "0x");
    for (int i = 0; i < ADDRESS_SIZE; i++) {
        sprintf(str + 2 + i * 2, "%02x", address[i]);
    }
}

/* Additional stub implementations */
int mining_start(uint32_t blockchain_id, uint8_t miner_address[ADDRESS_SIZE]) {
    if (blockchain_id >= blockchain_system.blockchain_count) return -EINVAL;
    
    blockchain_system.mining_enabled = true;
    
    /* Start mining threads */
    for (uint32_t i = 0; i < blockchain_system.mining_threads; i++) {
        pthread_create(&blockchain_system.mining_threads_handles[i], NULL, mining_thread_func, &i);
    }
    
    printf("Mining started on blockchain %u with %u threads\n", 
           blockchain_id, blockchain_system.mining_threads);
    
    return 0;
}

int mining_stop(void) {
    if (!blockchain_system.mining_enabled) return 0;
    
    blockchain_system.mining_enabled = false;
    
    /* Wait for mining threads to finish */
    for (uint32_t i = 0; i < blockchain_system.mining_threads; i++) {
        pthread_join(blockchain_system.mining_threads_handles[i], NULL);
    }
    
    printf("Mining stopped\n");
    
    return 0;
}

int network_add_peer(const char *ip_address, uint16_t port) {
    if (!ip_address || blockchain_system.peer_count >= MAX_PEERS) {
        return -EINVAL;
    }
    
    blockchain_peer_t *peer = &blockchain_system.peers[blockchain_system.peer_count];
    
    peer->peer_id = blockchain_system.peer_count;
    strncpy(peer->ip_address, ip_address, sizeof(peer->ip_address) - 1);
    peer->port = port;
    snprintf(peer->node_id, sizeof(peer->node_id), "node_%s_%u", ip_address, port);
    
    peer->full_node = true;
    peer->mining_node = false;
    peer->validator_node = false;
    peer->protocol_version = 1;
    
    peer->connected = false;
    peer->last_seen = time(NULL);
    peer->connection_time = peer->last_seen;
    peer->latency_ms = 0;
    
    peer->trust_score = 0.5; /* Neutral starting score */
    peer->successful_interactions = 0;
    peer->failed_interactions = 0;
    
    pthread_mutex_init(&peer->lock, NULL);
    
    blockchain_system.peer_count++;
    
    printf("Added peer: %s:%u (ID: %u)\n", ip_address, port, peer->peer_id);
    
    return peer->peer_id;
}