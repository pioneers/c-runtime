/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: dev_data.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "dev_data.pb-c.h"
void   dev_data__param__init
                     (DevData__Param         *message)
{
  static const DevData__Param init_value = DEV_DATA__PARAM__INIT;
  *message = init_value;
}
void   dev_data__device__init
                     (DevData__Device         *message)
{
  static const DevData__Device init_value = DEV_DATA__DEVICE__INIT;
  *message = init_value;
}
void   dev_data__init
                     (DevData         *message)
{
  static const DevData init_value = DEV_DATA__INIT;
  *message = init_value;
}
size_t dev_data__get_packed_size
                     (const DevData *message)
{
  assert(message->base.descriptor == &dev_data__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t dev_data__pack
                     (const DevData *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &dev_data__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t dev_data__pack_to_buffer
                     (const DevData *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &dev_data__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
DevData *
       dev_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (DevData *)
     protobuf_c_message_unpack (&dev_data__descriptor,
                                allocator, len, data);
}
void   dev_data__free_unpacked
                     (DevData *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &dev_data__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor dev_data__param__field_descriptors[4] =
{
  {
    "name",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(DevData__Param, name),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "fval",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    offsetof(DevData__Param, val_case),
    offsetof(DevData__Param, fval),
    NULL,
    NULL,
    0 | PROTOBUF_C_FIELD_FLAG_ONEOF,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ival",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    offsetof(DevData__Param, val_case),
    offsetof(DevData__Param, ival),
    NULL,
    NULL,
    0 | PROTOBUF_C_FIELD_FLAG_ONEOF,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "bval",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BOOL,
    offsetof(DevData__Param, val_case),
    offsetof(DevData__Param, bval),
    NULL,
    NULL,
    0 | PROTOBUF_C_FIELD_FLAG_ONEOF,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned dev_data__param__field_indices_by_name[] = {
  3,   /* field[3] = bval */
  1,   /* field[1] = fval */
  2,   /* field[2] = ival */
  0,   /* field[0] = name */
};
static const ProtobufCIntRange dev_data__param__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor dev_data__param__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "DevData.Param",
  "Param",
  "DevData__Param",
  "",
  sizeof(DevData__Param),
  4,
  dev_data__param__field_descriptors,
  dev_data__param__field_indices_by_name,
  1,  dev_data__param__number_ranges,
  (ProtobufCMessageInit) dev_data__param__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor dev_data__device__field_descriptors[4] =
{
  {
    "type",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(DevData__Device, type),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "uid",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(DevData__Device, uid),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "itype",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(DevData__Device, itype),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "params",
    4,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(DevData__Device, n_params),
    offsetof(DevData__Device, params),
    &dev_data__param__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned dev_data__device__field_indices_by_name[] = {
  2,   /* field[2] = itype */
  3,   /* field[3] = params */
  0,   /* field[0] = type */
  1,   /* field[1] = uid */
};
static const ProtobufCIntRange dev_data__device__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor dev_data__device__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "DevData.Device",
  "Device",
  "DevData__Device",
  "",
  sizeof(DevData__Device),
  4,
  dev_data__device__field_descriptors,
  dev_data__device__field_indices_by_name,
  1,  dev_data__device__number_ranges,
  (ProtobufCMessageInit) dev_data__device__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor dev_data__field_descriptors[1] =
{
  {
    "devices",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(DevData, n_devices),
    offsetof(DevData, devices),
    &dev_data__device__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned dev_data__field_indices_by_name[] = {
  0,   /* field[0] = devices */
};
static const ProtobufCIntRange dev_data__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor dev_data__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "DevData",
  "DevData",
  "DevData",
  "",
  sizeof(DevData),
  1,
  dev_data__field_descriptors,
  dev_data__field_indices_by_name,
  1,  dev_data__number_ranges,
  (ProtobufCMessageInit) dev_data__init,
  NULL,NULL,NULL    /* reserved[123] */
};
