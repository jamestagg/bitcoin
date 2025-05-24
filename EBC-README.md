<!--
Copyright (c) 2025 Valis Labs
Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.
Original Bitcoin Core code licensed under MIT. See LICENSE-MIT.
-->

# Emergency Bitcoin Cut-Over Chain (EBC) Implementation

This document describes the implementation of the Emergency Bitcoin Cut-Over Chain (EBC), a quantum-safe backup network for Bitcoin as specified in the [EBC whitepaper](ebc-whitepaper-v1.md).

## Overview

The EBC implementation provides:

1. **Post-quantum cryptography support** using CRYSTALS-Dilithium and FALCON
2. **New script opcodes** (OP_PQCHECKSIG, OP_PQCHECKSIGVERIFY) for PQ signature verification
3. **EBC address format** with `ebc1` prefix using Bech32m encoding
4. **EBC chain parameters** with identical economic properties to Bitcoin
5. **Quantum-safe emergency activation** mechanisms

## Building with EBC Support

### Prerequisites

- liboqs (Open Quantum Safe library)
- All standard Bitcoin Core dependencies

### Build Instructions

```bash
# Configure with EBC support
cmake -B build -DENABLE_EBC=ON

# Build
cmake --build build

# Run tests
ctest --test-dir build
```

### Using vcpkg (Recommended)

```bash
# Install liboqs via vcpkg
vcpkg install liboqs

# Configure with vcpkg
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DENABLE_EBC=ON

# Build
cmake --build build
```

## Implementation Components

### 1. Post-Quantum Cryptography (`src/crypto/pq/`)

#### Key Classes

- **`pq::PQPubKey`**: Post-quantum public key wrapper
- **`pq::PQPrivKey`**: Post-quantum private key wrapper  
- **`pq::PQSignature`**: Post-quantum signature wrapper

#### Supported Algorithms

- **CRYSTALS-Dilithium (mode 3)**: Primary algorithm (~2420 byte signatures)
- **FALCON-512**: Fallback algorithm (~690 byte signatures)

#### Example Usage

```cpp
#include <crypto/pq/pq_keys.h>

// Generate a key pair
auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);

// Sign a message
std::vector<unsigned char> message = {0x01, 0x02, 0x03, 0x04};
auto signature_data = privkey.Sign(message);

// Create and verify signature
pq::PQSignature signature(signature_data, pq::PQAlgorithm::DILITHIUM3);
bool valid = signature.Verify(message, pubkey);
```

### 2. Script System Extensions (`src/script/`)

#### New Opcodes

- **`OP_PQCHECKSIG` (0xbb)**: Verify post-quantum signature
- **`OP_PQCHECKSIGVERIFY` (0xbc)**: Verify PQ signature and fail if invalid

#### Script Formats

**P2PQPKH (Pay-to-PQ-Public-Key-Hash):**
```
OP_DUP OP_HASH160 <pubkey_hash> OP_EQUALVERIFY OP_PQCHECKSIG
```

**P2PQSH (Pay-to-PQ-Script-Hash):**
```
OP_HASH256 <script_hash> OP_EQUAL
```

### 3. EBC Address Format (`src/ebc/`)

#### Address Types

- **P2PQPKH**: `ebc1q...` (PQ public key hash)
- **P2PQSH**: `ebc1p...` (PQ script hash)
- **Witness v0/v1**: Future extensions

#### Example Usage

```cpp
#include <ebc/ebc_address.h>

// Create address from PQ public key
auto address = EBCAddress::FromPQPubKey(pubkey);
std::string addr_str = address.ToString(); // "ebc1q..."

// Decode address
auto decoded = EBCAddress::FromString(addr_str);
if (decoded.IsValid()) {
    auto script = decoded.GetScript();
}
```

### 4. Chain Parameters (`src/chainparams_ebc.cpp`)

#### EBC Networks

- **EBC Mainnet**: Port 8444, `ebc1` addresses
- **EBC Testnet**: Port 18444, `tebc1` addresses  
- **EBC Regtest**: Port 18445, `rebc1` addresses

#### Key Parameters

- **Grace Period**: 30 days (4320 blocks) for dual-signature transactions
- **White-Knight Sweep**: 180 days (25920 blocks) for unmigrated UTXOs
- **Emergency Council**: 1 year (52560 blocks) governance authority
- **Quantum Resistance Factor**: 2x difficulty for Grover's algorithm

## Testing

### Unit Tests

```bash
# Run EBC-specific tests
./build/src/test/test_bitcoin --run_test=ebc_tests

# Run all tests
./build/src/test/test_bitcoin
```

### Test Coverage

The implementation includes tests for:

- PQ key generation and validation
- PQ signature creation and verification
- EBC address encoding/decoding
- Script generation and validation
- Algorithm utility functions

## Usage Examples

### Generating EBC Addresses

```cpp
// Generate Dilithium key pair
auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);

// Create EBC address
auto address = EBCAddress::FromPQPubKey(pubkey);
std::cout << "EBC Address: " << address.ToString() << std::endl;

// Get the locking script
auto script = address.GetScript();
```

### Creating PQ Transactions

```cpp
// Create P2PQPKH output script
auto pubkey_hash = ebc_address::HashPQPubKey(pubkey);
auto script = ebc_address::CreateP2PQPKHScript(pubkey_hash, pq::PQAlgorithm::DILITHIUM3);

// The script will contain: OP_DUP OP_HASH160 <hash> OP_EQUALVERIFY OP_PQCHECKSIG
```

### Verifying PQ Signatures

```cpp
// In script interpreter
if (opcode == OP_PQCHECKSIG) {
    auto vchSig = stacktop(-2);
    auto vchPubKey = stacktop(-1);
    
    // Extract PQ signature and verify
    pq::PQSignature sig;
    PQSigHashType sigHashType;
    if (PQSignatureChecker::ExtractPQSigData(vchSig, sig, sigHashType)) {
        pq::PQPubKey pubkey = pq::PQPubKey::Deserialize(vchPubKey);
        bool valid = sig.Verify(hash, pubkey);
        stack.push_back(valid ? vchTrue : vchFalse);
    }
}
```

## Security Considerations

### Algorithm Agility

The implementation supports multiple PQ algorithms to provide flexibility:

- Primary: CRYSTALS-Dilithium (NIST standardized)
- Fallback: FALCON (compact signatures)
- Future: Additional algorithms can be added

### Hybrid Mode

During the grace period, transactions can use both ECDSA and PQ signatures:

```cpp
// Dual signature validation
bool ecdsa_valid = CheckECDSASignature(ecdsa_sig, ecdsa_pubkey, hash);
bool pq_valid = CheckPQSignature(pq_sig, pq_pubkey, hash);
return ecdsa_valid && pq_valid; // Both must be valid
```

### Key Management

- Private keys are securely cleared from memory
- Public keys are validated before use
- Signature verification includes algorithm checks

## Performance Considerations

### Signature Sizes

| Algorithm | Public Key | Private Key | Signature |
|-----------|------------|-------------|-----------|
| Dilithium3 | 1952 bytes | 4000 bytes | ~2420 bytes |
| FALCON-512 | 897 bytes | 1281 bytes | ~690 bytes |
| ECDSA (comparison) | 33 bytes | 32 bytes | ~71 bytes |

### Optimization Strategies

1. **Signature Caching**: Cache PQ signature verifications
2. **Batch Verification**: Verify multiple signatures together when possible
3. **Algorithm Selection**: Use FALCON for size-constrained scenarios
4. **Precomputation**: Pre-validate PQ public keys

## Migration Path

### Phase 1: Pre-registration
Users register PQ public keys on Bitcoin mainnet via OP_RETURN

### Phase 2: Emergency Activation
Emergency Council triggers EBC activation upon quantum threat detection

### Phase 3: Grace Period (30 days)
Dual-signature transactions required (ECDSA + PQ)

### Phase 4: PQ-Only Mode
Only PQ signatures accepted, white-knight sweeps begin

### Phase 5: Full Migration
All funds migrated to PQ addresses, emergency governance expires

## Future Enhancements

### Planned Features

1. **Threshold Signatures**: Multi-signature PQ schemes
2. **Zero-Knowledge Proofs**: Privacy-preserving ownership proofs
3. **Hybrid Time-Locks**: Automatic redemption mechanisms
4. **Advanced Governance**: On-chain voting systems

### Research Areas

1. **Quantum Gravity Computer Safe Cryptography**: Next-generation PQ schemes
2. **Lattice-based Multisig**: Efficient threshold signatures
3. **Post-Quantum Lightning**: Layer 2 scaling solutions
4. **Quantum-Safe Smart Contracts**: Advanced scripting capabilities

## Troubleshooting

### Common Issues

**Build Errors:**
```bash
# Ensure liboqs is installed
sudo apt-get install liboqs-dev  # Ubuntu/Debian
brew install liboqs              # macOS

# Or use vcpkg
vcpkg install liboqs
```

**Runtime Errors:**
```bash
# Check algorithm support
./bitcoind -ebc -debug=pq

# Verify EBC chain parameters
./bitcoind -chain=ebc -printtoconsole
```

### Debug Options

```bash
# Enable EBC debugging
./bitcoind -debug=ebc -debug=pq -debug=script

# Test EBC functionality
./bitcoin-cli -chain=ebc getblockchaininfo
```

## Contributing

### Development Guidelines

1. All PQ operations must be constant-time when possible
2. Include comprehensive tests for new functionality
3. Document security assumptions and limitations
4. Follow Bitcoin Core coding standards

### Testing Requirements

- Unit tests for all new classes and functions
- Integration tests for script execution
- Performance benchmarks for PQ operations
- Security audits for cryptographic code

## References

1. [EBC Whitepaper v1.0](ebc-whitepaper-v1.md)
2. [CRYSTALS-Dilithium Specification](https://pq-crystals.org/dilithium/)
3. [FALCON Specification](https://falcon-sign.info/)
4. [Open Quantum Safe Project](https://openquantumsafe.org/)
5. [NIST Post-Quantum Cryptography Standards](https://csrc.nist.gov/projects/post-quantum-cryptography)

---

**Note**: This implementation is experimental and should not be used in production without thorough security review and testing. The EBC system is designed as an emergency backup mechanism and should only be activated in response to a genuine quantum threat to Bitcoin's cryptography. 