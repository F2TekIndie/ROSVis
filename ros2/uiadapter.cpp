

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
    connect( _rosadapter.get(), &rosadapter::newDataAvailable, this, &UIAdapter::getData );
}

void UIAdapter::sendCommand( UIAdapter::ROS_CMD nCmd ){
    std::string sCmd = _rosadapter->MakeCommand( static_cast<int>(nCmd) );
    _rosadapter->SendCommand( sCmd );
}

void UIAdapter::getData( int nCmd ){
    //
    switch( nCmd ){
        case RC_INVALID:
            //auto xxx = _rosadapter->GetData( nCmd );
            break;
        case RC_GET_TOPICS:
            //auto* pData = _rosadapter->GetData( nCmd );
            //for ( auto& entry : pData->_topics )
            //    emit newTopic( QString::fromStdString( entry ) );
            break;
        case RC_SUBSCRIBE:
            //auto xxx = _rosadapter->GetData( nCmd );
            break;
        case RC_UNSUBSCRIBE:
            //auto xxx = _rosadapter->GetData( nCmd );
            break;
    }
}