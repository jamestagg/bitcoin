// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#ifdef ENABLE_EBC
#include <crypto/pq/pq_keys.h>
#include <ebc/ebc_address.h>
#include <script/script.h>
#include <test/util/setup_common.h>

BOOST_FIXTURE_TEST_SUITE(ebc_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(pq_key_generation_test)
{
    // Test Dilithium key generation
    auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);
    
    BOOST_CHECK(privkey.IsValid());
    BOOST_CHECK(pubkey.IsValid());
    BOOST_CHECK_EQUAL(pubkey.GetAlgorithm(), pq::PQAlgorithm::DILITHIUM3);
    
    // Test that we can derive the public key from private key
    auto derived_pubkey = privkey.GetPubKey();
    BOOST_CHECK(derived_pubkey.IsValid());
    BOOST_CHECK_EQUAL(derived_pubkey.GetAlgorithm(), pq::PQAlgorithm::DILITHIUM3);
}

BOOST_AUTO_TEST_CASE(pq_signature_test)
{
    // Generate a key pair
    auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);
    BOOST_REQUIRE(privkey.IsValid() && pubkey.IsValid());
    
    // Create a test message
    std::vector<unsigned char> message = {0x01, 0x02, 0x03, 0x04};
    
    // Sign the message
    auto signature_data = privkey.Sign(message);
    BOOST_CHECK(!signature_data.empty());
    
    // Create signature object
    pq::PQSignature signature(signature_data, pq::PQAlgorithm::DILITHIUM3);
    BOOST_CHECK(signature.IsValid());
    
    // Verify the signature
    BOOST_CHECK(signature.Verify(message, pubkey));
    
    // Test with wrong message
    std::vector<unsigned char> wrong_message = {0x05, 0x06, 0x07, 0x08};
    BOOST_CHECK(!signature.Verify(wrong_message, pubkey));
}

BOOST_AUTO_TEST_CASE(ebc_address_test)
{
    // Generate a PQ key pair
    auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);
    BOOST_REQUIRE(privkey.IsValid() && pubkey.IsValid());
    
    // Create EBC address from public key
    auto address = EBCAddress::FromPQPubKey(pubkey);
    BOOST_CHECK(address.IsValid());
    BOOST_CHECK(address.IsP2PQPKH());
    BOOST_CHECK_EQUAL(address.GetAlgorithm(), pq::PQAlgorithm::DILITHIUM3);
    
    // Test address string encoding/decoding
    std::string addr_str = address.ToString();
    BOOST_CHECK(!addr_str.empty());
    BOOST_CHECK(addr_str.substr(0, 4) == "ebc1");
    
    // Decode the address back
    auto decoded_address = EBCAddress::FromString(addr_str);
    BOOST_CHECK(decoded_address.IsValid());
    BOOST_CHECK(decoded_address == address);
}

BOOST_AUTO_TEST_CASE(ebc_script_test)
{
    // Generate a PQ key pair
    auto [privkey, pubkey] = pq::PQPrivKey::GenerateKeyPair(pq::PQAlgorithm::DILITHIUM3);
    BOOST_REQUIRE(privkey.IsValid() && pubkey.IsValid());
    
    // Create EBC address and get its script
    auto address = EBCAddress::FromPQPubKey(pubkey);
    BOOST_REQUIRE(address.IsValid());
    
    auto script = address.GetScript();
    BOOST_CHECK(!script.empty());
    
    // Check that the script contains OP_PQCHECKSIG
    bool found_pqchecksig = false;
    for (auto it = script.begin(); it != script.end(); ++it) {
        if (*it == OP_PQCHECKSIG) {
            found_pqchecksig = true;
            break;
        }
    }
    BOOST_CHECK(found_pqchecksig);
}

BOOST_AUTO_TEST_CASE(pq_algorithm_utils_test)
{
    // Test algorithm string conversion
    BOOST_CHECK_EQUAL(pq::util::AlgorithmToString(pq::PQAlgorithm::DILITHIUM3), "Dilithium3");
    BOOST_CHECK_EQUAL(pq::util::AlgorithmToString(pq::PQAlgorithm::FALCON512), "Falcon512");
    BOOST_CHECK_EQUAL(pq::util::AlgorithmToString(pq::PQAlgorithm::UNKNOWN), "Unknown");
    
    BOOST_CHECK_EQUAL(pq::util::StringToAlgorithm("Dilithium3"), pq::PQAlgorithm::DILITHIUM3);
    BOOST_CHECK_EQUAL(pq::util::StringToAlgorithm("Falcon512"), pq::PQAlgorithm::FALCON512);
    BOOST_CHECK_EQUAL(pq::util::StringToAlgorithm("Invalid"), pq::PQAlgorithm::UNKNOWN);
    
    // Test supported algorithms
    auto supported = pq::util::GetSupportedAlgorithms();
    BOOST_CHECK(!supported.empty());
}

BOOST_AUTO_TEST_CASE(ebc_address_validation_test)
{
    // Test invalid EBC address strings
    BOOST_CHECK(!ebc_address::IsValidEBCAddress(""));
    BOOST_CHECK(!ebc_address::IsValidEBCAddress("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"));
    BOOST_CHECK(!ebc_address::IsValidEBCAddress("invalid"));
    
    // Test HRP
    BOOST_CHECK_EQUAL(ebc_address::GetHRP(), "ebc1");
}

BOOST_AUTO_TEST_SUITE_END()

#else // !ENABLE_EBC

// Dummy test when EBC is not enabled
BOOST_AUTO_TEST_SUITE(ebc_tests_disabled)

BOOST_AUTO_TEST_CASE(ebc_disabled_test)
{
    // EBC functionality is disabled
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

#endif // ENABLE_EBC 