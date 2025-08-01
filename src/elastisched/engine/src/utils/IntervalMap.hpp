#ifndef INTERVALMAP_HPP
#define INTERVALMAP_HPP

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include "Interval.hpp"

enum Color { RED, BLACK };

template<typename T, typename U>
struct Node {
    Node<T, U>* left;
    Node<T, U>* right;
    Node<T, U>* parent;
    Interval<T> interval;
    U value;
    T max;
    Color color;

    Node(const Interval<T>& i, U v)
        : interval(i), value(v), max(i.getHigh()), color(RED),
          left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename T, typename U>
class IntervalMap {
private:
    Node<T, U>* root;

    void leftRotate(Node<T, U>* x) {
        Node<T, U>* y = x->right;
        
        x->right = y->left;
        if (y->left != nullptr) {
            y->left->parent = x;
        }

        y->parent = x->parent;
        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;

        updateMax(x);
        updateMax(y);
    }

    void rightRotate(Node<T, U>* y) {
        Node<T, U>* x = y->left;
        
        y->left = x->right;
        if (x->right != nullptr) {
            x->right->parent = y;
        }

        x->parent = y->parent;
        if (y->parent == nullptr) {
            root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }

        x->right = y;
        y->parent = x;

        updateMax(y);
        updateMax(x);
    }

    void updateMax(Node<T, U>* node) {
        if (!node) return;
        
        node->max = node->interval.getHigh();
        if (node->left && node->left->max > node->max) {
            node->max = node->left->max;
        }
        if (node->right && node->right->max > node->max) {
            node->max = node->right->max;
        }
    }

    void insertFixup(Node<T, U>* z) {
        while (z->parent && z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node<T, U>* y = z->parent->parent->right;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                Node<T, U>* y = z->parent->parent->left;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void updateMaxToRoot(Node<T, U>* node) {
        while (node) {
            updateMax(node);
            node = node->parent;
        }
    }

    void insertNode(Node<T, U>* newNode) {
        Node<T, U>* y = nullptr;
        Node<T, U>* x = root;

        while (x) {
            y = x;
            if (newNode->interval.getLow() < x->interval.getLow()) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        newNode->parent = y;

        if (!y) {
            root = newNode;
            root->color = BLACK;
        } else if (newNode->interval.getLow() < y->interval.getLow()) {
            y->left = newNode;
            updateMaxToRoot(y);
            insertFixup(newNode);
        } else {
            y->right = newNode;
            updateMaxToRoot(y);
            insertFixup(newNode);
        }
    }

    void findOverlapping(const Node<T, U>* node, const Interval<T>& key, std::vector<const U*>& result) const {
        if (!node) return;

        if (node->interval.overlaps(key)) {
            result.push_back(&node->value);
        }

        if (node->left && node->left->max >= key.getLow()) {
            findOverlapping(node->left, key, result);
        }
        if (node->right && node->interval.getLow() <= key.getHigh()) {
            findOverlapping(node->right, key, result);
        }
    }

    void findOverlappingKV(const Node<T, U>* node, const Interval<T>& key, std::vector<std::pair<const Interval<T>*, const U*>>& result) const {
        if (!node) return;

        if (node->interval.overlaps(key)) {
            result.emplace_back(&node->interval, &node->value);
        }

        if (node->left && node->left->max >= key.getLow()) {
            findOverlappingKV(node->left, key, result);
        }
        if (node->right && node->interval.getLow() <= key.getHigh()) {
            findOverlappingKV(node->right, key, result);
        }
    }

    bool queryOverlap(const Node<T, U>* node, const Interval<T>& key) const {
        if (!node) return false;
        if (node->interval.overlaps(key)) return true;
        if (node->left && node->left->max >= key.getLow()) {
            if (queryOverlap(node->left, key)) return true;
        }
        if (node->right && node->interval.getLow() <= key.getHigh()) {
            if (queryOverlap(node->right, key)) return true;
        }
        return false;
    }

    void printNode(const Node<T, U>* node, int depth = 0) const {
        if (!node) return;
        printNode(node->left, depth + 1);
        std::cout << std::string(depth * 4, ' ') << node->interval << " | val: " << node->value << " | max: " << node->max
                  << " | color: " << (node->color == RED ? "RED" : "BLACK") << std::endl;
        printNode(node->right, depth + 1);
    }

    void destroyTree(Node<T, U>* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    IntervalMap() : root(nullptr) {}

    ~IntervalMap() {
        destroyTree(root);
    }

    IntervalMap(const IntervalMap& other) : root(nullptr) {
        if (other.root) {
            root = copyTree(other.root, nullptr);
        }
    }

    IntervalMap& operator=(const IntervalMap& other) {
        if (this != &other) {
            destroyTree(root);
            root = nullptr;
            if (other.root) {
                root = copyTree(other.root, nullptr);
            }
        }
        return *this;
    }

private:
    Node<T, U>* copyTree(const IntervalMapNode<T, U>* node, IntervalMapNode<T, U>* parent) {
        if (!node) return nullptr;
        
        Node<T, U>* newNode = new IntervalMapNode<T, U>(node->interval, node->value);
        newNode->parent = parent;
        newNode->max = node->max;
        newNode->color = node->color;
        
        newNode->left = copyTree(node->left, newNode);
        newNode->right = copyTree(node->right, newNode);
        
        return newNode;
    }

public:
    void insert(const Interval<T>& key, const U& value) {
        Node<T, U>* newNode = new IntervalMapNode<T, U>(key, value);
        insertNode(newNode);
    }

    std::vector<const U*> find(const Interval<T>& key) const {
        std::vector<const U*> result;
        findOverlapping(root, key, result);
        return result;
    }

    std::vector<std::pair<const Interval<T>*, const U*>> find_kv(const Interval<T>& key) const {
        std::vector<std::pair<const Interval<T>*, const U*>> result;
        findOverlappingKV(root, key, result);
        return result;
    }

    bool query(const Interval<T>& key) const {
        return queryOverlap(root, key);
    }

    void print() const {
        if (!root) {
            std::cout << "Empty tree" << std::endl;
            return;
        }
        printNode(root);
    }

    bool empty() const {
        return root == nullptr;
    }
};

#endif