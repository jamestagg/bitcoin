// Copyright (c) 2025 Valis Labs
// Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.
// Original Bitcoin Core code licensed under MIT. See LICENSE-MIT.

#include <kernel/chainparams.h>
#include <chainparams.h>
#include <consensus/params.h>
#include <hash.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <util/strencodings.h>

#include <cassert>

/**
 * Emergency Bitcoin Cut-Over Chain (EBC) parameters
 * 
 * EBC maintains identical economic parameters to Bitcoin but with
 * quantum-safe cryptography and emergency activation mechanisms.
 */
class CEBCParams : public CChainParams {
public:
    CEBCParams() {
        m_chain_type = ChainType::EBC;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        
        // Identical to Bitcoin mainnet economic parameters
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Height = 0; // Always active on EBC
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0; // Segwit always active
        consensus.MinBIP9WarningHeight = 0;
        
        // EBC-specific consensus parameters
        consensus.nEBCActivationHeight = 0; // EBC active from genesis
        consensus.nGracePeriodBlocks = 4320; // 30 days at 10 min/block
        consensus.nWhiteKnightSweepHeight = 25920; // 180 days
        consensus.fEBCEmergencyCouncilActive = true;
        consensus.nEmergencyCouncilSunsetHeight = 52560; // 1 year
        
        // Proof of work parameters (identical to Bitcoin)
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60; // 10 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;
        
        // Quantum-resistance factor: slightly increase difficulty to account for Grover's algorithm
        // This is a 1-bit security reduction, so we double the required work
        consensus.nQuantumResistanceFactor = 2;
        
        // The best chain should have at least this much work
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        
        // By default assume that the signatures in ancestors of this block are valid
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        
        // EBC network parameters
        pchMessageStart[0] = 0xeb; // 'e' for EBC
        pchMessageStart[1] = 0xbc; // 'b' for Bitcoin
        pchMessageStart[2] = 0x01; // version 1
        pchMessageStart[3] = 0x00; // mainnet
        
        nDefaultPort = 8444; // Different from Bitcoin's 8333
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 500; // GB
        m_assumed_chain_state_size = 6;  // GB
        
        // EBC genesis block
        genesis = CreateEBCGenesisBlock();
        consensus.hashGenesisBlock = genesis.GetHash();
        
        // EBC uses ebc1 prefix for addresses
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0); // Disabled
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5); // Disabled  
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        
        bech32_hrp = "ebc1"; // EBC addresses use ebc1 prefix
        
        vSeeds.emplace_back("seed.ebc.bitcoin.org");
        vSeeds.emplace_back("dnsseed.ebc.bitcoin.org");
        
        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;
        
        checkpointData = {
            {
                // EBC checkpoints will be added as the chain develops
            }
        };
        
        m_assumeutxo_data = {
            // EBC assumeutxo data for fast sync
        };
        
        chainTxData = ChainTxData{
            // EBC chain transaction data
            .nTime = 0,
            .tx_count = 0,
            .dTxRate = 0,
        };
    }

private:
    CBlock CreateEBCGenesisBlock() {
        const char* pszTimestamp = "Emergency Bitcoin Cut-Over Chain activated due to quantum threat";
        const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        return CreateGenesisBlock(pszTimestamp, genesisOutputScript, 1640995200, 0x1d00ffff, 1, 50 * COIN);
    }
    
    CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, const CAmount& genesisReward) {
        CMutableTransaction txNew;
        txNew.nVersion = 1;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = genesisReward;
        txNew.vout[0].scriptPubKey = genesisOutputScript;

        CBlock genesis;
        genesis.nTime    = nTime;
        genesis.nBits    = nBits;
        genesis.nNonce   = nNonce;
        genesis.nVersion = 1;
        genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
        genesis.hashPrevBlock.SetNull();
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
        return genesis;
    }
};

/**
 * EBC Testnet parameters
 */
class CEBCTestNetParams : public CEBCParams {
public:
    CEBCTestNetParams() {
        m_chain_type = ChainType::EBCTESTNET;
        
        // Testnet allows min difficulty blocks
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60; // 10 minutes
        
        // Faster grace period for testing
        consensus.nGracePeriodBlocks = 144; // 1 day
        consensus.nWhiteKnightSweepHeight = 1008; // 1 week
        consensus.nEmergencyCouncilSunsetHeight = 2016; // 2 weeks
        
        pchMessageStart[0] = 0xeb;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0x01;
        pchMessageStart[3] = 0x01; // testnet
        
        nDefaultPort = 18444;
        
        bech32_hrp = "tebc1"; // Testnet EBC addresses
        
        vSeeds.clear();
        vSeeds.emplace_back("testnet-seed.ebc.bitcoin.org");
        
        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;
        
        checkpointData = {
            {
                // EBC testnet checkpoints
            }
        };
        
        m_assumeutxo_data = {};
        
        chainTxData = ChainTxData{
            .nTime = 0,
            .tx_count = 0,
            .dTxRate = 0,
        };
    }
};

/**
 * EBC Regression test parameters
 */
class CEBCRegTestParams : public CEBCParams {
public:
    explicit CEBCRegTestParams() {
        m_chain_type = ChainType::EBCREGTEST;
        
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60;
        
        // Very fast parameters for testing
        consensus.nGracePeriodBlocks = 10;
        consensus.nWhiteKnightSweepHeight = 50;
        consensus.nEmergencyCouncilSunsetHeight = 100;
        
        pchMessageStart[0] = 0xeb;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0x01;
        pchMessageStart[3] = 0x02; // regtest
        
        nDefaultPort = 18445;
        
        bech32_hrp = "rebc1"; // Regtest EBC addresses
        
        vSeeds.clear();
        
        fDefaultConsistencyChecks = true;
        m_is_mockable_chain = true;
        
        checkpointData = {
            {
                {0, uint256S("0x")},
            }
        };
        
        m_assumeutxo_data = {};
        
        chainTxData = ChainTxData{
            .nTime = 0,
            .tx_count = 0,
            .dTxRate = 0,
        };
    }
};

// EBC chain parameter factory functions
std::unique_ptr<const CChainParams> CreateEBCChainParams() {
    return std::make_unique<CEBCParams>();
}

std::unique_ptr<const CChainParams> CreateEBCTestNetParams() {
    return std::make_unique<CEBCTestNetParams>();
}

std::unique_ptr<const CChainParams> CreateEBCRegTestParams() {
    return std::make_unique<CEBCRegTestParams>();
} 