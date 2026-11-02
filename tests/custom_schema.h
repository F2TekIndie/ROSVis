#pragma once

#include "ros_options.pb.h"

//
namespace TestSchema {
    inline google::protobuf::FileDescriptorProto descriptor(){
        google::protobuf::FileDescriptorProto file;
        file.set_name( "custom_vehicle.proto" );
        file.set_package( "custom.vehicle" );
        file.set_syntax( "proto3" );
        file.add_dependency( "ros_options.proto" );
        auto* message = file.add_message_type();
        message->set_name( "Telemetry" );
        message->mutable_options()->SetExtension( rosdata::ros_type, "custom_vehicle/msg/Telemetry" );
        auto* field = message->add_field();
        field->set_name( "counter" );
        field->set_number( 1 );
        field->set_type( google::protobuf::FieldDescriptorProto::TYPE_UINT64 );
        field->set_label( google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL );
        return file;
    }
} // namespace TestSchema
