
#include "topicdatamodel.h"




TopicDataModel::TopicDataModel(QObject *parent)
    : QAbstractItemModel(parent)
{}

QVariant TopicDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    return QVariant( "Type" );
}

QModelIndex TopicDataModel::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
    return QModelIndex();
}

QModelIndex TopicDataModel::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
    return QModelIndex();
}

int TopicDataModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
    return _data.count();
}

int TopicDataModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 1;
}

QVariant TopicDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}

void TopicDataModel::onLatestReceived( const QString& topic, const QVariantMap& data ){
    _data[ topic ] = data;
}