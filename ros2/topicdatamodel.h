#ifndef TOPICDATAMODEL_H
#define TOPICDATAMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>



//
class TopicDataModel : public QAbstractListModel{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY( QString currentTopic READ currentTopic WRITE setCurrentTopic NOTIFY currentTopicChanged )
    Q_PROPERTY( QString messageType READ messageType NOTIFY sampleChanged )
    Q_PROPERTY( bool hasData READ hasData NOTIFY sampleChanged )
    Q_PROPERTY( qulonglong sequence READ sequence NOTIFY sampleChanged )
    Q_PROPERTY( QDateTime receivedTime READ receivedTime NOTIFY sampleChanged )
    Q_PROPERTY( QString conversionError READ conversionError NOTIFY sampleChanged )

public:
    enum Role{
        FieldPathRole = Qt::UserRole + 1,
        DisplayValueRole,
        RawValueRole,
        ValueKindRole
    };
    Q_ENUM( Role )

    explicit TopicDataModel( QObject* parent = nullptr );
    int rowCount( const QModelIndex& parent = {} ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentTopic() const;
    QString messageType() const;
    bool hasData() const;
    qulonglong sequence() const;
    QDateTime receivedTime() const;
    QString conversionError() const;

public slots:
    void setCurrentTopic( const QString& topic );
    void onLatestReceived( const QString& topic, const QVariantMap& data );

signals:
    void currentTopicChanged();
    void sampleChanged();

private:
    struct Field{
        QString path;
        QString display;
        QVariant value;
        QString kind;
    };

    void rebuildFields();
    void appendFields( QList<Field>& fields, const QString& path, const QString& schemaPath,
        const QVariant& value, const QSet<QString>& binaryFields, int depth, bool& truncated ) const;

    QMap<QString, QVariantMap> _data{};
    QList<Field> _fields{};
    QVariantMap _sample{};
    QString _currentTopic{""};
};

#endif // TOPICDATAMODEL_H
