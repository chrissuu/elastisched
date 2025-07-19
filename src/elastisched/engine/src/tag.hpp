#ifndef TAG_HPP
#define TAG_HPP

#include <string>

class Tag {
private:
    std::string name;

public:
    // Constructors
    Tag(const std::string& name);
    
    // Getter and setter
    const std::string& getName() const;
    void setName(const std::string& name);

    // Comparison operators
    bool operator==(const Tag& other) const;
    bool operator!=(const Tag& other) const;
    bool operator<(const Tag& other) const;
};

#endif // TAG_HPP