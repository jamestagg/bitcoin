// Copyright (c) 2025 Valis Labs
// Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.
// Original Bitcoin Core code licensed under MIT. See LICENSE-MIT.

#ifndef BITCOIN_CRYPTO_PQ_PQ_KEYS_H
#define BITCOIN_CRYPTO_PQ_PQ_KEYS_H

#include <span>
#include <vector>
#include <memory>
#include <string>

// Forward declarations for liboqs types
struct OQS_SIG;

namespace pq {

/** Post-quantum signature algorithm types */
enum class PQAlgorithm {
    DILITHIUM3,  // CRYSTALS-Dilithium mode 3 (primary)
    FALCON512,   // FALCON-512 (fallback)
    UNKNOWN
};

/** Post-quantum public key */
class PQPubKey {
private:
    std::vector<unsigned char> m_data;
    PQAlgorithm m_algorithm;

public:
    PQPubKey() : m_algorithm(PQAlgorithm::UNKNOWN) {}
    PQPubKey(const std::vector<unsigned char>& data, PQAlgorithm algo);
    
    /** Get the raw public key data */
    const std::vector<unsigned char>& GetData() const { return m_data; }
    
    /** Get the algorithm type */
    PQAlgorithm GetAlgorithm() const { return m_algorithm; }
    
    /** Check if the key is valid */
    bool IsValid() const;
    
    /** Get the expected public key size for the algorithm */
    static size_t GetPubKeySize(PQAlgorithm algo);
    
    /** Serialize the public key */
    std::vector<unsigned char> Serialize() const;
    
    /** Deserialize a public key */
    static PQPubKey Deserialize(const std::vector<unsigned char>& data);
    
    bool operator==(const PQPubKey& other) const;
    bool operator!=(const PQPubKey& other) const { return !(*this == other); }
};

/** Post-quantum private key */
class PQPrivKey {
private:
    std::vector<unsigned char> m_data;
    PQAlgorithm m_algorithm;
    mutable std::unique_ptr<OQS_SIG> m_sig_ctx;

public:
    PQPrivKey() : m_algorithm(PQAlgorithm::UNKNOWN) {}
    PQPrivKey(const std::vector<unsigned char>& data, PQAlgorithm algo);
    ~PQPrivKey();
    
    // Disable copy constructor and assignment to prevent key duplication
    PQPrivKey(const PQPrivKey&) = delete;
    PQPrivKey& operator=(const PQPrivKey&) = delete;
    
    // Enable move constructor and assignment
    PQPrivKey(PQPrivKey&& other) noexcept;
    PQPrivKey& operator=(PQPrivKey&& other) noexcept;
    
    /** Get the corresponding public key */
    PQPubKey GetPubKey() const;
    
    /** Get the algorithm type */
    PQAlgorithm GetAlgorithm() const { return m_algorithm; }
    
    /** Check if the key is valid */
    bool IsValid() const;
    
    /** Sign a message hash */
    std::vector<unsigned char> Sign(const std::vector<unsigned char>& hash) const;
    
    /** Get the expected private key size for the algorithm */
    static size_t GetPrivKeySize(PQAlgorithm algo);
    
    /** Generate a new key pair */
    static std::pair<PQPrivKey, PQPubKey> GenerateKeyPair(PQAlgorithm algo);
    
    /** Clear the private key data */
    void Clear();
};

/** Post-quantum signature */
class PQSignature {
private:
    std::vector<unsigned char> m_data;
    PQAlgorithm m_algorithm;

public:
    PQSignature() : m_algorithm(PQAlgorithm::UNKNOWN) {}
    PQSignature(const std::vector<unsigned char>& data, PQAlgorithm algo);
    
    /** Get the raw signature data */
    const std::vector<unsigned char>& GetData() const { return m_data; }
    
    /** Get the algorithm type */
    PQAlgorithm GetAlgorithm() const { return m_algorithm; }
    
    /** Check if the signature is valid */
    bool IsValid() const;
    
    /** Verify the signature against a message hash and public key */
    bool Verify(const std::vector<unsigned char>& hash, const PQPubKey& pubkey) const;
    
    /** Get the expected signature size for the algorithm */
    static size_t GetSignatureSize(PQAlgorithm algo);
    
    /** Serialize the signature */
    std::vector<unsigned char> Serialize() const;
    
    /** Deserialize a signature */
    static PQSignature Deserialize(const std::vector<unsigned char>& data);
    
    bool operator==(const PQSignature& other) const;
    bool operator!=(const PQSignature& other) const { return !(*this == other); }
};

/** Utility functions */
namespace util {
    /** Convert algorithm enum to string */
    std::string AlgorithmToString(PQAlgorithm algo);
    
    /** Convert string to algorithm enum */
    PQAlgorithm StringToAlgorithm(const std::string& str);
    
    /** Check if liboqs supports the algorithm */
    bool IsAlgorithmSupported(PQAlgorithm algo);
    
    /** Get all supported algorithms */
    std::vector<PQAlgorithm> GetSupportedAlgorithms();
}

} // namespace pq

#endif // BITCOIN_CRYPTO_PQ_PQ_KEYS_H 