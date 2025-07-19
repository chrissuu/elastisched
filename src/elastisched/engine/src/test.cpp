#include "tag.hpp"

Tag::Tag(std::string name) :
    name(name) {};

bool  Tag::operator==(const& Tag other) const {
    return this.name == other.name;
}