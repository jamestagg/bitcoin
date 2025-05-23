// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_EBC_EBC_ADDRESS_H
#define BITCOIN_EBC_EBC_ADDRESS_H

#include <crypto/pq/pq_keys.h>
#include <script/script.h>
#include <uint256.h>

#include <string>
#include <vector>

/** EBC address version types */
enum class EBCAddressVersion : uint8_t {
    PQ_PUBKEY = 0,      // Post-quantum public key hash (P2PQPKH)
    PQ_SCRIPT = 1,      // Post-quantum script hash (P2PQSH)
    PQ_WITNESS_V0 = 2,  // Post-quantum witness v0
    PQ_WITNESS_V1 = 3,  // Post-quantum witness v1 (future)
};

/** EBC address type */
class EBCAddress {
private:
    EBCAddressVersion m_version;
    std::vector<unsigned char> m_data;
    pq::PQAlgorithm m_algorithm;

public:
    EBCAddress() : m_version(EBCAddressVersion::PQ_PUBKEY), m_algorithm(pq::PQAlgorithm::UNKNOWN) {}
    
    EBCAddress(EBCAddressVersion version, const std::vector<unsigned char>& data, 
               pq::PQAlgorithm algo = pq::PQAlgorithm::DILITHIUM3)
        : m_version(version), m_data(data), m_algorithm(algo) {}

    /** Get the address version */
    EBCAddressVersion GetVersion() const { return m_version; }
    
    /** Get the address data */
    const std::vector<unsigned char>& GetData() const { return m_data; }
    
    /** Get the PQ algorithm */
    pq::PQAlgorithm GetAlgorithm() const { return m_algorithm; }
    
    /** Check if the address is valid */
    bool IsValid() const;
    
    /** Encode to EBC address string */
    std::string ToString() const;
    
    /** Decode from EBC address string */
    static EBCAddress FromString(const std::string& str);
    
    /** Create address from PQ public key */
    static EBCAddress FromPQPubKey(const pq::PQPubKey& pubkey);
    
    /** Create address from script hash */
    static EBCAddress FromScriptHash(const uint256& hash, pq::PQAlgorithm algo = pq::PQAlgorithm::DILITHIUM3);
    
    /** Get the corresponding script */
    CScript GetScript() const;
    
    /** Check if this is a P2PQPKH address */
    bool IsP2PQPKH() const { return m_version == EBCAddressVersion::PQ_PUBKEY; }
    
    /** Check if this is a P2PQSH address */
    bool IsP2PQSH() const { return m_version == EBCAddressVersion::PQ_SCRIPT; }
    
    /** Check if this is a witness address */
    bool IsWitness() const { 
        return m_version == EBCAddressVersion::PQ_WITNESS_V0 || 
               m_version == EBCAddressVersion::PQ_WITNESS_V1; 
    }
    
    bool operator==(const EBCAddress& other) const;
    bool operator!=(const EBCAddress& other) const { return !(*this == other); }
};

/** EBC address utility functions */
namespace ebc_address {
    /** Get the human-readable part for EBC addresses */
    std::string GetHRP();
    
    /** Check if a string is a valid EBC address */
    bool IsValidEBCAddress(const std::string& str);
    
    /** Extract EBC address from any address string */
    EBCAddress ExtractEBCAddress(const std::string& str);
    
    /** Convert legacy Bitcoin address to EBC address (for migration) */
    EBCAddress ConvertFromLegacy(const std::string& legacy_addr, const pq::PQPubKey& pq_pubkey);
    
    /** Create P2PQPKH script */
    CScript CreateP2PQPKHScript(const std::vector<unsigned char>& pubkey_hash, pq::PQAlgorithm algo);
    
    /** Create P2PQSH script */
    CScript CreateP2PQSHScript(const uint256& script_hash, pq::PQAlgorithm algo);
    
    /** Hash a PQ public key for address creation */
    std::vector<unsigned char> HashPQPubKey(const pq::PQPubKey& pubkey);
    
    /** Hash a script for P2PQSH address creation */
    uint256 HashScript(const CScript& script);
}

#endif // BITCOIN_EBC_EBC_ADDRESS_H 