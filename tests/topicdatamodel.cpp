#include "topicdatamodel.h"
#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QtTest>



//
namespace{
    QVariantMap sample( const QVariantMap& fields, qulonglong sequence = 1 ){
        return {
            { "type", "nav_msgs/msg/Odometry" },
            { "hasData", true },
            { "sequence", QVariant::fromValue( sequence ) },
            { "receivedUnixNanoseconds", QVariant::fromValue<qlonglong>( 1700000000123456789 ) },
            { "fields", fields }
        };
    }

    int fieldRow( const TopicDataModel& model, const QString& path ){
        for ( int row = 0; row < model.rowCount(); ++row ){
            if ( model.data( model.index( row ), TopicDataModel::FieldPathRole ).toString() == path ){
                return row;
            }
        }
        return -1;
    }
}

//
class TopicDataModelTest : public QObject{
    Q_OBJECT

private slots:
    void emptyAndCachedTopics(){
        TopicDataModel model;
        QAbstractItemModelTester tester( &model, QAbstractItemModelTester::FailureReportingMode::QtTest );
        QCOMPARE( model.rowCount(), 0 );
        QVERIFY( !model.index( 0 ).isValid() );
        QVERIFY( !model.hasData() );
        QVERIFY( !model.receivedTime().isValid() );
        model.onLatestReceived( "/odom", sample( { { "x", 1.5 } } ) );
        QCOMPARE( model.rowCount(), 0 );
        model.setCurrentTopic( "/odom" );
        QCOMPARE( model.rowCount(), 1 );
        QCOMPARE( model.messageType(), "nav_msgs/msg/Odometry" );
        QCOMPARE( model.sequence(), 1ULL );
        QCOMPARE( model.receivedTime().toMSecsSinceEpoch(), 1700000000123LL );
        QCOMPARE( model.rowCount( model.index( 0 ) ), 0 );
        QVERIFY( !model.data( {} ).isValid() );
        model.setCurrentTopic( "/other" );
        QCOMPARE( model.rowCount(), 0 );
        QVERIFY( !model.hasData() );
        model.onLatestReceived( "/other", { { "hasData", false }, { "conversionError", "No valid sample" } } );
        QCOMPARE( model.conversionError(), "No valid sample" );
        model.setCurrentTopic( "/odom" );
        QCOMPARE( model.rowCount(), 1 );
        QVERIFY( model.conversionError().isEmpty() );
    }

    void nestedFieldsAndValues(){
        TopicDataModel model;
        QAbstractItemModelTester tester( &model, QAbstractItemModelTester::FailureReportingMode::QtTest );
        model.setCurrentTopic( "/odom" );
        QVariantMap linear{ { "x", 0.5 }, { "y", 0.0 } };
        QVariantMap twist{ { "linear", linear } };
        auto input = sample( {
            { "twist", QVariantMap{ { "twist", twist } } },
            { "enabled", true },
            { "frame", "" },
            { "counter", "18446744073709551615" },
            { "empty", QVariantMap{} }
        } );
        model.onLatestReceived( "/odom", input );
        auto index = model.index( fieldRow( model, "twist.twist.linear.x" ) );
        QCOMPARE( model.data( index, TopicDataModel::DisplayValueRole ).toString(), "0.5" );
        QCOMPARE( model.data( index, TopicDataModel::ValueKindRole ).toString(), "number" );
        index = model.index( fieldRow( model, "enabled" ) );
        QCOMPARE( model.data( index ).toString(), "true" );
        index = model.index( fieldRow( model, "frame" ) );
        QCOMPARE( model.data( index ).toString(), "\"\"" );
        index = model.index( fieldRow( model, "counter" ) );
        QCOMPARE( model.data( index, TopicDataModel::RawValueRole ).toString(), "18446744073709551615" );
        QVERIFY( fieldRow( model, "empty" ) >= 0 );
        QCOMPARE( model.roleNames().value( TopicDataModel::FieldPathRole ), "fieldPath" );
    }

    void stableUpdates(){
        TopicDataModel model;
        QAbstractItemModelTester tester( &model, QAbstractItemModelTester::FailureReportingMode::QtTest );
        model.setCurrentTopic( "/odom" );
        QSignalSpy resets( &model, &QAbstractItemModel::modelReset );
        QSignalSpy changed( &model, &QAbstractItemModel::dataChanged );
        QSignalSpy metadata( &model, &TopicDataModel::sampleChanged );
        model.onLatestReceived( "/odom", sample( { { "x", 1 }, { "y", 2 } } ) );
        QCOMPARE( resets.count(), 1 );
        model.onLatestReceived( "/odom", sample( { { "x", 3 }, { "y", 2 } }, 2 ) );
        QCOMPARE( resets.count(), 1 );
        QCOMPARE( changed.count(), 1 );
        model.onLatestReceived( "/odom", sample( { { "x", 3 }, { "y", 2 } }, 3 ) );
        QCOMPARE( changed.count(), 1 );
        QCOMPARE( model.sequence(), 3ULL );
        const int notifications = metadata.count();
        model.onLatestReceived( "/other", sample( { { "x", 99 } } ) );
        QCOMPARE( metadata.count(), notifications );
        model.onLatestReceived( "/odom", sample( { { "z", 5 } } ) );
        QCOMPARE( resets.count(), 2 );
    }

    void arraysAndBinary(){
        TopicDataModel model;
        QAbstractItemModelTester tester( &model, QAbstractItemModelTester::FailureReportingMode::QtTest );
        model.setCurrentTopic( "/odom" );
        model.setArrayExpansionLimit( 12 );
        QVariantList values;
        for ( int i = 0; i < 12; ++i ){
            values.append( i );
        }
        auto input = sample( {
            { "values", values },
            { "data", "AP+A" },
            { "empty", QVariantList{} },
            { "raw", QByteArray( "\0\xff", 2 ) },
            { "children", QVariantList{ QVariantMap{ { "bytes", "AA==" } } } }
        } );
        input.insert( "binaryFields", QStringList{ "data", "children[].bytes" } );
        model.onLatestReceived( "/odom", input );
        const int first = fieldRow( model, "values[0]" );
        QCOMPARE( fieldRow( model, "values[10]" ), first + 10 );
        QCOMPARE( model.data( model.index( fieldRow( model, "data" ) ) ).toString(), "3 bytes" );
        QCOMPARE( model.data( model.index( fieldRow( model, "raw" ) ) ).toString(), "2 bytes" );
        QCOMPARE( model.data( model.index( fieldRow( model, "children[0].bytes" ) ) ).toString(), "1 bytes" );
        model.setArrayExpansionLimit( 2 );
        QCOMPARE( model.data( model.index( fieldRow( model, "values" ) ) ).toString(), "12 elements" );
        QCOMPARE( model.data( model.index( fieldRow( model, "values" ) ), TopicDataModel::RawValueRole ).toList().size(), 12 );
        model.setArrayExpansionLimit( -1 );
        QCOMPARE( model.arrayExpansionLimit(), 0 );
    }

    void boundedDisplay(){
        TopicDataModel model;
        model.setCurrentTopic( "/odom" );
        QVariantMap fields;
        for ( int i = 0; i < 5000; ++i ){
            fields.insert( QString::number( i ).rightJustified( 5, '0' ), i );
        }
        model.onLatestReceived( "/odom", sample( fields ) );
        QCOMPARE( model.rowCount(), 4096 );
        QCOMPARE( model.data( model.index( 4095 ), TopicDataModel::ValueKindRole ).toString(), "summary" );
        const QString longString( 10000, 'a' );
        model.onLatestReceived( "/odom", sample( { { "long", longString } } ) );
        QVERIFY( model.data( model.index( 0 ) ).toString().size() < 1000 );
        QCOMPARE( model.data( model.index( 0 ), TopicDataModel::RawValueRole ).toString(), longString );
    }
};

QTEST_GUILESS_MAIN( TopicDataModelTest )
#include "topicdatamodel.moc"
