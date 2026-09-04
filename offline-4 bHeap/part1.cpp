// Binomial Min-Heap (Part A) -- C++17
//
// Maintains two binomial min-heaps, H1 and H2, and supports:
//     I h x       Insert x into Hh                         O(log n)
//     F h         Find Min of Hh                            O(log n)
//     E h         Extract Min of Hh                          O(log n)
//     D h x y     Decrease key x to y in Hh                  O(log n)*
//     R h x       Remove key x from Hh                       O(log n)*
//     U h1 h2     Hh1 <- Union(Hh1, Hh2); Hh2 becomes empty  O(log n)
//     P h         Print Hh in the fixed checking format
//
// Commands are read from input.txt (one command per line). Every line of
// required output (from F, E, and P) is written to output.txt AND printed
// to the console, identically.
//
// Design notes (mirrors the Python version):
//   - Each binomial tree node stores: key, degree, parent, child (first
//     child), sibling (next sibling in the same root/child list).
//   - Root lists are kept, at all times, sorted in strictly increasing
//     order of degree (the standard binomial-heap invariant), which is
//     exactly what CLRS's BINOMIAL-HEAP-UNION produces.
//   - Union follows the CLRS merge-then-consolidate algorithm, which
//     implements the assignment's convention verbatim:
//         * merge root lists by degree (stable: first heap's roots come
//           before the second heap's roots on a degree tie)
//         * walk the merged list; if the current root and the next root
//           have the same degree AND the root after that also has that
//           same degree (three in a row), skip linking and just advance
//           -- i.e. never link the first two of three equal-degree roots
//         * otherwise link the two equal-degree roots, with the smaller
//           key becoming the parent and the other root becoming its
//           FIRST child (prepended to the child list)
//   - Decrease-Key walks key values up toward the root, exchanging key
//     values between a node and its parent (never physically relinking
//     nodes), exactly as specified.
//   - Remove-Key decreases the key below the minimum valid input key
//     (-10^9) and then performs Extract-Min, per the assignment's
//     sentinel convention (using a sentinel strictly below -10^9, since
//     -10^9 itself may already be a valid key in the heap).
//   - A global hash map maps "current key value" -> node, since keys are
//     unique across both heaps at any instant. Decrease-Key updates this
//     map every time it swaps a key value between two nodes.

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

using ll = long long;

static const ll SENTINEL = -2000000000LL; // strictly below min valid key (-1e9)

struct Node {
    ll key;
    int degree = 0;
    Node* parent = nullptr;
    Node* child = nullptr;
    Node* sibling = nullptr;
    explicit Node(ll k) : key(k) {}
};

struct BinomialHeap {
    Node* head = nullptr;
    long long size = 0;
};

// y becomes the FIRST child of z (z's key is smaller).
static void link(Node* y, Node* z) {
    y->parent = z;
    y->sibling = z->child;
    z->child = y;
    z->degree += 1;
}

static std::vector<Node*> toList(Node* head) {
    std::vector<Node*> out;
    while (head != nullptr) {
        out.push_back(head);
        head = head->sibling;
    }
    return out;
}

// Merge two root lists into one sorted by non-decreasing degree.
// Stable: on a degree tie, a's root precedes b's root.
static Node* mergeRootLists(Node* aHead, Node* bHead) {
    std::vector<Node*> a = toList(aHead);
    std::vector<Node*> b = toList(bHead);
    std::vector<Node*> merged;
    merged.reserve(a.size() + b.size());
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i]->degree <= b[j]->degree) merged.push_back(a[i++]);
        else merged.push_back(b[j++]);
    }
    while (i < a.size()) merged.push_back(a[i++]);
    while (j < b.size()) merged.push_back(b[j++]);

    for (size_t k = 0; k + 1 < merged.size(); ++k) merged[k]->sibling = merged[k + 1];
    if (!merged.empty()) {
        merged.back()->sibling = nullptr;
        return merged.front();
    }
    return nullptr;
}

// CLRS BINOMIAL-HEAP-UNION, operating on head pointers; returns new head.
static Node* unionHeads(Node* aHead, Node* bHead) {
    Node* head = mergeRootLists(aHead, bHead);
    if (head == nullptr) return nullptr;

    Node* prevX = nullptr;
    Node* x = head;
    Node* nextX = x->sibling;

    while (nextX != nullptr) {
        if ((x->degree != nextX->degree) ||
            (nextX->sibling != nullptr && nextX->sibling->degree == x->degree)) {
            // different degrees, OR three consecutive roots share this degree:
            // do not link the first two -- just advance.
            prevX = x;
            x = nextX;
        } else {
            // exactly two consecutive roots share this degree: link them.
            if (x->key < nextX->key) {
                x->sibling = nextX->sibling;
                link(nextX, x);
            } else {
                if (prevX == nullptr) head = nextX;
                else prevX->sibling = nextX;
                link(x, nextX);
                x = nextX;
            }
        }
        nextX = x->sibling;
    }

    return head;
}

static void unionHeaps(BinomialHeap& h1, BinomialHeap& h2) {
    Node* newHead = unionHeads(h1.head, h2.head);
    h1.head = newHead;
    h1.size += h2.size;
    h2.head = nullptr;
    h2.size = 0;
}

static void insertKey(BinomialHeap& h, ll key, std::unordered_map<ll, Node*>& keyMap) {
    Node* node = new Node(key);
    keyMap[key] = node;
    h.head = unionHeads(node, h.head);
    h.size += 1;
}

static ll findMin(const BinomialHeap& h) {
    Node* node = h.head;
    ll best = node->key;
    node = node->sibling;
    while (node != nullptr) {
        if (node->key < best) best = node->key;
        node = node->sibling;
    }
    return best;
}

static ll extractMin(BinomialHeap& h, std::unordered_map<ll, Node*>& keyMap) {
    Node* prevOfMin = nullptr;
    Node* p = nullptr;
    Node* c = h.head;
    Node* minNode = h.head;

    while (c != nullptr) {
        if (c->key < minNode->key) {
            minNode = c;
            prevOfMin = p;
        }
        p = c;
        c = c->sibling;
    }

    // remove minNode from the root list
    if (prevOfMin == nullptr) h.head = minNode->sibling;
    else prevOfMin->sibling = minNode->sibling;

    // reverse minNode's child list (degrees k-1..0) into ascending order (0..k-1)
    Node* rev = nullptr;
    Node* child = minNode->child;
    while (child != nullptr) {
        Node* nxt = child->sibling;
        child->sibling = rev;
        child->parent = nullptr;
        rev = child;
        child = nxt;
    }

    h.head = unionHeads(h.head, rev);
    h.size -= 1;

    ll result = minNode->key;
    keyMap.erase(minNode->key);
    delete minNode;
    return result;
}

static void decreaseKey(Node* node, ll newKey, std::unordered_map<ll, Node*>& keyMap) {
    ll oldKey = node->key;
    keyMap.erase(oldKey);
    node->key = newKey;
    keyMap[newKey] = node;

    Node* y = node;
    Node* z = y->parent;
    while (z != nullptr && y->key < z->key) {
        std::swap(y->key, z->key);
        keyMap[y->key] = y;
        keyMap[z->key] = z;
        y = z;
        z = y->parent;
    }
}

static void removeKey(BinomialHeap& h, Node* node, std::unordered_map<ll, Node*>& keyMap) {
    decreaseKey(node, SENTINEL, keyMap);
    extractMin(h, keyMap);
}

static void printHeap(int hId, const BinomialHeap& h, std::vector<std::string>& outLines) {
    outLines.push_back("Printing Binomial Heap H" + std::to_string(hId));
    outLines.push_back("Heap size: " + std::to_string(h.size));
    if (h.size == 0) {
        outLines.push_back("Heap H" + std::to_string(hId) + " is empty.");
        return;
    }

    Node* node = h.head;
    while (node != nullptr) {
        outLines.push_back("Binomial Tree, B" + std::to_string(node->degree));
        std::vector<Node*> levelNodes = {node};
        int level = 0;
        while (!levelNodes.empty()) {
            std::vector<ll> keys;
            keys.reserve(levelNodes.size());
            for (Node* n : levelNodes) keys.push_back(n->key);
            std::sort(keys.begin(), keys.end());

            std::ostringstream oss;
            oss << "Level " << level << ": ";
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i) oss << ' ';
                oss << keys[i];
            }
            outLines.push_back(oss.str());

            std::vector<Node*> nextLevel;
            for (Node* n : levelNodes) {
                Node* c = n->child;
                while (c != nullptr) {
                    nextLevel.push_back(c);
                    c = c->sibling;
                }
            }
            levelNodes = std::move(nextLevel);
            level += 1;
        }
        node = node->sibling;
    }
}

int main() {
    std::ifstream fin("input.txt");
    if (!fin) {
        std::cerr << "Could not open input.txt\n";
        return 1;
    }

    std::unordered_map<int, BinomialHeap> heaps;
    heaps[1] = BinomialHeap();
    heaps[2] = BinomialHeap();
    std::unordered_map<ll, Node*> keyMap;

    std::vector<std::string> outLines;

    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) continue; // skip blank lines

        if (cmd == "I") {
            int h; ll x;
            iss >> h >> x;
            insertKey(heaps[h], x, keyMap);
        } else if (cmd == "F") {
            int h;
            iss >> h;
            outLines.push_back("Find Min returned: " + std::to_string(findMin(heaps[h])));
        } else if (cmd == "E") {
            int h;
            iss >> h;
            ll result = extractMin(heaps[h], keyMap);
            outLines.push_back("Extract Min returned: " + std::to_string(result));
        } else if (cmd == "D") {
            int h; ll x, y;
            iss >> h >> x >> y;
            Node* node = keyMap[x];
            decreaseKey(node, y, keyMap);
        } else if (cmd == "R") {
            int h; ll x;
            iss >> h >> x;
            Node* node = keyMap[x];
            removeKey(heaps[h], node, keyMap);
        } else if (cmd == "U") {
            int h1, h2;
            iss >> h1 >> h2;
            unionHeaps(heaps[h1], heaps[h2]);
        } else if (cmd == "P") {
            int h;
            iss >> h;
            printHeap(h, heaps[h], outLines);
        } else {
            std::cerr << "Unrecognized command: " << line << "\n";
            return 1;
        }
    }

    std::ofstream fout("output.txt");
    for (const auto& s : outLines) {
        fout << s << "\n";
        std::cout << s << "\n";
    }

    return 0;
}