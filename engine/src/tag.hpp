#ifndef ELASTISCHED_TAG_HPP
#define ELASTISCHED_TAG_HPP

#include <string>

class Tag {
private:
    std::string name;
    std::string description;

public:
    Tag(const std::string& name, const std::string& description = "");
    
    const std::string& get_name() const;
    void set_name(const std::string& name);
    const std::string& get_description() const;
    void set_description(const std::string& description);

    bool operator==(const Tag& other) const;
    bool operator!=(const Tag& other) const;
    bool operator<(const Tag& other) const;
};

#endif // ELASTISCHED_TAG_HPP
