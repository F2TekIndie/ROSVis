

#include "uiadapter.h"
#include <boost/asio.hpp>
#include <QVariant>
#include "rosadapter.h"



UIAdapter::UIAdapter()
    :QObject()
{
    _rosadapter.reset( new rosadapter() );
}

void UIAdapter::getTopics(){
    std::string sCmd = _rosadapter->MakeCommand();
    _rosadapter->SendCommand( sCmd );
}

QVariant UIAdapter::getData(const QString& sTopic){
    QVariant bob(123);
    return bob;
}