#include "tag.hpp"

Tag::Tag(const std::string& name) : name(name) {
}

const std::string& Tag::getName() const {
    return this->name;
}

void Tag::setName(const std::string& name) {
    this->name = name;
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