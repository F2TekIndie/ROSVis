#ifndef SHADERS_H
#define SHADERS_H

#include <QString>
#include <map>



//
struct Shader{
    QString     name_{""};
    QString     path_{""};
    QByteArray  data_{nullptr};
    //
    uint size() const { return data_.size(); }
    char const* constData() const { return data_.constData(); }
};

//
class Shaders
{
public:
    //
    enum SHADER_TYPES {FRAG, VERTEX, TASK, MESH};

    //
    Shaders();
    ~Shaders() = default;

    //
    void init();
    //
    Shader shader( SHADER_TYPES type, int index ) const;
    int    shaderCount( SHADER_TYPES type ) const;

private:
    std::map< SHADER_TYPES, std::vector<Shader> > shaders_;
};

#endif // SHADERS_H
