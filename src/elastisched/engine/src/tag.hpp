#ifndef TAG_HPP
#define TAG_HPP

#include <string>

class Tag {
private:
    std::string name;

public:
    Tag(const std::string& name);
    
    const std::string& getName() const;
    void setName(const std::string& name);

    bool operator==(const Tag& other) const;
    bool operator!=(const Tag& other) const;
    bool operator<(const Tag& other) const;
};

#endif // TAG_HPP