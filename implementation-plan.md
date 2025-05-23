# Emergency Bitcoin Cut-Over Chain (EBC) Implementation Plan

## Overview

This document outlines the implementation plan for the Emergency Bitcoin Cut-Over Chain (EBC), a quantum-safe backup network that can activate instantly upon detection of a catastrophic quantum threat to Bitcoin's ECDSA cryptography.

## Implementation Phases

### Phase 1: Core Infrastructure Changes

#### 1.1 Post-Quantum Cryptography Integration
- **Add CRYSTALS-Dilithium support** as the primary PQ signature scheme
- **Add FALCON support** as the fallback PQ signature scheme  
- **Integrate liboqs** (Open Quantum Safe) library for standardized PQ implementations
- **Create PQ key management** classes and utilities

#### 1.2 New Script Opcode Implementation
- **Add OP_PQCHECKSIG** (opcode 0xbb) for post-quantum signature verification
- **Modify script interpreter** to handle PQ signatures
- **Update script validation** for dual-mode (ECDSA + PQ) during grace period

#### 1.3 Address Format Changes
- **Implement ebc1 prefix** for post-quantum addresses using Bech32m encoding
- **Modify address encoding/decoding** to support EBC addresses
- **Add address validation** for EBC-specific formats

#### 1.4 EBC Chain Configuration
- **Create EBC chainparams** with identical economic parameters to Bitcoin
- **Implement snapshot-based genesis** block containing Bitcoin UTXO merkle root
- **Add replay protection** through different chain ID and message start chars
- **Configure difficulty adjustment** with quantum-resistance considerations

### Phase 2: Chain Parameters and Genesis

#### 2.1 Consensus Rule Modifications
- **Grace period enforcement** (30 days dual-sig requirement)
- **PQ-only transaction validation** after grace period
- **White-knight sweep mechanism** for unmigrated UTXOs
- **Emergency Council governance** with time-limited authority

#### 2.2 Network Protocol Changes
- **EBC message headers** and protocol versioning
- **Peer discovery** for EBC network
- **Cross-chain communication** for trigger events
- **Emergency activation** signaling mechanism

### Phase 3: Key Migration System

#### 3.1 Pre-registration Infrastructure
- **PQ key registry** (`pq_map.dat`) for storing ECDSA→PQ mappings
- **OP_RETURN registration** mechanism on Bitcoin mainnet
- **Registry validation** and conflict resolution
- **Backup and synchronization** across nodes

#### 3.2 Migration Workflow
- **Dual-signature validation** during grace period
- **Automatic UTXO migration** for pre-registered keys
- **White-knight daemon** for sweeping unmigrated funds
- **Proof-of-ownership** system for fund recovery

### Phase 4: Network and Protocol

#### 4.1 RPC and API Updates
- **EBC-specific RPC calls** for key management and migration
- **Monitoring APIs** for quantum threat detection
- **Emergency Council** governance interfaces
- **White-knight operation** management

#### 4.2 Wallet and User Interface
- **PQ key generation** and management
- **EBC address creation** and validation
- **Migration transaction** creation and signing
- **Backup and recovery** for PQ keys

### Phase 5: Testing and Deployment

#### 5.1 Testing Infrastructure
- **Comprehensive test scenarios** for quantum threat simulation
- **Annual Q-day drills** implementation
- **Cutover process validation** without disrupting Bitcoin
- **Performance benchmarking** for PQ operations

#### 5.2 Documentation and Training
- **User migration guides** and tutorials
- **Developer documentation** for EBC APIs
- **Emergency procedures** documentation
- **Recovery protocols** and best practices

## Implementation Uncertainties and Questions

### 1. Post-Quantum Library Integration
- Should we use liboqs, or implement Dilithium/FALCON directly?
- What are the exact signature size requirements and performance implications?
- How do we handle potential future PQ algorithm updates?

### 2. Snapshot Mechanism
- How do we efficiently create and verify the Bitcoin UTXO snapshot?
- What's the exact format for encoding the snapshot in the genesis block?
- How do we handle potential snapshot corruption or disputes?

### 3. Emergency Trigger System
- What constitutes sufficient proof of quantum breakthrough for activation?
- How do we prevent false triggers or malicious activation attempts?
- What's the exact Emergency Council governance mechanism?

### 4. White-Knight Implementation
- What are the legal and technical requirements for the custodial rescue pool?
- How do we implement the 30-year reclaim window securely?
- What proof-of-ownership mechanisms are acceptable?

### 5. Performance and Scalability
- What are the transaction size implications of PQ signatures?
- How do we optimize block validation with larger signatures?
- What are the storage requirements for the PQ key registry?

### 6. Testing and Validation
- How do we create comprehensive test scenarios for quantum threat simulation?
- What are the requirements for the annual Q-day drills?
- How do we validate the entire cutover process without disrupting Bitcoin?

## Technical Specifications

### Post-Quantum Signature Schemes
- **CRYSTALS-Dilithium (mode 3)**: Primary scheme, ~2420 byte signatures
- **FALCON**: Fallback scheme, ~690 byte signatures
- **Hybrid mode**: Support for ECDSA+PQ dual signatures during grace period

### Address Format
- **EBC addresses**: Use `ebc1` prefix with Bech32m encoding
- **Backward compatibility**: Legacy addresses disabled post-grace period
- **Version bits**: Support for future PQ algorithm upgrades

### Chain Parameters
- **Block time**: 10 minutes (unchanged from Bitcoin)
- **Difficulty adjustment**: Every 2016 blocks with quantum-resistance factor
- **Subsidy schedule**: Identical to Bitcoin (21M coin cap)
- **Grace period**: 30 days for dual-signature transactions

### Network Protocol
- **Magic bytes**: Different from Bitcoin for replay protection
- **Port**: 8444 (EBC mainnet), 18444 (EBC testnet)
- **Protocol version**: Based on Bitcoin Core 25.x with EBC extensions
- **Message types**: Extended for PQ operations and emergency signaling

## File Structure Changes

### New Files
```
src/crypto/pq/
├── dilithium.h/cpp          # CRYSTALS-Dilithium implementation
├── falcon.h/cpp             # FALCON implementation
├── pq_keys.h/cpp            # PQ key management
└── pq_registry.h/cpp        # Key registration system

src/script/
├── pq_interpreter.h/cpp     # PQ signature verification
└── pq_script.h/cpp          # PQ script operations

src/chainparams_ebc.cpp      # EBC chain parameters
src/ebc/
├── migration.h/cpp          # UTXO migration logic
├── whitknight.h/cpp         # Sweep mechanism
└── emergency.h/cpp          # Emergency activation

src/rpc/ebc.cpp              # EBC-specific RPC calls
```

### Modified Files
```
src/script/script.h          # Add OP_PQCHECKSIG
src/script/interpreter.cpp   # PQ signature verification
src/key_io.cpp              # EBC address encoding
src/chainparams.cpp         # EBC chain registration
src/validation.cpp          # EBC consensus rules
src/net.cpp                 # EBC network protocol
```

## Dependencies

### External Libraries
- **liboqs**: Open Quantum Safe library for PQ cryptography
- **Existing**: All current Bitcoin Core dependencies remain

### Build System Changes
- **CMakeLists.txt**: Add liboqs dependency and EBC-specific targets
- **vcpkg.json**: Include PQ cryptography packages
- **CI/CD**: Extended test suites for EBC functionality

## Security Considerations

### Cryptographic Security
- **Algorithm agility**: Support for multiple PQ schemes
- **Hybrid signatures**: ECDSA+PQ during transition
- **Key rotation**: Mechanisms for PQ key updates
- **Quantum-safe hashing**: SHA-3 consideration for future

### Network Security
- **Replay protection**: Different chain IDs and magic bytes
- **Emergency activation**: Multi-signature Emergency Council
- **False trigger prevention**: Robust quantum threat verification
- **Governance sunset**: Time-limited emergency powers

### Economic Security
- **Identical monetary policy**: Preserve Bitcoin's economic properties
- **UTXO preservation**: Maintain all balances from snapshot
- **White-knight incentives**: Economic model for rescue operations
- **Recovery mechanisms**: Long-term fund reclamation process

## Timeline and Milestones

### Q3 2025: Foundation (Months 1-3)
- [ ] Post-quantum cryptography integration
- [ ] OP_PQCHECKSIG implementation
- [ ] EBC address format
- [ ] Basic chainparams

### Q4 2025: Core Features (Months 4-6)
- [ ] Key migration system
- [ ] Dual-signature validation
- [ ] Emergency activation mechanism
- [ ] White-knight sweep logic

### Q1 2026: Network Integration (Months 7-9)
- [ ] EBC network protocol
- [ ] RPC API implementation
- [ ] Wallet integration
- [ ] Testing infrastructure

### Q2 2026: Testing and Validation (Months 10-12)
- [ ] Comprehensive test suite
- [ ] Security audit preparation
- [ ] Performance optimization
- [ ] Documentation completion

### Q3 2026: Security Audit and Release
- [ ] Third-party security audit (Trail of Bits)
- [ ] Release Candidate 1
- [ ] Community review and feedback
- [ ] Final release preparation

## Success Criteria

### Technical Criteria
- [ ] All EBC transactions validate correctly
- [ ] PQ signatures verify successfully
- [ ] Address encoding/decoding works properly
- [ ] Chain synchronization functions normally
- [ ] Emergency activation triggers correctly

### Performance Criteria
- [ ] Block validation time < 2x Bitcoin Core
- [ ] Transaction throughput maintained
- [ ] Memory usage < 1.5x Bitcoin Core
- [ ] Network synchronization speed acceptable
- [ ] PQ operations complete within timeout

### Security Criteria
- [ ] No critical vulnerabilities found in audit
- [ ] Emergency Council governance functions properly
- [ ] White-knight operations secure and auditable
- [ ] Key migration process preserves funds
- [ ] Replay protection prevents cross-chain attacks

## Risk Mitigation

### Technical Risks
- **PQ algorithm changes**: Implement algorithm agility
- **Performance degradation**: Optimize critical paths
- **Implementation bugs**: Comprehensive testing and auditing
- **Compatibility issues**: Maintain Bitcoin Core compatibility

### Economic Risks
- **Market confusion**: Clear communication and education
- **Exchange support**: Early engagement with major exchanges
- **Mining incentives**: Preserve economic model
- **Adoption challenges**: User-friendly migration tools

### Governance Risks
- **Emergency Council capture**: Diverse, reputable membership
- **False activation**: Robust trigger mechanisms
- **Sunset failure**: Automatic governance transition
- **Community split**: Transparent development process

## Next Steps

1. **Immediate (Week 1-2)**
   - Set up development environment
   - Integrate liboqs library
   - Implement basic PQ key operations

2. **Short-term (Month 1)**
   - Complete OP_PQCHECKSIG implementation
   - Implement EBC address format
   - Create basic EBC chainparams

3. **Medium-term (Months 2-3)**
   - Develop key migration system
   - Implement dual-signature validation
   - Create emergency activation mechanism

4. **Long-term (Months 4-12)**
   - Complete network protocol changes
   - Implement white-knight sweep system
   - Develop comprehensive testing suite
   - Prepare for security audit

This implementation plan provides a roadmap for creating a quantum-safe emergency backup for Bitcoin while maintaining compatibility and preserving the network's core properties. 