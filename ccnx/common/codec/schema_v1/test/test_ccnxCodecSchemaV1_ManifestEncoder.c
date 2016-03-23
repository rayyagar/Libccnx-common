/*
 * Copyright (c) 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
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
#include "../ccnxCodecSchemaV1_ManifestEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "testrig_encoder.c"

#include <ccnx/common/ccnx_Manifest.h>

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_ManifestEncoder)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_ManifestEncoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_ManifestEncoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeEmpty);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_AddPointer);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeEmpty)
{
    CCNxName *locator = ccnxName_CreateFromCString("lci:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);

    assertTrue(result == 0, "Expected an empty Manifest to be encoded to size 0, got %zu", result);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_AddPointer)
{
    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCBuffer *pointer = parcBuffer_Flip(parcBuffer_ParseHexString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
    ccnxManifestHashGroup_AddPointer(group, CCNxManifestHashGroupPointerType_Data, pointer);

    ccnxManifest_AddHashGroup(manifest, group);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);
    size_t expected = 4 + 4 + parcBuffer_Remaining(pointer); // hash group TL, pointer TL, pointer V

    assertTrue(result == expected, "Expected an empty Manifest to be encoded to size %zu, got %zu", expected, result);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    const struct iovec *vector = ccnxCodecNetworkBufferIoVec_GetArray(iovec);

    assertTrue(vector->iov_len == expected, "Expected the IO vector to contain the encoded manifest");
    assertTrue(memcmp(vector->iov_base + 8, parcBuffer_Overlay(pointer, parcBuffer_Remaining(pointer)), vector->iov_len - 8) == 0, "Expected the same pointer to be encoded");

    uint16_t expectedType = CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer;
    uint8_t *base = (uint8_t *) vector->iov_base;
    uint16_t actualType = (base[4] << 8) | base[5];
    assertTrue(expectedType == actualType, "Expected the type to be written correctly as CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    // Cleanup
    parcBuffer_Release(&pointer);
    ccnxManifestHashGroup_Release(&group);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup)
{
    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCBuffer *pointer = parcBuffer_Flip(parcBuffer_ParseHexString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
    ccnxManifestHashGroup_AddPointer(group, CCNxManifestHashGroupPointerType_Data, pointer);

    ccnxManifest_AddHashGroup(manifest, group);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);
    size_t expected = 4 + 4 + parcBuffer_Remaining(pointer); // hash group TL, pointer TL, pointer V

    assertTrue(result == expected, "Expected an empty Manifest to be encoded to size %zu, got %zu", expected, result);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    const struct iovec *vector = ccnxCodecNetworkBufferIoVec_GetArray(iovec);

    uint8_t expectedVector[24] = {0x00,0x07,0x00,0x14,0x00,0x02,0x00,0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    assertTrue(vector->iov_len == 24, "Expected the IO vector to contain the encoded manifest");
    assertTrue(memcmp(vector->iov_base, expectedVector, vector->iov_len) == 0, "Expected the same pointer to be encoded");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    // Cleanup
    parcBuffer_Release(&pointer);
    ccnxManifestHashGroup_Release(&group);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_ManifestEncoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}