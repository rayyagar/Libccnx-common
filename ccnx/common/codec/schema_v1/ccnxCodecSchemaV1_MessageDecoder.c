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
/**
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright 2014-2015, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_MessageDecoder.h>

#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>
#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/ccnx_InterestReturn.h>

static bool
_translateWirePayloadTypeToCCNxPayloadType(CCNxCodecSchemaV1Types_PayloadType wireFormatType, CCNxPayloadType *payloadTypePtr)
{
    bool success = true;
    switch (wireFormatType) {
        case CCNxCodecSchemaV1Types_PayloadType_Data:
            *payloadTypePtr = CCNxPayloadType_DATA;
            break;

        case CCNxCodecSchemaV1Types_PayloadType_Key:
            *payloadTypePtr = CCNxPayloadType_KEY;
            break;

        case CCNxCodecSchemaV1Types_PayloadType_Link:
            *payloadTypePtr = CCNxPayloadType_LINK;
            break;

        default:
            // unknown type
            success = false;
    }
    return success;
}

/**
 * Translates the wire format value for the PayloadType to CCNxPayloadType
 */
static bool
_decodePayloadType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t length)
{
    CCNxPayloadType payloadType;

    uint64_t wireFormatVarInt;
    bool success = ccnxCodecTlvDecoder_GetVarInt(decoder, length, &wireFormatVarInt);
    if (success) {
        CCNxCodecSchemaV1Types_PayloadType wireFormatType = (CCNxCodecSchemaV1Types_PayloadType) wireFormatVarInt;

        success = _translateWirePayloadTypeToCCNxPayloadType(wireFormatType, &payloadType);
    }

    if (success) {
        success = ccnxTlvDictionary_PutInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE, payloadType);
    }

    return success;
}

static bool
_decodeType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    bool success = false;
    switch (type) {
        case CCNxCodecSchemaV1Types_CCNxMessage_Name:
            success = ccnxCodecTlvUtilities_PutAsName(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_Payload:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_KeyIdRestriction:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_ContentObjectHashRestriction:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_PayloadType:
            success = _decodePayloadType(decoder, packetDictionary, length);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_ExpiryTime:
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME);
            break;

        case CCNxCodecSchemaV1Types_CCNxMessage_EndChunkNumber:
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
            break;

        default:
            // if we do not know the TLV type, put it in this container's unknown list
            success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_Lists_MESSAGE_LIST);
            break;
    }

    if (!success) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    }
    return success;
}

/*
 * We are given a decoder that points to the first TLV of a list of TLVs.  We keep walking the
 * list until we come to the end of the decoder.
 */
bool
ccnxCodecSchemaV1MessageDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    return ccnxCodecTlvUtilities_DecodeContainer(decoder, packetDictionary, _decodeType);
}

