/*
** Copyright (c) 2018 LunarG, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef BRIMSTONE_FORMAT_POINTER_DECODER_H
#define BRIMSTONE_FORMAT_POINTER_DECODER_H

#include <cassert>
#include <limits>
#include <memory>

#include "util/defines.h"
#include "format/format.h"
#include "format/pointer_decoder_base.h"
#include "format/value_decoder.h"

BRIMSTONE_BEGIN_NAMESPACE(brimstone)
BRIMSTONE_BEGIN_NAMESPACE(format)

template<typename T>
class PointerDecoder : public PointerDecoderBase
{
public:
    PointerDecoder() { }

    ~PointerDecoder() { }

    T* GetPointer() const { return data_.get(); }

    size_t Decode(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<T>(buffer, buffer_size); }

    // Decode for special types that may require conversion.
    size_t DecodeEnum(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<EnumEncodeType>(buffer, buffer_size); }
    size_t DecodeFlags(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<FlagsEncodeType>(buffer, buffer_size); }
    size_t DecodeSampleMask(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<SampleMaskEncodeType>(buffer, buffer_size); }
    size_t DecodeHandle(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<HandleEncodeType>(buffer, buffer_size); }
    size_t DecodeDeviceSize(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<DeviceSizeEncodeType>(buffer, buffer_size); }
    size_t DecodeSizeT(const uint8_t* buffer, size_t buffer_size) { return DecodeFrom<SizeTEncodeType>(buffer, buffer_size); }

private:
    template <typename SrcT>
    size_t DecodeFrom(const uint8_t* buffer, size_t buffer_size)
    {
        size_t bytes_read = 0;

        bytes_read += ValueDecoder::DecodeUInt32Value((buffer + bytes_read), (buffer_size - bytes_read), &attrib_);

        assert((attrib_ & (PointerAttributes::kIsString | PointerAttributes::kIsArray)) != (PointerAttributes::kIsString | PointerAttributes::kIsArray));

        if ((attrib_ & PointerAttributes::kIsNull) != PointerAttributes::kIsNull)
        {
            if ((attrib_ & PointerAttributes::kHasAddress) == PointerAttributes::kHasAddress)
            {
                bytes_read += ValueDecoder::DecodeAddress((buffer + bytes_read), (buffer_size - bytes_read), &address_);
            }

            if (((attrib_ & PointerAttributes::kIsArray) == PointerAttributes::kIsArray) ||
                ((attrib_ & PointerAttributes::kIsString) == PointerAttributes::kIsString))
            {
                bytes_read += ValueDecoder::DecodeSizeTValue((buffer + bytes_read), (buffer_size - bytes_read), &len_);
            }
            else
            {
                len_ = 1;
            }

            if (((attrib_ & PointerAttributes::kHasData) == PointerAttributes::kHasData))
            {
                data_ = std::make_unique<T[]>(len_);

                bytes_read += ValueDecoder::DecodeArrayFrom<SrcT>(buffer, buffer_size, data_->get(), len_);
            }
        }

        return bytes_read;
    }

private:
    std::unique_ptr<T[]>    data_;
};

BRIMSTONE_END_NAMESPACE(format)
BRIMSTONE_END_NAMESPACE(brimstone)

#endif // BRIMSTONE_FORMAT_POINTER_DECODER_H
