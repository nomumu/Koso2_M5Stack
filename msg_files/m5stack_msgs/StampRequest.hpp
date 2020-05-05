// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*! 
 * @file StampRequest.hpp
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _M5STACK_MSGS_STAMP_REQUEST_HPP_
#define _M5STACK_MSGS_STAMP_REQUEST_HPP_

#include "../topic.hpp"

namespace m5stack_msgs {

class StampRequest : public ros2::Topic<StampRequest>
{
public:
    uint16_t seq;
    uint16_t type;
    uint16_t index;

    StampRequest():
        Topic("m5stack_msgs::msg::dds_::StampRequest_", "StampRequest", M5STACK_MSGS_STAMP_REQUEST_ID)
    {
    }
    bool serialize(void* msg_buf, const StampRequest* topic)
    {
        ucdrBuffer* writer = (ucdrBuffer*)msg_buf;
        (void) ucdr_serialize_uint16_t(writer, topic->seq);
        (void) ucdr_serialize_uint16_t(writer, topic->type);
        (void) ucdr_serialize_uint16_t(writer, topic->index);

        return !writer->error;
    }

    bool deserialize(void* msg_buf, StampRequest* topic)
    {
        ucdrBuffer* reader = (ucdrBuffer*)msg_buf;
        (void) ucdr_deserialize_uint16_t(reader, &topic->seq);
        (void) ucdr_deserialize_uint16_t(reader, &topic->type);
        (void) ucdr_deserialize_uint16_t(reader, &topic->index);

        return !reader->error;
    }
    uint32_t size_of_topic(const StampRequest* topic, uint32_t size)
    {
        uint32_t previousSize = size;
        size += ucdr_alignment(size, 2) + 2;
        size += ucdr_alignment(size, 2) + 2;
        size += ucdr_alignment(size, 2) + 2;

        return size - previousSize;
    }
};
}
#endif // _M5STACK_MSGS_STAMP_REQUEST_HPP_