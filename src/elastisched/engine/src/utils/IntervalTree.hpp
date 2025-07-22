#ifndef INTERVALTREE_HPP
#define INTERVALTREE_HPP
#include <memory>
#include <iostream>
#include <utility>
#include "Interval.hpp"

template<typename T, typename U>
struct Node {
    std::unique_ptr<Interval<T>> interval;
    U value;
    T max;
    std::unique_ptr<Node<T, U>> left;
    std::unique_ptr<Node<T, U>> right;
    
    Node(std::unique_ptr<Interval<T>> i, U v)
        : interval(std::move(i)), value(v), max(interval->getHigh()), left(nullptr), right(nullptr) {}
};

template<typename T, typename U>
class IntervalTree {
private:
    std::unique_ptr<Node<T, U>> root;
    
    std::unique_ptr<Node<T, U>> insert(std::unique_ptr<Node<T, U>> node,
                                       std::unique_ptr<Interval<T>> i,
                                       U value) {
        if (!node) {
            return std::make_unique<Node<T, U>>(std::move(i), value);
        }
        
        // CRITICAL FIX: Store values before moving the interval
        T intervalLow = i->getLow();
        T intervalHigh = i->getHigh();
        
        if (intervalLow < node->interval->getLow()) {
            node->left = insert(std::move(node->left), std::move(i), value);
        } else {
            node->right = insert(std::move(node->right), std::move(i), value);
        }
        
        // CRITICAL FIX: Use stored value instead of moved interval
        node->max = std::max(node->max, intervalHigh);
        return node;
    }
    
    bool doOverlap(Interval<T>* i1, Interval<T>* i2) const {
        return (i1->getLow() <= i2->getHigh() && i2->getLow() <= i1->getHigh());
    }
    
    Node<T, U>* overlapSearch(Node<T, U>* node, Interval<T>& i) const {
        if (!node)
            return nullptr;
            
        if (doOverlap(node->interval.get(), &i))
            return node;
            
        if (node->left && node->left->max >= i.getLow())
            return overlapSearch(node->left.get(), i);
            
        return overlapSearch(node->right.get(), i);
    }
    
    std::unique_ptr<Node<T, U>> cloneNode(const std::unique_ptr<Node<T, U>>& node) const {
        if (!node)
            return nullptr;
            
        auto newInterval = std::make_unique<Interval<T>>(*node->interval);
        auto newNode = std::make_unique<Node<T, U>>(std::move(newInterval), node->value);
        newNode->max = node->max;
        newNode->left = cloneNode(node->left);
        newNode->right = cloneNode(node->right);
        return newNode;
    }
    
    void printInOrder(const Node<T, U>* node) const {
        if (!node) return;
        printInOrder(node->left.get());
        std::cout << "[" << node->interval->getLow() << ", " << node->interval->getHigh() 
                  << "] max=" << node->max << std::endl;
        printInOrder(node->right.get());
    }

public:
    IntervalTree() = default;
    
    IntervalTree(const IntervalTree<T, U>& other) {
        root = cloneNode(other.root);
    }
    
    IntervalTree<T, U>& operator=(const IntervalTree<T, U>& other) {
        if (this != &other) {
            root = cloneNode(other.root);
        }
        return *this;
    }
    
    void insert(T low, T high, U value) {
        auto interval = std::make_unique<Interval<T>>(low, high);
        root = insert(std::move(root), std::move(interval), value);
    }
    
    void insert(Interval<T> interval, U value) {
        auto i = std::make_unique<Interval<T>>(interval);
        root = insert(std::move(root), std::move(i), value);
    }

    Interval<T>* searchOverlap(Interval<T> query) const {
        auto result = overlapSearch(root.get(), query);
        return result ? result->interval.get() : nullptr;
    }
    
    Interval<T>* searchOverlap(T low, T high) const {
        return searchOverlap(Interval<T>(low, high));
    }
    
    U* searchValue(T low, T high) const {
        Interval<T> query(low, high);
        auto result = overlapSearch(root.get(), query);
        return result ? &result->value : nullptr;
    }
    
    U* searchValue(Interval<T> interval) const {
        return searchValue(interval.getLow(), interval.getHigh());
    }
    
    bool isIn(const Interval<T>& interval) {
        return searchOverlap(interval.getLow(), interval.getHigh()) != nullptr;
    }
    
    void print() const {
        printInOrder(root.get());
    }
};

#endif