template<typename T>
class IntervalTree {
private:
    std::unique_ptr<Node<T>> root;

    std::unique_ptr<Node<T>> insert(std::unique_ptr<Node<T>> node, std::unique_ptr<Interval<T>> i) {
        if (!node)
            return std::make_unique<Node<T>>(std::move(i));

        if (i->low < node->interval->low)
            node->left = insert(std::move(node->left), std::move(i));
        else
            node->right = insert(std::move(node->right), std::move(i));

        node->max = std::max(node->max, i->high);
        return node;
    }

    bool doOverlap(const Interval<T>* i1, const Interval<T>* i2) const {
        return (i1->low <= i2->high && i2->low <= i1->high);
    }

    const Interval<T>* overlapSearch(const Node<T>* node, const Interval<T>& i) const {
        if (!node)
            return nullptr;

        if (doOverlap(node->interval.get(), &i))
            return node->interval.get();

        if (node->left && node->left->max >= i.low)
            return overlapSearch(node->left.get(), i);

        return overlapSearch(node->right.get(), i);
    }

    std::unique_ptr<Node<T>> cloneNode(const std::unique_ptr<Node<T>>& node) const {
        if (!node)
            return nullptr;

        auto newNode = std::make_unique<Node<T>>(std::make_unique<Interval<T>>(*node->interval));
        newNode->max = node->max;
        newNode->left = cloneNode(node->left);
        newNode->right = cloneNode(node->right);
        return newNode;
    }

public:
    IntervalTree() : root(nullptr) {}

    IntervalTree(const IntervalTree<T>& other) {
        root = cloneNode(other.root);
    }

    void insert(T low, T high) {
        auto interval = std::make_unique<Interval<T>>(low, high);
        root = insert(std::move(root), std::move(interval));
    }

    void insert(Interval<T> interval) {
        auto i = std::make_unique<Interval<T>>(interval);
        root = insert(std::move(root), std::move(i));
    }

    const Interval<T>* searchOverlap(T low, T high) const {
        Interval<T> query(low, high);
        return overlapSearch(root.get(), query);
    }

    bool isIn(Interval<T> interval) {
        const Interval<T>* overlappingInterval = searchOverlap(interval.low(), interval.high());
        return overlappingInterval != nullptr;
    }
};
