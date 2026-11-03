
#ifndef TOPICDATAMODEL_H
#define TOPICDATAMODEL_H

#include <QAbstractItemModel>
#include <QQuickItem>




//
class TopicDataModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TopicDataModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void onLatestReceived( const QString& topic, const QVariantMap& data );

private:
    QMap<QString, QVariantMap> _data{};
};

#endif // TOPICDATAMODEL_H
