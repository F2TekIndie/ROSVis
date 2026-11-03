#include "topicdatamodel.h"
#include <QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QDir>
#include <QtTest>



//
class PreviewAdapter : public QObject{
    Q_OBJECT
    Q_PROPERTY( QString currentTopic READ currentTopic NOTIFY currentTopicChanged )

public:
    QString currentTopic() const{
        return "/model/vehicle_blue/odometry";
    }
    Q_INVOKABLE void getLatest(){}

signals:
    void currentTopicChanged( const QString& topic );
    void latestReceived( const QString& topic, const QVariantMap& data );
    void errorOccurred( const QString& message );
};

//
namespace{
    bool hasText( const QQuickItem* item, const QString& text ){
        if ( item->property( "text" ).toString() == text ){
            return true;
        }
        for ( const auto* child : item->childItems() ){
            if ( hasText( child, text ) ){
                return true;
            }
        }
        return false;
    }
}

//
class DataViewTest : public QObject{
    Q_OBJECT

private slots:
    void displaySampleAndStates(){
        qmlRegisterType<PreviewAdapter>( "ROSVis", 1, 0, "UIAdapter" );
        qmlRegisterType<TopicDataModel>( "ROSVis", 1, 0, "TopicDataModel" );
        PreviewAdapter adapter;
        TopicDataModel model;
        QQuickView view;
        view.setResizeMode( QQuickView::SizeRootObjectToView );
        view.resize( 1000, 450 );
        view.setInitialProperties( {
            { "adapter", QVariant::fromValue( &adapter ) },
            { "dataModel", QVariant::fromValue( &model ) }
        } );
        view.setSource( QUrl::fromLocalFile( DATA_VIEW_SOURCE ) );
        QCOMPARE( view.status(), QQuickView::Ready );
        QVERIFY( view.rootObject() );
        view.show();
        QTRY_VERIFY( hasText( view.rootObject(), "No topic selected" ) );
        model.setCurrentTopic( adapter.currentTopic() );
        QTRY_VERIFY( hasText( view.rootObject(), "Waiting for the first message. Subscribe to the topic, then refresh." ) );
        model.onLatestReceived( adapter.currentTopic(), {
            { "type", "nav_msgs/msg/Odometry" },
            { "hasData", true },
            { "sequence", 42 },
            { "receivedUnixNanoseconds", QVariant::fromValue<qlonglong>( 1700000000123456789 ) },
            { "fields", QVariantMap{
                { "header", QVariantMap{ { "frame_id", "odom" } } },
                { "twist", QVariantMap{ { "twist", QVariantMap{ { "linear", QVariantMap{ { "x", 0.5 }, { "y", 0.0 } } } } } } }
            } }
        } );
        QTRY_VERIFY( hasText( view.rootObject(), "twist.twist.linear.x" ) );
        QTRY_VERIFY( hasText( view.rootObject(), "0.5" ) );
        QTRY_VERIFY( hasText( view.rootObject(), "nav_msgs/msg/Odometry" ) );
        QTest::qWait( 50 );
        const auto image = view.grabWindow();
        QVERIFY( !image.isNull() );
        QVERIFY( image.save( QDir::tempPath() + "/rosvis-topic-data-preview.png" ) );
        emit adapter.errorOccurred( "Topic is not subscribed" );
        QTRY_VERIFY( hasText( view.rootObject(), "Topic is not subscribed" ) );
    }
};

//
int main( int argc, char** argv ){
    QQuickWindow::setGraphicsApi( QSGRendererInterface::Software );
    QGuiApplication app( argc, argv );
    DataViewTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "data_view.moc"
