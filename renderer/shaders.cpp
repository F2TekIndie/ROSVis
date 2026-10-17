
#include "shaders.h"
#include <QFile>
#include <QDir>



Shaders::Shaders()
{
    shaders_[ VERTEX ] = {};
    shaders_[ FRAG ]   = {};
    shaders_[ TASK ]   = {};
    shaders_[ MESH ]   = {};
}

void Shaders::init(){
    QString sBasePath = QDir::currentPath();
    // Load Vert
    QString sVert("/shader/simple.vert.spv");
    QFile fVert( sBasePath + sVert);
    if ( !fVert.open(QIODevice::ReadOnly) )
        qFatal("Failed to read shader %s", qPrintable(sVert));
    shaders_[ VERTEX ].push_back( {sVert, sBasePath, fVert.readAll() } );
    fVert.close();

    // Load frag
    QString sFrag("/shader/simple.frag.spv");
    QFile fFrag( sBasePath + sFrag );
    if (!fFrag.open(QIODevice::ReadOnly))
        qFatal("Failed to read shader %s", qPrintable(sFrag));
    shaders_[ FRAG ].push_back( {sFrag, sBasePath, fFrag.readAll() } );
    fFrag.close();

    // Load Task
    // Load Mesh
}

Shader Shaders::shader( SHADER_TYPES type, int index ) const{
    //
    for ( const auto& [key, value] : shaders_){
        if ( key == type ){
            if ( index < value.size() ){
                return value[index];
            }
        }
    }
    //
    return {};
}

int Shaders::shaderCount( SHADER_TYPES type ) const{
    //
    for ( const auto& [key, value] : shaders_){
        if ( key == type ){
            return value.size();
        }
    }
    //
    return 0;
}
