

#include "uiadapter.h"
#include <boost/asio.hpp>
#include <QVariant>



UIAdapter::UIAdapter()
    :QObject()
{
}

void UIAdapter::GetTopics(){

}

QVariant UIAdapter::GetData(const QString& sTopic){
    QVariant bob(123);
    return bob;
}