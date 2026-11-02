#include "uiadapter.h"
#include "rosadapter.h"



//
UIAdapter::UIAdapter( QObject* parent )
    :QObject( parent )
    ,_rosadapter( std::make_shared<rosadapter>() )
{
    connect( _rosadapter.get(), &rosadapter::topicsReceived, this, [this]( const QVariantList& topics ){
        _types.clear();
        for ( const auto& value : topics ){
            const auto entry = value.toMap();
            _types.insert( entry.value( "topicName" ).toString(), entry.value( "topicTypes" ).toStringList() );
        }
        emit topicsReceived( topics );
    } );
    connect( _rosadapter.get(), &rosadapter::subscriptionChanged, this, &UIAdapter::subscriptionChanged );
    connect( _rosadapter.get(), &rosadapter::latestReceived, this, &UIAdapter::latestReceived );
    connect( _rosadapter.get(), &rosadapter::schemas, this, &UIAdapter::schemas );
    connect( _rosadapter.get(), &rosadapter::errorOccurred, this, &UIAdapter::errorOccurred );
}

//
void UIAdapter::sendCommand( Commands::ROS_CMD command ){
    QString type;
    if ( command == Commands::RC_SUBSCRIBE ){
        const auto types = _types.value( _currTopic );
        if ( types.size() != 1 ){
            emit errorOccurred( "Select a topic with exactly one advertised type" );
            return;
        }
        type = types.front();
    }
    _rosadapter->sendCommand( command, _currTopic, type );
}

//
void UIAdapter::getLatest(){
    _rosadapter->getLatest( _currTopic );
}

//
QString UIAdapter::currentTopic() const{
    return _currTopic;
}

//
void UIAdapter::setCurrentTopic( const QString& topic ){
    if ( _currTopic == topic ){
        return;
    }
    _currTopic = topic;
    emit currentTopicChanged( topic );
}

//
void UIAdapter::getSchemas(){
    _rosadapter->getSchemas();
}