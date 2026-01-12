#include "tag.hpp"

Tag::Tag(const std::string& name, const std::string& description)
    : name(name),
      description(description) {
}

const std::string& Tag::getName() const {
    return this->name;
}

void Tag::setName(const std::string& name) {
    this->name = name;
}

const std::string& Tag::getDescription() const {
    return this->description;
}

void Tag::setDescription(const std::string& description) {
    this->description = description;
}

bool Tag::operator==(const Tag& other) const {
    return name == other.name;
}

bool Tag::operator!=(const Tag& other) const {
    return !(*this == other);
}

bool Tag::operator<(const Tag& other) const {
    return name < other.name;
}
