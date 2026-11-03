#include "topicdatamodel.h"
#include <QTimeZone>



//
namespace{
    constexpr int MAX_FIELD_ROWS = 4096;
    constexpr int MAX_FIELD_DEPTH = 64;
    constexpr int MAX_DISPLAY_CHARACTERS = 512;

    QString shortened( const QString& value ){
        if ( value.size() > MAX_DISPLAY_CHARACTERS ){
            return value.left( MAX_DISPLAY_CHARACTERS ) + QStringLiteral( "…" );
        }
        return value;
    }
}

//
TopicDataModel::TopicDataModel( QObject* parent )
    :QAbstractListModel( parent )
{}

//
int TopicDataModel::rowCount( const QModelIndex& parent ) const{
    return parent.isValid() ? 0 : _fields.size();
}

//
QVariant TopicDataModel::data( const QModelIndex& index, int role ) const{
    if ( !index.isValid() || index.model() != this || index.column() != 0 || index.row() < 0 || index.row() >= _fields.size() ){
        return {};
    }
    const auto& field = _fields[index.row()];
    switch ( role ){
        case Qt::DisplayRole:
        case DisplayValueRole:
            return field.display;
        case FieldPathRole:
            return field.path;
        case RawValueRole:
            return field.value;
        case ValueKindRole:
            return field.kind;
        default:
            return {};
    }
}

//
QHash<int, QByteArray> TopicDataModel::roleNames() const{
    return {
        { FieldPathRole, "fieldPath" },
        { DisplayValueRole, "displayValue" },
        { RawValueRole, "rawValue" },
        { ValueKindRole, "valueKind" }
    };
}

//
QString TopicDataModel::currentTopic() const{
    return _currentTopic;
}

//
QString TopicDataModel::messageType() const{
    return _sample.value( "type" ).toString();
}

//
bool TopicDataModel::hasData() const{
    return _sample.value( "hasData" ).toBool();
}

//
qulonglong TopicDataModel::sequence() const{
    return _sample.value( "sequence" ).toULongLong();
}

//
QDateTime TopicDataModel::receivedTime() const{
    const auto nanoseconds = _sample.value( "receivedUnixNanoseconds" ).toLongLong();
    if ( !hasData() ){
        return {};
    }
    return QDateTime::fromMSecsSinceEpoch( nanoseconds / 1000000, QTimeZone::UTC );
}

//
QString TopicDataModel::conversionError() const{
    return _sample.value( "conversionError" ).toString();
}

//
void TopicDataModel::setCurrentTopic( const QString& topic ){
    if ( _currentTopic == topic ){
        return;
    }
    _currentTopic = topic;
    _sample = _data.value( topic );
    rebuildFields();
    emit currentTopicChanged();
    emit sampleChanged();
}

//
void TopicDataModel::onLatestReceived( const QString& topic, const QVariantMap& data ){
    _data[topic] = data;
    if ( topic != _currentTopic ){
        return;
    }
    _sample = data;
    rebuildFields();
    emit sampleChanged();
}

//
void TopicDataModel::appendFields( QList<Field>& fields, const QString& path, const QString& schemaPath,
    const QVariant& value, const QSet<QString>& binaryFields, int depth, bool& truncated ) const{
    // Reserve the last row for an explicit truncation notice.
    if ( fields.size() >= MAX_FIELD_ROWS - 1 ){
        truncated = true;
        return;
    }
    if ( depth > MAX_FIELD_DEPTH ){
        fields.append( { path, tr( "Nesting limit reached" ), {}, "summary" } );
        return;
    }
    if ( value.metaType().id() == QMetaType::QVariantMap ){
        const auto object = value.toMap();
        if ( object.isEmpty() && !path.isEmpty() ){
            fields.append( { path, "{}", value, "object" } );
        }
        for ( auto it = object.cbegin(); it != object.cend() && !truncated; ++it ){
            appendFields( fields, path.isEmpty() ? it.key() : path + "." + it.key(),
                schemaPath.isEmpty() ? it.key() : schemaPath + "." + it.key(), it.value(), binaryFields, depth + 1, truncated );
        }
        return;
    }
    if ( value.metaType().id() == QMetaType::QVariantList || value.metaType().id() == QMetaType::QStringList ){
        const auto array = value.toList();
        if ( array.isEmpty() ){
            fields.append( { path, tr( "%1 elements" ).arg( array.size() ), value, "array" } );
        }
        else{
            for ( int i = 0; i < array.size() && !truncated; ++i ){
                appendFields( fields, path + "[" + QString::number( i ) + "]", schemaPath + "[]", array[i], binaryFields, depth + 1, truncated );
            }
        }
        return;
    }
    if ( binaryFields.contains( schemaPath ) || value.metaType().id() == QMetaType::QByteArray ){
        qsizetype size = 0;
        if ( value.metaType().id() == QMetaType::QByteArray ){
            size = value.toByteArray().size();
        }
        if ( value.metaType().id() != QMetaType::QByteArray ){
            // Protobuf JSON uses padded base64. Count bytes without allocating a decoded copy.
            const auto encoded = value.toString();
            size = ( encoded.size() / 4 ) * 3;
            if ( encoded.endsWith( "==" ) ){
                size -= 2;
            }
            else if ( encoded.endsWith( "=" ) ){
                --size;
            }
        }
        fields.append( { path, tr( "%1 bytes" ).arg( size ), value, "binary" } );
        return;
    }
    QString kind = "number";
    QString display;
    switch ( value.metaType().id() ){
        case QMetaType::Bool:
            kind = "boolean";
            display = value.toBool() ? "true" : "false";
            break;
        case QMetaType::QString:
            kind = "string";
            display = value.toString().isEmpty() ? "\"\"" : shortened( value.toString() );
            break;
        case QMetaType::UnknownType:
        case QMetaType::Nullptr:
            kind = "null";
            display = "null";
            break;
        default:
            display = shortened( value.toString() );
            break;
    }
    fields.append( { path, display, value, kind } );
}

//
void TopicDataModel::rebuildFields(){
    QList<Field> fields;
    if ( hasData() ){
        const auto binaryPaths = _sample.value( "binaryFields" ).toStringList();
        const QSet<QString> binaryFields( binaryPaths.begin(), binaryPaths.end() );
        bool truncated = false;
        appendFields( fields, {}, {}, _sample.value( "fields", QVariantMap{} ), binaryFields, 0, truncated );
        if ( truncated ){
            fields.append( { {}, tr( "Field limit reached; remaining fields are hidden" ), {}, "summary" } );
        }
    }
    bool sameStructure = fields.size() == _fields.size();
    for ( int i = 0; sameStructure && i < fields.size(); ++i ){
        sameStructure = fields[i].path == _fields[i].path && fields[i].kind == _fields[i].kind;
    }
    if ( !sameStructure ){
        beginResetModel();
        _fields = std::move( fields );
        endResetModel();
        return;
    }
    for ( int i = 0; i < fields.size(); ++i ){
        if ( fields[i].display != _fields[i].display || fields[i].value != _fields[i].value ){
            _fields[i] = std::move( fields[i] );
            emit dataChanged( index( i ), index( i ), { Qt::DisplayRole, DisplayValueRole, RawValueRole } );
        }
    }
}
