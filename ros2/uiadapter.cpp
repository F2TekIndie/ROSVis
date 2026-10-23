

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
    std::string sCmd = _rosadapter->MakeCommand( nCmd );
    _rosadapter->SendCommand( sCmd );
}

void UIAdapter::handleNewTopic( const QString& sTopic ){
    emit newTopic( sTopic );
}