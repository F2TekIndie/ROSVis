

#include "uiadapter.h"
#include <boost/asio.hpp>
#include <QVariant>
#include "rosadapter.h"



//
UIAdapter::UIAdapter(QObject* parent)
    :QObject(parent)
{
    _rosadapter.reset( new rosadapter() );
    //
    connect( _rosadapter.get(), &rosadapter::newTopic, this, &UIAdapter::handleNewTopic );
}

void UIAdapter::sendCommand( Commands::ROS_CMD nCmd ){
    std::string sCmd{ "" };
    switch( nCmd ){
        case Commands::RC_GET_TOPICS:
            sCmd = _rosadapter->MakeCommand( nCmd );
            break;
        case Commands::RC_SUBSCRIBE:
        {
            // TODO: TYPE IS NOT KNOW YET. REFACTOR
            sCmd = _rosadapter->MakeCommand( nCmd, {_currTopic, "String"} );
            break;
        }
        case Commands::RC_UNSUBSCRIBE:
        {
            // TODO: TYPE IS NOT KNOW YET. REFACTOR
            sCmd = _rosadapter->MakeCommand( nCmd, {_currTopic} );
            break;
        }
        default:
            break;
    }
    _rosadapter->SendCommand( sCmd );
}

void UIAdapter::handleNewTopic( const QString& sTopic ){
    emit newTopic( sTopic );
}

QString UIAdapter::currentTopic() const{
    return _currTopic;
}

void UIAdapter::setCurrentTopic( const QString& sTopic ){
    if ( _currTopic == sTopic ){
        return;
    }
    _currTopic = sTopic;
    emit currentTopicChanged( _currTopic );
}
