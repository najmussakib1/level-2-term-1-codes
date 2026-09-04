#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

const long long SENTINEL = -2000000000LL;

struct node {
    long long key;
    int degree = 0;

    node* parent = nullptr;
    node* child = nullptr;
    node* sibling = nullptr;

    explicit node(long long k) : key(k) {}
};

struct binomial_heap {
    node* head = nullptr;
    long long size = 0;
};

struct link_record {
    int degree;
    long long parentKey;
    long long childKey;
};

static void printBoth(ostream& output, const string& s) {
    output << s << '\n';
    cout << s << '\n';
}

static void link(node* y, node* z, vector<link_record>* records = nullptr) {
    if (records != nullptr) {
        link_record rec;
        rec.degree = z->degree;
        rec.parentKey = z->key;
        rec.childKey = y->key;
        records->push_back(rec);
    }

    y->parent = z;
    y->sibling = z->child;
    z->child = y;
    z->degree += 1;
}

static vector<node*> toList(node* head) {
    vector<node*> out;

    while (head != nullptr) {
        out.push_back(head);
        head = head->sibling;
    }

    return out;
}

static node* mergeRootLists(node* aHead, node* bHead) {
    vector<node*> a = toList(aHead);
    vector<node*> b = toList(bHead);
    vector<node*> merged;

    merged.reserve(a.size() + b.size());

    int i = 0;
    int j = 0;

    while (i < static_cast<int>(a.size()) && j < static_cast<int>(b.size())) {
        if (a[i]->degree <= b[j]->degree) {
            merged.push_back(a[i]);
            ++i;
        } else {
            merged.push_back(b[j]);
            ++j;
        }
    }

    while (i < static_cast<int>(a.size())) {
        merged.push_back(a[i]);
        ++i;
    }

    while (j < static_cast<int>(b.size())) {
        merged.push_back(b[j]);
        ++j;
    }

    for (int k = 0; k + 1 < static_cast<int>(merged.size()); ++k) {
        merged[k]->sibling = merged[k + 1];
    }

    if (!merged.empty()) {
        merged.back()->sibling = nullptr;
        return merged.front();
    }

    return nullptr;
}

static node* unionHeads(node* aHead, node* bHead, vector<link_record>* records = nullptr) {
    node* head = mergeRootLists(aHead, bHead);

    if (head == nullptr) {
        return nullptr;
    }

    node* prevX = nullptr;
    node* x = head;
    node* nextX = x->sibling;

    while (nextX != nullptr) {
        if ((x->degree != nextX->degree) ||
            (nextX->sibling != nullptr && nextX->sibling->degree == x->degree)) {
            prevX = x;
            x = nextX;
        } else {
            if (x->key < nextX->key) {
                x->sibling = nextX->sibling;
                link(nextX, x, records);
            } else {
                if (prevX == nullptr) {
                    head = nextX;
                } else {
                    prevX->sibling = nextX;
                }

                link(x, nextX, records);
                x = nextX;
            }
        }

        nextX = x->sibling;
    }

    return head;
}

void insert_key(binomial_heap& h, long long k, unordered_map<long long, node*>& keymap) {
    node* n = new node(k);
    keymap[k] = n;
    h.head = unionHeads(h.head, n);
    h.size += 1;
}

long long findMin(const binomial_heap& h) {
    node* current = h.head;
    long long best = current->key;

    current = current->sibling;
    while (current != nullptr) {
        if (current->key < best) {
            best = current->key;
        }
        current = current->sibling;
    }

    return best;
}

long long extractMin(binomial_heap& h, unordered_map<long long, node*>& keyMap) {
    node* prevofmin = nullptr;
    node* p = nullptr;
    node* c = h.head;
    node* minnode = h.head;

    while (c != nullptr) {
        if (c->key < minnode->key) {
            minnode = c;
            prevofmin = p;
        }
        p = c;
        c = c->sibling;
    }

    if (prevofmin == nullptr) {
        h.head = minnode->sibling;
    } else {
        prevofmin->sibling = minnode->sibling;
    }

    node* rev = nullptr;
    node* child = minnode->child;

    while (child != nullptr) {
        node* nxt = child->sibling;
        child->sibling = rev;
        child->parent = nullptr;
        rev = child;
        child = nxt;
    }

    h.head = unionHeads(h.head, rev);
    h.size -= 1;

    long long result = minnode->key;
    keyMap.erase(minnode->key);
    delete minnode;

    return result;
}

static void decreasekey(node* n, long long newKey, unordered_map<long long, node*>& keyMap) {
    long long oldKey = n->key;
    keyMap.erase(oldKey);

    n->key = newKey;
    keyMap[newKey] = n;

    node* y = n;
    node* z = y->parent;

    while (z != nullptr && y->key < z->key) {
        swap(y->key, z->key);

        keyMap[y->key] = y;
        keyMap[z->key] = z;

        y = z;
        z = y->parent;
    }
}

void removeKey(binomial_heap& h, node* n, unordered_map<long long, node*>& keyMap) {
    decreasekey(n, SENTINEL, keyMap);
    extractMin(h, keyMap);
}

void unionHeaps(binomial_heap& h1, binomial_heap& h2) {
    node* newHead = unionHeads(h1.head, h2.head);
    h1.head = newHead;
    h1.size += h2.size;
    h2.head = nullptr;
    h2.size = 0;
}

void printHeap(int hId, const binomial_heap& h, ostream& out) {
    auto emit = [&](const string& s) {
        out << s << "\n";
        cout << s << "\n";
    };

    emit("Printing Binomial Heap H" + to_string(hId));
    emit("Heap size: " + to_string(h.size));

    if (h.size == 0) {
        emit("Heap H" + to_string(hId) + " is empty.");
        return;
    }

    node* n = h.head;
    while (n != nullptr) {
        emit("Binomial Tree, B" + to_string(n->degree));

        vector<node*> levelNodes = {n};
        int level = 0;

        while (!levelNodes.empty()) {
            vector<long long> keys;
            keys.reserve(levelNodes.size());

            for (node* ln : levelNodes) {
                keys.push_back(ln->key);
            }

            sort(keys.begin(), keys.end());

            ostringstream oss;
            oss << "Level " << level << ": ";

            for (size_t i = 0; i < keys.size(); ++i) {
                if (i) {
                    oss << ' ';
                }
                oss << keys[i];
            }

            emit(oss.str());

            vector<node*> nextLevel;
            for (node* xx : levelNodes) {
                node* c = xx->child;
                while (c != nullptr) {
                    nextLevel.push_back(c);
                    c = c->sibling;
                }
            }

            levelNodes = move(nextLevel);
            ++level;
        }

        n = n->sibling;
    }
}

static long long countNodes(node* n) {
    if (n == nullptr) {
        return 0;
    }

    long long result = 1;
    node* c = n->child;

    while (c != nullptr) {
        result += countNodes(c);
        c = c->sibling;
    }

    return result;
}

static int treeHeight(node* n) {
    if (n == nullptr) {
        return 0;
    }

    int maximum = 0;
    node* c = n->child;

    while (c != nullptr) {
        maximum = max(maximum, treeHeight(c));
        c = c->sibling;
    }

    return maximum + 1;
}

static long long treeMinimum(node* root) {
    long long answer = root->key;
    vector<node*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        node* current = stack.back();
        stack.pop_back();

        answer = min(answer, current->key);

        node* c = current->child;
        while (c != nullptr) {
            stack.push_back(c);
            c = c->sibling;
        }
    }

    return answer;
}

static void visualizeNode(node* n, const string& prefix, bool isLast, bool isRoot, ostream& out) {
    if (n == nullptr) {
        return;
    }

    string connector;
    if (isRoot) {
        connector = "";
    } else if (isLast) {
        connector = "└── ";
    } else {
        connector = "├── ";
    }

    out << prefix << connector << "[" << n->key << "]";
    if (isRoot) {
        out << " ROOT";
    }
    out << "\n";

    vector<node*> children;
    node* c = n->child;
    while (c != nullptr) {
        children.push_back(c);
        c = c->sibling;
    }

    for (int i = 0; i < static_cast<int>(children.size()); ++i) {
        bool last = (i == static_cast<int>(children.size()) - 1);
        string newPrefix;

        if (isRoot) {
            newPrefix = "";
        } else if (isLast) {
            newPrefix = prefix + "    ";
        } else {
            newPrefix = prefix + "│   ";
        }

        visualizeNode(children[i], newPrefix, last, false, out);
    }
}

static void visualizeTree(node* root, ostream& out) {
    out << "\n";
    out << "------------------------------------------------------------\n";
    out << "Binomial Tree B" << root->degree << "\n";
    out << "------------------------------------------------------------\n";

    visualizeNode(root, "", true, true, out);

    out << "\n";
    out << "Tree Statistics:\n";
    out << "  Order       : B" << root->degree << "\n";
    out << "  Nodes       : " << countNodes(root) << "\n";
    out << "  Height      : " << treeHeight(root) << "\n";
    out << "  Root        : " << root->key << "\n";
    out << "  Minimum     : " << treeMinimum(root) << "\n";
}

static void visualizeHeap(int hId, const binomial_heap& h, ostream& out) {
    out << "\n";
    out << "============================================================\n";
    out << "                 BINOMIAL HEAP H" << hId << "\n";
    out << "============================================================\n";
    out << "Heap size: " << h.size << "\n";

    if (h.head == nullptr) {
        out << "Heap H" << hId << " is empty.\n";
        return;
    }

    int treeCount = 0;
    node* current = h.head;
    while (current != nullptr) {
        ++treeCount;
        current = current->sibling;
    }

    out << "Number of trees: " << treeCount << "\n";

    current = h.head;
    while (current != nullptr) {
        visualizeTree(current, out);
        current = current->sibling;
    }

    out << "\n";
    out << "============================================================\n";
}

static void visualizeLink(const link_record& rec, int number, ostream& out) {
    out << "\n";
    out << "******************** LINK " << number << " ********************\n";
    out << "Operation: B" << rec.degree << " + B" << rec.degree << " -> B" << rec.degree + 1 << "\n";
    out << "\n";
    out << "Parent root: [" << rec.parentKey << "]\n";
    out << "Child root : [" << rec.childKey << "]\n";
    out << "\n";
    out << "Result:\n";
    out << "    [" << rec.parentKey << "]\n";
    out << "    └── [" << rec.childKey << "]\n";
    out << "**************************************************\n";
}

static void visualizeUnion(int h1Id, int h2Id, binomial_heap& h1, binomial_heap& h2, ostream& out) {
    out << "\n";
    out << "############################################################\n";
    out << "                    BINOMIAL UNION\n";
    out << "############################################################\n";
    out << "\n";
    out << "==================== BEFORE UNION ====================\n";

    visualizeHeap(h1Id, h1, out);
    visualizeHeap(h2Id, h2, out);

    vector<link_record> records;
    node* newHead = unionHeads(h1.head, h2.head, &records);

    h1.head = newHead;
    h1.size += h2.size;
    h2.head = nullptr;
    h2.size = 0;

    out << "\n";
    out << "==================== LINKING STEPS ====================\n";

    if (records.empty()) {
        out << "No Binomial Trees were linked.\n";
    } else {
        for (int i = 0; i < static_cast<int>(records.size()); ++i) {
            visualizeLink(records[i], i + 1, out);
        }
    }

    out << "\n";
    out << "==================== AFTER UNION ====================\n";
    visualizeHeap(h1Id, h1, out);

    out << "\n";
    out << "H" << h2Id << " is now empty.\n";
    out << "\n";
    out << "############################################################\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: ./a.out input.txt output.txt" << endl;
        return 1;
    }

    ifstream input(argv[1]);
    ofstream output(argv[2]);
    ofstream visualization("visualization.txt");

    if (!input.is_open()) {
        cout << "Error opening input file." << endl;
        return 1;
    }

    if (!output.is_open()) {
        cout << "Error opening output file." << endl;
        return 1;
    }

    if (!visualization.is_open()) {
        cout << "Error opening visualization.txt." << endl;
        return 1;
    }

    unordered_map<int, binomial_heap> heaps;
    heaps[1] = binomial_heap();
    heaps[2] = binomial_heap();

    unordered_map<long long, node*> keymap;
    string line;

    while (input >> line) {
        if (line == "I") {
            long long x;
            int h;
            input >> h >> x;
            insert_key(heaps[h], x, keymap);
        } else if (line == "F") {
            int h;
            input >> h;
            long long result = findMin(heaps[h]);
            output << "Find Min returned: " << result << endl;
            cout << "Find Min returned: " << result << endl;
        } else if (line == "E") {
            int h;
            input >> h;
            long long result = extractMin(heaps[h], keymap);
            output << "Extract Min returned: " << result << endl;
            cout << "Extract Min returned: " << result << endl;
        } else if (line == "D") {
            int h;
            long long x;
            long long y;
            input >> h >> x >> y;

            node* n = keymap[x];
            decreasekey(n, y, keymap);
        } else if (line == "R") {
            int h;
            long long x;
            input >> h >> x;

            node* n = keymap[x];
            removeKey(heaps[h], n, keymap);
        } else if (line == "U") {
            int h1;
            int h2;
            input >> h1 >> h2;
            unionHeaps(heaps[h1], heaps[h2]);
        } else if (line == "P") {
            int h;
            input >> h;
            printHeap(h, heaps[h], output);
        } else if (line == "V") {
            int h;
            input >> h;

            visualizeHeap(h, heaps[h], cout);
            visualizeHeap(h, heaps[h], visualization);
        } else if (line == "VU") {
            int h1;
            int h2;
            input >> h1 >> h2;

            ostringstream buffer;
            visualizeUnion(h1, h2, heaps[h1], heaps[h2], buffer);
            string result = buffer.str();

            cout << result;
            visualization << result;
        }
    }

    unordered_map<node*, bool> deleted;
    for (auto& heapPair : heaps) {
        node* current = heapPair.second.head;

        while (current != nullptr) {
            vector<node*> stack;
            stack.push_back(current);

            while (!stack.empty()) {
                node* cur = stack.back();
                stack.pop_back();

                if (deleted[cur]) {
                    continue;
                }

                deleted[cur] = true;

                node* child = cur->child;
                while (child != nullptr) {
                    stack.push_back(child);
                    child = child->sibling;
                }

                delete cur;
            }

            current = current->sibling;
        }
    }

    input.close();
    output.close();
    visualization.close();

    return 0;
}