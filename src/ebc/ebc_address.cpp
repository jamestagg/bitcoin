// Copyright (c) 2025 Valis Labs
// Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.

#include <ebc/ebc_address.h>
#include <bech32.h>
#include <hash.h>
#include <util/strencodings.h>
#include <chainparams.h>

#include <algorithm>

// EBC address constants
static const std::string EBC_HRP = "ebc1";
static const size_t EBC_PUBKEY_HASH_SIZE = 20;  // RIPEMD160 size
static const size_t EBC_SCRIPT_HASH_SIZE = 32;  // SHA256 size

bool EBCAddress::IsValid() const {
    if (m_algorithm == pq::PQAlgorithm::UNKNOWN) return false;
    
    switch (m_version) {
        case EBCAddressVersion::PQ_PUBKEY:
            return m_data.size() == EBC_PUBKEY_HASH_SIZE;
        case EBCAddressVersion::PQ_SCRIPT:
            return m_data.size() == EBC_SCRIPT_HASH_SIZE;
        case EBCAddressVersion::PQ_WITNESS_V0:
            return m_data.size() == EBC_PUBKEY_HASH_SIZE || m_data.size() == EBC_SCRIPT_HASH_SIZE;
        case EBCAddressVersion::PQ_WITNESS_V1:
            return m_data.size() == EBC_SCRIPT_HASH_SIZE;
        default:
            return false;
    }
}

std::string EBCAddress::ToString() const {
    if (!IsValid()) return "";
    
    std::vector<uint8_t> data;
    data.reserve(2 + m_data.size());
    
    // Add version and algorithm
    data.push_back(static_cast<uint8_t>(m_version));
    data.push_back(static_cast<uint8_t>(m_algorithm));
    
    // Add address data
    data.insert(data.end(), m_data.begin(), m_data.end());
    
    // Convert to 5-bit groups for Bech32m
    std::vector<uint8_t> conv_data;
    conv_data.reserve(1 + (data.size() * 8 + 4) / 5);
    
    if (!ConvertBits<8, 5, true>([&](unsigned char c) { conv_data.push_back(c); }, 
                                  data.begin(), data.end())) {
        return "";
    }
    
    return bech32::Encode(bech32::Encoding::BECH32M, EBC_HRP, conv_data);
}

EBCAddress EBCAddress::FromString(const std::string& str) {
    auto dec = bech32::Decode(str);
    
    if (dec.encoding != bech32::Encoding::BECH32M || dec.hrp != EBC_HRP) {
        return EBCAddress(); // Invalid
    }
    
    if (dec.data.empty()) return EBCAddress();
    
    // Convert from 5-bit groups back to 8-bit
    std::vector<uint8_t> data;
    if (!ConvertBits<5, 8, false>([&](unsigned char c) { data.push_back(c); }, 
                                   dec.data.begin(), dec.data.end())) {
        return EBCAddress();
    }
    
    if (data.size() < 2) return EBCAddress(); // Need at least version + algorithm
    
    EBCAddressVersion version = static_cast<EBCAddressVersion>(data[0]);
    pq::PQAlgorithm algorithm = static_cast<pq::PQAlgorithm>(data[1]);
    
    std::vector<unsigned char> addr_data(data.begin() + 2, data.end());
    
    EBCAddress addr(version, addr_data, algorithm);
    return addr.IsValid() ? addr : EBCAddress();
}

EBCAddress EBCAddress::FromPQPubKey(const pq::PQPubKey& pubkey) {
    if (!pubkey.IsValid()) return EBCAddress();
    
    auto hash = ebc_address::HashPQPubKey(pubkey);
    return EBCAddress(EBCAddressVersion::PQ_PUBKEY, hash, pubkey.GetAlgorithm());
}

EBCAddress EBCAddress::FromScriptHash(const uint256& hash, pq::PQAlgorithm algo) {
    std::vector<unsigned char> hash_data(hash.begin(), hash.end());
    return EBCAddress(EBCAddressVersion::PQ_SCRIPT, hash_data, algo);
}

CScript EBCAddress::GetScript() const {
    if (!IsValid()) return CScript();
    
    switch (m_version) {
        case EBCAddressVersion::PQ_PUBKEY:
            return ebc_address::CreateP2PQPKHScript(m_data, m_algorithm);
        case EBCAddressVersion::PQ_SCRIPT: {
            uint256 hash;
            std::copy(m_data.begin(), m_data.end(), hash.begin());
            return ebc_address::CreateP2PQSHScript(hash, m_algorithm);
        }
        case EBCAddressVersion::PQ_WITNESS_V0:
        case EBCAddressVersion::PQ_WITNESS_V1:
            // Witness scripts - to be implemented
            return CScript();
        default:
            return CScript();
    }
}

bool EBCAddress::operator==(const EBCAddress& other) const {
    return m_version == other.m_version && 
           m_data == other.m_data && 
           m_algorithm == other.m_algorithm;
}

// EBC address utility functions
namespace ebc_address {

std::string GetHRP() {
    return EBC_HRP;
}

bool IsValidEBCAddress(const std::string& str) {
    return EBCAddress::FromString(str).IsValid();
}

EBCAddress ExtractEBCAddress(const std::string& str) {
    return EBCAddress::FromString(str);
}

EBCAddress ConvertFromLegacy(const std::string& legacy_addr, const pq::PQPubKey& pq_pubkey) {
    // This would convert a legacy Bitcoin address to an EBC address
    // For now, just create from the PQ public key
    return EBCAddress::FromPQPubKey(pq_pubkey);
}

CScript CreateP2PQPKHScript(const std::vector<unsigned char>& pubkey_hash, pq::PQAlgorithm algo) {
    CScript script;
    
    // EBC P2PQPKH format: OP_DUP OP_HASH160 <pubkey_hash> OP_EQUALVERIFY OP_PQCHECKSIG
    script << OP_DUP 
           << OP_HASH160 
           << pubkey_hash 
           << OP_EQUALVERIFY 
           << OP_PQCHECKSIG;
    
    return script;
}

CScript CreateP2PQSHScript(const uint256& script_hash, pq::PQAlgorithm algo) {
    CScript script;
    
    // EBC P2PQSH format: OP_HASH256 <script_hash> OP_EQUAL
    script << OP_HASH256 
           << ToByteVector(script_hash) 
           << OP_EQUAL;
    
    return script;
}

std::vector<unsigned char> HashPQPubKey(const pq::PQPubKey& pubkey) {
    if (!pubkey.IsValid()) return {};
    
    // Hash the serialized public key: RIPEMD160(SHA256(pubkey))
    auto serialized = pubkey.Serialize();
    uint256 sha_hash = Hash(serialized);
    
    CRIPEMD160 ripemd;
    ripemd.Write(sha_hash.data(), sha_hash.size());
    
    std::vector<unsigned char> result(CRIPEMD160::OUTPUT_SIZE);
    ripemd.Finalize(result.data());
    
    return result;
}

uint256 HashScript(const CScript& script) {
    return Hash(script);
}

} // namespace ebc_address 