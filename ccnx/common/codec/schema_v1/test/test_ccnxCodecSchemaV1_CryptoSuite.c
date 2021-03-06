/*
 * Copyright (c) 2014-2015, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnxCodecSchemaV1_CryptoSuite.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <stdio.h>

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_CryptoSuite)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_CryptoSuite)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_CryptoSuite)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_ParcToTlv);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_TlvToParc);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_ParcToTlv)
{
    struct test_vector {
        PARCCryptoSuite input;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite output;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .input = PARCCryptoSuite_RSA_SHA256,  .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,  .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_HMAC_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256, .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_NULL_CRC32C, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,     .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_DSA_SHA256,  .output = 0,                                                     .success = false, .sentinel = false },
        { .input = PARCCryptoSuite_RSA_SHA512,  .output = 0,                                                     .success = false, .sentinel = false },
        { .input = PARCCryptoSuite_HMAC_SHA512, .output = 0,                                                     .success = false, .sentinel = false },
        { .input = 13579,                       .output = 0,                                                     .success = false, .sentinel = false },
        { .input = 0,                           .output = 0,                                                     .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_ParcToTlv(vectors[i].input, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_TlvToParc)
{
    struct test_vector {
        PARCCryptoSuite output;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite input;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,   .output = PARCCryptoSuite_RSA_SHA256,  .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256,  .output = PARCCryptoSuite_HMAC_SHA256, .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,      .output = PARCCryptoSuite_NULL_CRC32C, .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_EcSecp256K1, .output = 0,                           .success = false, .sentinel = false },
        { .input = 13579,                                                  .output = 0,                           .success = false, .sentinel = false },
        { .input = 0,                                                      .output = 0,                           .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_TlvToParc(vectors[i].input, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv)
{
    struct test_vector {
        PARCSigningAlgorithm signAlg;
        PARCCryptoHashType hashType;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite output;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .signAlg = PARCSigningAlgorithm_RSA,  .hashType = PARC_HASH_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,  .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_HMAC, .hashType = PARC_HASH_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256, .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgortihm_NULL, .hashType = PARC_HASH_CRC32C, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,     .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_RSA,  .hashType = 12345,            .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_HMAC, .hashType = 12345,            .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = PARCSigningAlgortihm_NULL, .hashType = 12345,            .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = 12345,                     .hashType = 12345,            .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = 0,                         .hashType = 0,                .output = 0,                                                     .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv(vectors[i].signAlg, vectors[i].hashType, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_CryptoSuite);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
