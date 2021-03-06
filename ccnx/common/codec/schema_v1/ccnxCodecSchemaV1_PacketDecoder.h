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
 * @file ccnxCodecSchemaV1_PacketDecoder.h
 * @brief Decoder for the version 1 TLV Packet
 *
 * The Schema version 1 Dictionary is organized in containers: FixedHeader, OptionalHeaders, (Interest, ContentObject, Control), Verification.
 *
 * Each container is its own dictionary.
 *
 * Example:
 * @code
 * {
 *      CCNxTlvDictionary *packetDictionary = ccnxTlvDictionary_Create();
 *      ccnxCodecSchemaV1PacketDecoder_Decode(packetBuffer, packetDictionary);
 *      // the fields in the packetDictionary are now set
 * }
 * @endcode
 *
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright 2014-2015, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */

#ifndef CCNxCodecSchemaV1_PacketDecoder_h
#define CCNxCodecSchemaV1_PacketDecoder_h

#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>


/**
 * Decode a packet in to a dictionary.
 *
 * The buffer should be set at the start of the fixed header.  This call is equivalent
 * to ccnxCodecSchemaV1PacketDecoder_Decode(), except it allocates and destroys a temporary
 * CCNxCodecTlvDecoder.
 *
 * The dictionary will be filled in with all fields available in the packetBuffer.
 *
 * Caveat: there is no way to find out where the error was if returned "false"
 *
 * @param [in] buffer The packet buffer
 * @param [in] packetDictionary The dictionary to fill in
 *
 * @return true Successful decode
 * @return false There was an error somewhere
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
bool ccnxCodecSchemaV1PacketDecoder_BufferDecode(PARCBuffer *packetBuffer, CCNxTlvDictionary *packetDictionary);

/**
 * Decode in to in to a dictionary.
 *
 * The buffer should be set at the start of the fixed header.
 *
 * The dictionary will be filled in with all fields available in the packetDecoder.
 *
 * Caveat: there is no way to find out where the error was if returned "false"
 *
 * @param [in] buffer The packet buffer
 * @param [in] packetDictionary The dictionary to fill in
 *
 * @return true Successful decode
 * @return false There was an error somewhere
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
bool ccnxCodecSchemaV1PacketDecoder_Decode(CCNxCodecTlvDecoder *packetDecoder, CCNxTlvDictionary *packetDictionary);

#endif // CCNxCodecSchemaV1_PacketDecoder_h
