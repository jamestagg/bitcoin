// Copyright (c) 2025 Valis Labs
// Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.
// Original Bitcoin Core code licensed under MIT. See LICENSE-MIT.

#ifndef BITCOIN_SCRIPT_PQ_INTERPRETER_H
#define BITCOIN_SCRIPT_PQ_INTERPRETER_H

#include <script/script.h>
#include <crypto/pq/pq_keys.h>

#include <vector>

class CTransaction;
class PrecomputedTransactionData;
class BaseSignatureChecker;

/** Post-quantum signature hash types */
enum class PQSigHashType : uint8_t {
    ALL = 0x01,
    NONE = 0x02,
    SINGLE = 0x03,
    ANYONECANPAY = 0x80,
};

/** Post-quantum script execution context */
struct PQScriptExecutionData {
    /** The transaction being validated */
    const CTransaction* tx;
    /** Input index being validated */
    unsigned int nIn;
    /** Amount being spent */
    int64_t amount;
    /** Precomputed transaction data for optimization */
    const PrecomputedTransactionData* txdata;
    /** EBC-specific flags */
    bool fEBCActive;
    bool fGracePeriod;
    
    PQScriptExecutionData(const CTransaction* tx_in, unsigned int nIn_in, 
                         int64_t amount_in, const PrecomputedTransactionData* txdata_in,
                         bool fEBCActive_in = false, bool fGracePeriod_in = false)
        : tx(tx_in), nIn(nIn_in), amount(amount_in), txdata(txdata_in),
          fEBCActive(fEBCActive_in), fGracePeriod(fGracePeriod_in) {}
};

/** Post-quantum signature checker */
class PQSignatureChecker : public BaseSignatureChecker {
private:
    const PQScriptExecutionData* m_execdata;

public:
    explicit PQSignatureChecker(const PQScriptExecutionData* execdata) 
        : m_execdata(execdata) {}

    /** Check a post-quantum signature */
    bool CheckPQSig(const std::vector<unsigned char>& vchSig,
                    const std::vector<unsigned char>& vchPubKey,
                    const CScript& scriptCode,
                    PQSigHashType sigHashType) const;

    /** Compute post-quantum signature hash */
    uint256 ComputePQSignatureHash(const CScript& scriptCode, 
                                   PQSigHashType sigHashType) const;

    /** Extract PQ signature and hash type from signature data */
    static bool ExtractPQSigData(const std::vector<unsigned char>& vchSig,
                                 pq::PQSignature& sig,
                                 PQSigHashType& sigHashType);

    /** Create PQ signature data with hash type */
    static std::vector<unsigned char> CreatePQSigData(const pq::PQSignature& sig,
                                                      PQSigHashType sigHashType);
};

/** Execute OP_PQCHECKSIG operation */
bool EvalPQCheckSig(const std::vector<unsigned char>& vchSig,
                    const std::vector<unsigned char>& vchPubKey,
                    const CScript& scriptCode,
                    const PQSignatureChecker& checker,
                    std::vector<std::vector<unsigned char>>& stack);

/** Execute OP_PQCHECKSIGVERIFY operation */
bool EvalPQCheckSigVerify(const std::vector<unsigned char>& vchSig,
                          const std::vector<unsigned char>& vchPubKey,
                          const CScript& scriptCode,
                          const PQSignatureChecker& checker);

/** Check if post-quantum operations are enabled */
bool IsPQEnabled(const PQScriptExecutionData& execdata);

/** Validate post-quantum public key format */
bool IsValidPQPubKey(const std::vector<unsigned char>& vchPubKey);

/** Validate post-quantum signature format */
bool IsValidPQSignature(const std::vector<unsigned char>& vchSig);

#endif // BITCOIN_SCRIPT_PQ_INTERPRETER_H 