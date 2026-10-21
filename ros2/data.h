#ifndef DATA_H
#define DATA_H

#include <string>
#include <vector>




// NEEDS MORE RETHINKING => TEST SAMPLE
// THE WHOLE BUFFER TRANSFER needs to change from string parsing to a more practical approach
//
struct Data{
    virtual void Dummy() {}
};

struct DataTopics : public Data {
    std::vector<std::string>  _topics = {};
    std::vector<std::string>  _types = {};
};

struct DataString : public Data {
    std::vector<std::string> _data = {};
};

struct DataInt : public Data {
    std::vector<int> _data = {};
};

#endif // DATA_H
