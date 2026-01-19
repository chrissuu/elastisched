#ifndef INTERVALMAP_HPP
#define INTERVALMAP_HPP

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include "Interval.hpp"

enum Color { red, black };

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
        : interval(i), value(v), max(i.get_high()), color(red),
          left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename T, typename U>
class IntervalMap {
private:
    Node<T, U>* root;

    void left_rotate(Node<T, U>* x) {
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

        update_max(x);
        update_max(y);
    }

    void right_rotate(Node<T, U>* y) {
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

        update_max(y);
        update_max(x);
    }

    void update_max(Node<T, U>* node) {
        if (!node) return;
        
        node->max = node->interval.get_high();
        if (node->left && node->left->max > node->max) {
            node->max = node->left->max;
        }
        if (node->right && node->right->max > node->max) {
            node->max = node->right->max;
        }
    }

    void insert_fixup(Node<T, U>* z) {
        while (z->parent && z->parent->color == red) {
            if (z->parent == z->parent->parent->left) {
                Node<T, U>* y = z->parent->parent->right;
                if (y && y->color == red) {
                    z->parent->color = black;
                    y->color = black;
                    z->parent->parent->color = red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = black;
                    z->parent->parent->color = red;
                    right_rotate(z->parent->parent);
                }
            } else {
                Node<T, U>* y = z->parent->parent->left;
                if (y && y->color == red) {
                    z->parent->color = black;
                    y->color = black;
                    z->parent->parent->color = red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = black;
                    z->parent->parent->color = red;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->color = black;
    }

    void update_max_to_root(Node<T, U>* node) {
        while (node) {
            update_max(node);
            node = node->parent;
        }
    }

    void insert_node(Node<T, U>* new_node) {
        Node<T, U>* y = nullptr;
        Node<T, U>* x = root;

        while (x) {
            y = x;
            if (new_node->interval.get_low() < x->interval.get_low()) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        new_node->parent = y;

        if (!y) {
            root = new_node;
            root->color = black;
        } else if (new_node->interval.get_low() < y->interval.get_low()) {
            y->left = new_node;
            update_max_to_root(y);
            insert_fixup(new_node);
        } else {
            y->right = new_node;
            update_max_to_root(y);
            insert_fixup(new_node);
        }
    }

    void find_overlapping(const Node<T, U>* node, const Interval<T>& key, std::vector<const U*>& result) const {
        if (!node) return;

        if (node->interval.overlaps(key)) {
            result.push_back(&node->value);
        }

        if (node->left && node->left->max >= key.get_low()) {
            find_overlapping(node->left, key, result);
        }
        if (node->right && node->interval.get_low() <= key.get_high()) {
            find_overlapping(node->right, key, result);
        }
    }

    void find_overlapping_kv(const Node<T, U>* node, const Interval<T>& key, std::vector<std::pair<const Interval<T>*, const U*>>& result) const {
        if (!node) return;

        if (node->interval.overlaps(key)) {
            result.emplace_back(&node->interval, &node->value);
        }

        if (node->left && node->left->max >= key.get_low()) {
            find_overlapping_kv(node->left, key, result);
        }
        if (node->right && node->interval.get_low() <= key.get_high()) {
            find_overlapping_kv(node->right, key, result);
        }
    }

    bool query_overlap(const Node<T, U>* node, const Interval<T>& key) const {
        if (!node) return false;
        if (node->interval.overlaps(key)) return true;
        if (node->left && node->left->max >= key.get_low()) {
            if (query_overlap(node->left, key)) return true;
        }
        if (node->right && node->interval.get_low() <= key.get_high()) {
            if (query_overlap(node->right, key)) return true;
        }
        return false;
    }

    void print_node(const Node<T, U>* node, int depth = 0) const {
        if (!node) return;
        print_node(node->left, depth + 1);
        std::cout << std::string(depth * 4, ' ') << node->interval << " | val: " << node->value << " | max: " << node->max
                  << " | color: " << (node->color == red ? "red" : "black") << std::endl;
        print_node(node->right, depth + 1);
    }

    void destroy_tree(Node<T, U>* node) {
        if (!node) return;
        destroy_tree(node->left);
        destroy_tree(node->right);
        delete node;
    }

public:
    IntervalMap() : root(nullptr) {}

    ~IntervalMap() {
        destroy_tree(root);
    }

    IntervalMap(const IntervalMap& other) : root(nullptr) {
        if (other.root) {
            root = copy_tree(other.root, nullptr);
        }
    }

    IntervalMap& operator=(const IntervalMap& other) {
        if (this != &other) {
            destroy_tree(root);
            root = nullptr;
            if (other.root) {
                root = copy_tree(other.root, nullptr);
            }
        }
        return *this;
    }

private:
    Node<T, U>* copy_tree(const IntervalMapNode<T, U>* node, IntervalMapNode<T, U>* parent) {
        if (!node) return nullptr;
        
        Node<T, U>* new_node = new IntervalMapNode<T, U>(node->interval, node->value);
        new_node->parent = parent;
        new_node->max = node->max;
        new_node->color = node->color;
        
        new_node->left = copy_tree(node->left, new_node);
        new_node->right = copy_tree(node->right, new_node);
        
        return new_node;
    }

public:
    void insert(const Interval<T>& key, const U& value) {
        Node<T, U>* new_node = new IntervalMapNode<T, U>(key, value);
        insert_node(new_node);
    }

    std::vector<const U*> find(const Interval<T>& key) const {
        std::vector<const U*> result;
        find_overlapping(root, key, result);
        return result;
    }

    std::vector<std::pair<const Interval<T>*, const U*>> find_kv(const Interval<T>& key) const {
        std::vector<std::pair<const Interval<T>*, const U*>> result;
        find_overlapping_kv(root, key, result);
        return result;
    }

    bool query(const Interval<T>& key) const {
        return query_overlap(root, key);
    }

    void print() const {
        if (!root) {
            std::cout << "Empty tree" << std::endl;
            return;
        }
        print_node(root);
    }

    bool empty() const {
        return root == nullptr;
    }
};

#endif
