#ifndef TAG_HPP
#define TAG_HPP

#include <string>

class Tag {
private:
    std::string name;
    std::string description;

public:
    Tag(const std::string& name, const std::string& description = "");
    
    const std::string& getName() const;
    void setName(const std::string& name);
    const std::string& getDescription() const;
    void setDescription(const std::string& description);

    bool operator==(const Tag& other) const;
    bool operator!=(const Tag& other) const;
    bool operator<(const Tag& other) const;
};

#endif // TAG_HPP
