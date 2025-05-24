// Copyright (c) 2025 Valis Labs
// Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.

#include <crypto/pq/pq_keys.h>
#include <logging.h>
#include <random.h>

#include <oqs/oqs.h>
#include <cassert>
#include <cstring>

namespace pq {

// Algorithm name mappings for liboqs
static const char* GetOQSAlgorithmName(PQAlgorithm algo) {
    switch (algo) {
        case PQAlgorithm::DILITHIUM3:
            return OQS_SIG_alg_dilithium_3;
        case PQAlgorithm::FALCON512:
            return OQS_SIG_alg_falcon_512;
        default:
            return nullptr;
    }
}

// PQPubKey implementation
PQPubKey::PQPubKey(const std::vector<unsigned char>& data, PQAlgorithm algo)
    : m_data(data), m_algorithm(algo) {
    if (!IsValid()) {
        m_data.clear();
        m_algorithm = PQAlgorithm::UNKNOWN;
    }
}

bool PQPubKey::IsValid() const {
    if (m_algorithm == PQAlgorithm::UNKNOWN) return false;
    
    size_t expected_size = GetPubKeySize(m_algorithm);
    return expected_size > 0 && m_data.size() == expected_size;
}

size_t PQPubKey::GetPubKeySize(PQAlgorithm algo) {
    const char* alg_name = GetOQSAlgorithmName(algo);
    if (!alg_name) return 0;
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return 0;
    
    size_t size = sig->length_public_key;
    OQS_SIG_free(sig);
    return size;
}

std::vector<unsigned char> PQPubKey::Serialize() const {
    std::vector<unsigned char> result;
    result.reserve(1 + m_data.size());
    
    // Add algorithm identifier
    result.push_back(static_cast<unsigned char>(m_algorithm));
    
    // Add key data
    result.insert(result.end(), m_data.begin(), m_data.end());
    
    return result;
}

PQPubKey PQPubKey::Deserialize(const std::vector<unsigned char>& data) {
    if (data.empty()) return PQPubKey();
    
    PQAlgorithm algo = static_cast<PQAlgorithm>(data[0]);
    if (algo == PQAlgorithm::UNKNOWN) return PQPubKey();
    
    std::vector<unsigned char> key_data(data.begin() + 1, data.end());
    return PQPubKey(key_data, algo);
}

bool PQPubKey::operator==(const PQPubKey& other) const {
    return m_algorithm == other.m_algorithm && m_data == other.m_data;
}

// PQPrivKey implementation
PQPrivKey::PQPrivKey(const std::vector<unsigned char>& data, PQAlgorithm algo)
    : m_data(data), m_algorithm(algo) {
    if (!IsValid()) {
        Clear();
    }
}

PQPrivKey::~PQPrivKey() {
    Clear();
}

PQPrivKey::PQPrivKey(PQPrivKey&& other) noexcept
    : m_data(std::move(other.m_data)), 
      m_algorithm(other.m_algorithm),
      m_sig_ctx(std::move(other.m_sig_ctx)) {
    other.m_algorithm = PQAlgorithm::UNKNOWN;
}

PQPrivKey& PQPrivKey::operator=(PQPrivKey&& other) noexcept {
    if (this != &other) {
        Clear();
        m_data = std::move(other.m_data);
        m_algorithm = other.m_algorithm;
        m_sig_ctx = std::move(other.m_sig_ctx);
        other.m_algorithm = PQAlgorithm::UNKNOWN;
    }
    return *this;
}

bool PQPrivKey::IsValid() const {
    if (m_algorithm == PQAlgorithm::UNKNOWN) return false;
    
    size_t expected_size = GetPrivKeySize(m_algorithm);
    return expected_size > 0 && m_data.size() == expected_size;
}

PQPubKey PQPrivKey::GetPubKey() const {
    if (!IsValid()) return PQPubKey();
    
    const char* alg_name = GetOQSAlgorithmName(m_algorithm);
    if (!alg_name) return PQPubKey();
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return PQPubKey();
    
    std::vector<unsigned char> pubkey_data(sig->length_public_key);
    
    // Extract public key from private key
    // Note: This is a simplified approach - in practice, we might need
    // algorithm-specific key derivation
    if (m_data.size() >= sig->length_public_key) {
        // For most PQ algorithms, public key is embedded in private key
        std::copy(m_data.end() - sig->length_public_key, m_data.end(), pubkey_data.begin());
    }
    
    OQS_SIG_free(sig);
    return PQPubKey(pubkey_data, m_algorithm);
}

std::vector<unsigned char> PQPrivKey::Sign(const std::vector<unsigned char>& hash) const {
    if (!IsValid()) return {};
    
    const char* alg_name = GetOQSAlgorithmName(m_algorithm);
    if (!alg_name) return {};
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return {};
    
    std::vector<unsigned char> signature(sig->length_signature);
    size_t signature_len = 0;
    
    OQS_STATUS status = OQS_SIG_sign(sig, signature.data(), &signature_len,
                                     hash.data(), hash.size(), m_data.data());
    
    OQS_SIG_free(sig);
    
    if (status != OQS_SUCCESS) {
        LogPrintf("PQ signature generation failed\n");
        return {};
    }
    
    signature.resize(signature_len);
    return signature;
}

size_t PQPrivKey::GetPrivKeySize(PQAlgorithm algo) {
    const char* alg_name = GetOQSAlgorithmName(algo);
    if (!alg_name) return 0;
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return 0;
    
    size_t size = sig->length_secret_key;
    OQS_SIG_free(sig);
    return size;
}

std::pair<PQPrivKey, PQPubKey> PQPrivKey::GenerateKeyPair(PQAlgorithm algo) {
    const char* alg_name = GetOQSAlgorithmName(algo);
    if (!alg_name) return {PQPrivKey(), PQPubKey()};
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return {PQPrivKey(), PQPubKey()};
    
    std::vector<unsigned char> pubkey(sig->length_public_key);
    std::vector<unsigned char> privkey(sig->length_secret_key);
    
    OQS_STATUS status = OQS_SIG_keypair(sig, pubkey.data(), privkey.data());
    OQS_SIG_free(sig);
    
    if (status != OQS_SUCCESS) {
        LogPrintf("PQ key generation failed for algorithm %s\n", alg_name);
        return {PQPrivKey(), PQPubKey()};
    }
    
    return {PQPrivKey(privkey, algo), PQPubKey(pubkey, algo)};
}

void PQPrivKey::Clear() {
    if (!m_data.empty()) {
        // Securely clear private key data
        memory_cleanse(m_data.data(), m_data.size());
        m_data.clear();
    }
    m_algorithm = PQAlgorithm::UNKNOWN;
    m_sig_ctx.reset();
}

// PQSignature implementation
PQSignature::PQSignature(const std::vector<unsigned char>& data, PQAlgorithm algo)
    : m_data(data), m_algorithm(algo) {
    if (!IsValid()) {
        m_data.clear();
        m_algorithm = PQAlgorithm::UNKNOWN;
    }
}

bool PQSignature::IsValid() const {
    if (m_algorithm == PQAlgorithm::UNKNOWN) return false;
    if (m_data.empty()) return false;
    
    // Basic size check - actual signature size can vary
    size_t max_size = GetSignatureSize(m_algorithm);
    return max_size > 0 && m_data.size() <= max_size;
}

bool PQSignature::Verify(const std::vector<unsigned char>& hash, const PQPubKey& pubkey) const {
    if (!IsValid() || !pubkey.IsValid()) return false;
    if (m_algorithm != pubkey.GetAlgorithm()) return false;
    
    const char* alg_name = GetOQSAlgorithmName(m_algorithm);
    if (!alg_name) return false;
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return false;
    
    OQS_STATUS status = OQS_SIG_verify(sig, hash.data(), hash.size(),
                                       m_data.data(), m_data.size(),
                                       pubkey.GetData().data());
    
    OQS_SIG_free(sig);
    return status == OQS_SUCCESS;
}

size_t PQSignature::GetSignatureSize(PQAlgorithm algo) {
    const char* alg_name = GetOQSAlgorithmName(algo);
    if (!alg_name) return 0;
    
    OQS_SIG* sig = OQS_SIG_new(alg_name);
    if (!sig) return 0;
    
    size_t size = sig->length_signature;
    OQS_SIG_free(sig);
    return size;
}

std::vector<unsigned char> PQSignature::Serialize() const {
    std::vector<unsigned char> result;
    result.reserve(1 + 4 + m_data.size());
    
    // Add algorithm identifier
    result.push_back(static_cast<unsigned char>(m_algorithm));
    
    // Add signature length (4 bytes, little-endian)
    uint32_t sig_len = static_cast<uint32_t>(m_data.size());
    result.push_back(sig_len & 0xFF);
    result.push_back((sig_len >> 8) & 0xFF);
    result.push_back((sig_len >> 16) & 0xFF);
    result.push_back((sig_len >> 24) & 0xFF);
    
    // Add signature data
    result.insert(result.end(), m_data.begin(), m_data.end());
    
    return result;
}

PQSignature PQSignature::Deserialize(const std::vector<unsigned char>& data) {
    if (data.size() < 5) return PQSignature(); // Need at least algo + length
    
    PQAlgorithm algo = static_cast<PQAlgorithm>(data[0]);
    if (algo == PQAlgorithm::UNKNOWN) return PQSignature();
    
    // Extract signature length
    uint32_t sig_len = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
    
    if (data.size() != 5 + sig_len) return PQSignature();
    
    std::vector<unsigned char> sig_data(data.begin() + 5, data.end());
    return PQSignature(sig_data, algo);
}

bool PQSignature::operator==(const PQSignature& other) const {
    return m_algorithm == other.m_algorithm && m_data == other.m_data;
}

// Utility functions
namespace util {

std::string AlgorithmToString(PQAlgorithm algo) {
    switch (algo) {
        case PQAlgorithm::DILITHIUM3:
            return "Dilithium3";
        case PQAlgorithm::FALCON512:
            return "Falcon512";
        default:
            return "Unknown";
    }
}

PQAlgorithm StringToAlgorithm(const std::string& str) {
    if (str == "Dilithium3") return PQAlgorithm::DILITHIUM3;
    if (str == "Falcon512") return PQAlgorithm::FALCON512;
    return PQAlgorithm::UNKNOWN;
}

bool IsAlgorithmSupported(PQAlgorithm algo) {
    const char* alg_name = GetOQSAlgorithmName(algo);
    if (!alg_name) return false;
    
    return OQS_SIG_alg_is_enabled(alg_name);
}

std::vector<PQAlgorithm> GetSupportedAlgorithms() {
    std::vector<PQAlgorithm> supported;
    
    if (IsAlgorithmSupported(PQAlgorithm::DILITHIUM3)) {
        supported.push_back(PQAlgorithm::DILITHIUM3);
    }
    if (IsAlgorithmSupported(PQAlgorithm::FALCON512)) {
        supported.push_back(PQAlgorithm::FALCON512);
    }
    
    return supported;
}

} // namespace util

} // namespace pq 