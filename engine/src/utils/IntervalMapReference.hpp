#ifndef INTERVALTREE_HPP
#define INTERVALTREE_HPP
#include <memory>
#include <iostream>
#include <utility>
#include <vector>
#include "Interval.hpp"

enum Color { RED, BLACK };

template<typename T, typename U>
struct IntervalMapNode {
    std::unique_ptr<IntervalMapNode<T, U>> left;
    std::unique_ptr<IntervalMapNode<T, U>> right;
    std::shared_ptr<IntervalMapNode<T, U>> parent;
    Interval<T> interval;
    U value;
    T max;
    Color color;
    
    IntervalMapNode(std::unique_ptr<Interval<T>> i, U v);
};


template<typename T, typename U>
class IntervalMap {
private:
    std::unique_ptr<IntervalMapNode<T, U>> root;
    void leftRotate()

public:
    IntervalMap() = default;
    IntervalMap(const IntervalMap<T, U>& other);
    IntervalMap<T, U>& operator=(const IntervalMap<T, U>& other);
    
    /**
     * insert
     * 
     * @param key: the interval that you'd like to insert into the tree
     * @param value: the value that the created node should carry
     */
    void insert(Interval<T> key, U value);

    /**
     * search
     * 
     * @param key: an interval that you'd like to search for
     * 
     * @return: overlapping intervals with key's values
     */
    std::vector<U*> find(const Interval<T>& key) const;

    /**
     * search
     * 
     * @param 
     */
    std::vector<std::pair<T*, U*>> find_kv(const Interval<T>& key) const;

    /**
     * query
     * 
     * @param interval: an interval that you'd like to query
     * 
     * @return: True if interval overlaps with an interval in the tree,
     *          False otherwise
     */
    bool query(const Interval<T>& key) const;
    
    void print() const;
};

#endif