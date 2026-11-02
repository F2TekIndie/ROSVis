#include "rosdata/schema.h"
#include "ros_options.pb.h"
#include "custom_schema.h"
#include <iostream>
#include <stdexcept>



//
namespace{
    void require( bool condition, const std::string& message ){
        if ( !condition ){
            throw std::runtime_error( message );
        }
    }


} // namespace

//
int main(){
    try{
        rosdata::SchemaRegistry schemas;
        const auto types = schemas.supportedTypes();
        require( types.size() >= 143, "Standard message bundle is incomplete" );
        auto schema = schemas.schema( "sensor_msgs/msg/PointCloud2" );
        rosdata::SchemaRegistry other;
        other.loadSchema( schema );
        auto message = other.create( "sensor_msgs/msg/PointCloud2" );
        auto* field = message->GetDescriptor()->FindFieldByName( "data" );
        message->GetReflection()->SetString( message.get(), field, std::string( "\0\xff\x80", 3 ) );
        google::protobuf::Any payload;
        payload.PackFrom( *message );
        auto restored = schemas.unpack( payload );
        require( restored->GetReflection()->GetString( *restored, field ) == std::string( "\0\xff\x80", 3 ), "Binary buffer was corrupted" );

        google::protobuf::FileDescriptorSet custom;
        *custom.add_file() = TestSchema::descriptor();
        schemas.loadSchema( custom );
        auto telemetry = schemas.create( "custom_vehicle/msg/Telemetry" );
        field = telemetry->GetDescriptor()->FindFieldByName( "counter" );
        telemetry->GetReflection()->SetUInt64( telemetry.get(), field, UINT64_MAX );
        payload.PackFrom( *telemetry );
        auto exported = schemas.schema( "custom_vehicle/msg/Telemetry" );
        // Import is deliberately out of dependency order.
        google::protobuf::FileDescriptorSet reversed;
        for ( int i = exported.file_size() - 1; i >= 0; --i ){
            *reversed.add_file() = exported.file( i );
        }
        std::shared_ptr<google::protobuf::Message> dynamic;
        {
            rosdata::SchemaRegistry temporary;
            temporary.loadSchema( reversed );
            dynamic = temporary.unpack( payload );
        }
        require( dynamic->GetReflection()->GetUInt64( *dynamic, dynamic->GetDescriptor()->FindFieldByName( "counter" ) ) == UINT64_MAX,
            "Dynamic message or descriptor lifetime failed" );

        bool rejected = false;
        auto conflict = custom;
        conflict.mutable_file( 0 )->mutable_message_type( 0 )->mutable_field( 0 )->set_number( 2 );
        try{
            schemas.loadSchema( conflict );
        }
        catch ( const std::invalid_argument& ){
            rejected = true;
        }
        require( rejected, "Conflicting schema was accepted" );

        payload.set_value( std::string( "\xff", 1 ) );
        rejected = false;
        try{
            schemas.unpack( payload );
        }
        catch ( const std::invalid_argument& ){
            rejected = true;
        }
        require( rejected, "Malformed protobuf data was accepted" );
    }
    catch ( const std::exception& error ){
        std::cerr << error.what() << std::endl;
        return 1;
    }
    return 0;
}
