

#include "uiadapter.h"
#include <boost/asio.hpp>
#include <QVariant>
#include "rosadapter.h"
#include "data.h"



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
    auto xxx = _rosadapter->GetData( nCmd );
    // Hack me
    auto pData = dynamic_cast<DataTopics*>( xxx.get() );
    for ( auto& entry : pData->_topics ){
        emit newTopic( QString::fromStdString( entry ) );
    }
}