<!--
Copyright (c) 2025 Valis Labs
Licensed under BSL1.1 License. See LICENSE-COMMERCIAL for details.
-->

# Emergency Bitcoin Cut-Over Chain (EBC)

**May 2025**  
**James Tagg, Valis Labs**  
**White Paper – Version 1.0**

## Abstract

The cybersecurity group at the University of Waterloo estimates a 5% probability that quantum computers will compromise Bitcoin within the next five years. At Valis Labs, we are developing a quantum bio-computer with an initial implementation targeting 5 billion qubits. The risk of a "DeepSeek event"—where a capability assumed to be decades away materializes overnight—is significant, particularly for the quantum compromise of elliptic curve cryptography. Such an event would require resolution within days, not the months typically required for Bitcoin consensus and migration.

The Emergency Bitcoin Cut-Over Chain (EBC) provides a quantum-safe backup network with pre-migrated addresses that can activate instantly upon detection of a catastrophic quantum threat. EBC preserves balances from a pre-selected snapshot, migrates all UTXOs to post-quantum (PQ) key pairs already pre-registered by users, and employs automated "white-knight" sweeps to rescue unmigrated funds. The system minimizes disruption while introducing governance mechanisms for rapid post-event upgrades and implementing long-desired network enhancements.

## 1. Background and Rationale

Recent research indicates that the earliest credible "Q-day" for breaking Bitcoin's ECDSA signatures could arrive as early as the 2030s under optimistic scenarios, with mainstream estimates extending to the 2040s. Hardware vendors are already shipping quantum-resistant devices, and proposals for mandatory wallet migration via hard fork have entered public discussion.

EBC acknowledges that achieving consensus for an in-place upgrade may arrive too late once a quantum break is demonstrated. A dormant, fully tested alternative chain provides a cryptographic fire escape for the Bitcoin ecosystem.

## 2. Threat Model

The EBC addresses four primary threat vectors:

- **Cryptanalytic Break**: Shor-class quantum attacks on ECDSA/SHA-256
- **Nation-State Censorship or Fork Capture**: Coordinated attacks on network integrity
- **Catastrophic Consensus Bug**: Chain-splitting vulnerabilities or inflation bugs
- **Emergency Governance Paralysis**: Inability to achieve timely consensus under crisis conditions

## 3. Design Principles

1. **Immediate Usability**: Pre-registered PQ keys enable instant user access
2. **Replay Immunity**: Transactions on EBC are invalid on Bitcoin and vice versa
3. **Economic Continuity**: Identical coin supply and issuance schedule
4. **White-Knight Rescue**: Automated sweeping of legacy UTXOs after grace period
5. **Minimal Code Delta**: Based on Bitcoin Core for maximum auditability

## 4. Technical Overview

### 4.1 Code Base and Language

EBC is built as a fork of Bitcoin Core 25.x (C++17). Primary modifications include:

- **PQ Signature Support**: CRYSTALS-Dilithium (mode 3) as default, FALCON as optional
- **New Script Opcode**: OP_PQCHECKSIG for quantum-resistant signature verification
- **Dual-Mode Addressing**: `ebc1` prefix for PQ addresses; legacy Bech32 disabled post-grace

Ancillary tooling (CLI wallet, sweeper, monitoring agents) utilizes Go and Python for rapid iteration and service integration.

### 4.2 Snapshot and Genesis

- **Cut-over Trigger**: Board-ratified declaration upon publication of practical break or >10 BLL (Bitcoin Log-Likelihood) alert
- **Snapshot Height**: Last stable block height N
- **Genesis Block**: EBC block 0 contains merkle root of Bitcoin block N plus hash of published trigger proof

### 4.3 Key Migration Workflow

1. **Pre-registration Phase** (Live Q3 2025):
   - Users sign messages with legacy ECDSA keys linking to PQ public keys
   - Broadcast via OP_RETURN on Bitcoin mainnet
   - Registrations stored in crawlable registry (`pq_map.dat`)

2. **Grace Period** (T = 30 days after cut-over):
   - Legacy UTXOs spendable only via co-signing with both ECDSA and PQ keys

3. **Sweep Period** (T + 180 days):
   - White-knight daemon sweeps unmigrated UTXOs to custodial rescue pool
   - Multisig control by reputable institutions
   - 30-year reclaim window with authenticated proof-of-ownership

### 4.4 Consensus and Economics

- **Block Parameters**: Interval, subsidy schedule, and halving cadence unchanged
- **Difficulty Algorithm**: Frozen at last Bitcoin difficulty for first 2016 blocks, then normal retargeting with one-bit increment for Grover's algorithm impact
- **Mempool Rules**: PQ-only inputs enforced after grace period
- **Miner Incentives**: 5% of block reward redirected during first six months for white-knight operations and security bounties

### 4.5 Governance

- **Emergency Council (EC)**: Nine-member multisig (5-of-9) comprising core developers, major custodians, and academic experts
- **Limited Mandate**: EC authority for emergency releases and parameter hot-fixes expires after 12 months
- **Transition**: Default to full on-chain BIP-style governance post-emergency period
- **Checkpointing**: Signed actions checkpointed every 144 blocks using OP_CHECKDATASIG

## 5. Cut-Over Procedure

| Phase | Actor | Action |
|-------|-------|--------|
| -12 mo | Users | Begin PQ key pre-registration |
| -3 mo | Exchanges | Dry-run snapshot extraction; list EBC markets |
| T₀ | EC | Publish quantum break proof; broadcast cut-over height |
| T₀+1 block | Miners | Switch to EBC software; mine first PQ-only block |
| T₀→T₀+30d | Users | Spend legacy coins using dual-sig to PQ addresses |
| T₀+30d | Protocol | Disable pure ECDSA spends |
| T₀+210d | White-knight | Execute final sweeps; publish audit |

## 6. Risk Analysis

### Technical Risks
- **Split-Market Risk**: Parallel BTC/EBC trading mitigated by coordinated exchange listing
- **Implementation Risk**: PQ cryptography maturity addressed via dual-algorithm fallback
- **Legal Uncertainty**: Rescue pool operations follow lost-property precedent

### Legal Framework
The white-knight sweep operates under established lost-property principles: finding and securing lost property for return to rightful owners does not constitute theft or trigger financial custodian regulations. Security and due process ensure proper reunification without executing transfers or deposits.

## 7. Roadmap and Milestones

1. **Q3 2025**: PQ key registry mainnet pilot launch
2. **Q4 2025**: Dilithium/Falcon integration into Core fork
3. **Q1 2026**: Public testnet with simulated Q-day event
4. **Q2 2026**: Third-party security audit (Trail of Bits)
5. **Q3 2026**: Release Candidate 1
6. **Ongoing**: Annual dry-run drills every April 26 (Bitcoin whitepaper anniversary)

## 8. Future Work

- Integrate lattice-based threshold signatures for multisig wallets
- Research zk-SNARK-based proof-of-ownership for post-sweep reclamations
- Enable hybrid time-lock contracts for automatic redemption on secure chain
- Develop Quantum Gravity Computer Safe Cryptography using entropic/proof-of-proof methods

## 9. Conclusion

EBC serves as an insurance policy: dormant and cost-effective until Bitcoin's foundational cryptography faces compromise. By combining pre-emptive PQ key registration, auditable cut-over scripts, and limited-mandate emergency governance, EBC provides a technically sound escape mechanism activatable within a single block confirmation. Community participation through key pre-registration today determines the seamlessness of tomorrow's potential rescue operation.

## References

1. Forbes Business Council. "Quantum Computing: A New Threat To Bitcoin And Crypto Security?" Forbes, April 4, 2025.

2. Chojecki, P. "Quantum Computers' Threat to Bitcoin." Medium, May 20, 2025. [Note: Specific article not found in search results]

3. Cruz, A. "Bitcoin Developer Proposes Hard Fork to Protect BTC From Quantum Threats." CoinDesk, April 5, 2025.

4. SEALSQ Corp. "SEALSQ Unveils Quantum-Resistant Cryptography with QS7001." Press Release, May 21, 2025.

## Appendix I: Why Planned Migration May Arrive Too Late

Bitcoin's consensus mechanism operates on geological timescales, not crisis timescales. Historical precedent demonstrates this clearly:

| Proposal | Timeline | Notes |
|----------|----------|-------|
| **Segregated Witness (BIP-141)** | Mar 2015 → Aug 2017 (≈2.5 years) | Required 95% miner signaling amid contentious debate |
| **Taproot (BIPs 340-342)** | Jan 2018 → Nov 2021 (≈4 years) | Smoother consensus but extensive review period |
| **BIP-119 (OP_CTV)** | Jan 2020 → Still pending (5+ years) | Demonstrates how minor changes stall indefinitely |
| **SegWit2x** | May 2017 → Abandoned Nov 2017 | Failed despite strong backing |

### Structural Impediments to Rapid Change

1. **Decentralized Governance**: No formal voting body; rough consensus must emerge organically
2. **Security Through Conservatism**: Changes treated as existential risks requiring extensive validation
3. **Deployment Mechanics**: Even soft forks require BIP drafting, implementation, testing, signaling periods, and lock-in delays
4. **Ecosystem Coordination**: Thousands of wallets, exchanges, and services must update independently

A full UTXO migration to post-quantum cryptography would constitute Bitcoin's largest change ever, likely requiring longer than SegWit or Taproot combined.

### EBC's Advantage

The dormant cut-over chain sidesteps social consensus bottlenecks:

- **Opt-in Activation**: Stakeholders pivot at crisis point rather than negotiating rule changes
- **Pre-positioned Code**: Published, audited, and drilled in advance
- **Off-chain Trigger**: Emergency Council signs "break has happened" statement
- **Replay Immunity**: Different chain ID and address format enable zero-risk switching

With proper preparation, cut-over can occur within one block rather than years.

## Appendix II: Why Migration Speedups Are Impossible

### Capacity Constraints

| Migration Step | Bytes per UTXO | Current UTXOs | Raw Data | Timeline at 1MB/block |
|----------------|----------------|---------------|----------|----------------------|
| Standard P2WPKH | ≈110 vB | 80 million | ~9 GB | ~65 days |
| Dilithium-3 sig | ≈675 vB | 80 million | ~54 GB | ~380 days |
| Full PQ migration | ≈3,300 vB | 80 million | ~260 GB | **~5 years** |

### Why Optimizations Fall Short

- **Batching**: 100-input transactions reduce overhead 50x but require complex coordination
- **Script Hacks**: Can compress PQ key storage but signatures remain large
- **Bigger Blocks**: Requires another contentious hard fork

### EBC Solution

1. **Off-chain Snapshot**: UTXO set hashed into genesis block (no Bitcoin bandwidth)
2. **No Migration Transactions**: Users prove ownership on new chain
3. **Graceful Fallback**: White-knight sweep uses only EBC block space

## Appendix III: Competitive Analysis

### Existing Approaches

| Project | Strengths | Limitations |
|---------|-----------|-------------|
| **Bitcoin Post-Quantum (BPQ)** | Proven snapshot fork with XMSS | Always live, no white-knight sweep |
| **Quantum Resistant Ledger (QRL)** | Purpose-built PQ chain | No Bitcoin balance linkage |
| **Post-Quantum Registry (PQR)** | Pre-registration concept | No implementation |
| **P2QRH/QRAMP BIPs** | In-protocol PQ support | Multi-year activation path |
| **Emergency Fork Discussions** | Conceptual alignment | No maintained code or drills |

### EBC's Unique Position

No existing project combines:
- Compiled and signed Core fork remaining dormant until triggered
- Automated white-knight sweeper with custody procedures
- Standing testnet running annual Q-day drills
- Formal governance time-capsule with sunset provisions

EBC fills this critical gap in Bitcoin's quantum preparedness infrastructure.