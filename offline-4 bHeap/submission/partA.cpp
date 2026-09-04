#include<iostream>
#include<vector>
#include<fstream>
#include<unordered_map>
#include<algorithm>
#include<sstream>
#include<string>
using namespace std;
const long long SENTINEL = -2000000000LL; 
struct node {
    long long key;
    int degree = 0;
    node* parent = nullptr;
    node* child = nullptr;
    node* sibling = nullptr;
    explicit node(long long k){
        key = k;
    }
};

struct binomial_heap {
    node* head = nullptr;
    long long size = 0;
};
static void link(node* y, node* z) {
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
    int i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i]->degree <= b[j]->degree) merged.push_back(a[i++]);
        else merged.push_back(b[j++]);
    }
    while (i < a.size()) merged.push_back(a[i++]);
    while (j < b.size()) merged.push_back(b[j++]);

    for (int k = 0; k + 1 < merged.size(); ++k) merged[k]->sibling = merged[k + 1];
    if (!merged.empty()) {
        merged.back()->sibling = nullptr;
        return merged.front();
    }
    return nullptr;
}

static node* unionHeads(node* aHead, node* bHead) {
    node* head = mergeRootLists(aHead, bHead);
    if (head == nullptr) return nullptr;

    node* prevX = nullptr;
    node* x = head;
    node* nextX = x->sibling;

    while (nextX != nullptr) {
        if ((x->degree != nextX->degree) || (nextX->sibling != nullptr && nextX->sibling->degree == x->degree)) {
            prevX = x;
            x = nextX;
        } else {
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

void insert_key(binomial_heap& h, long long k, unordered_map<long long, node*>& keymap){
    node* n = new node(k);
    keymap[k] = n;
    h.head = unionHeads(n, h.head);
    h.size += 1;
}
long long findMin(const binomial_heap& h) {
    node* node = h.head;
    long long best = node->key;
    node = node->sibling;
    while (node != nullptr) {
        if (node->key < best) best = node->key;
        node = node->sibling;
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

    if (prevofmin == nullptr) h.head = minnode->sibling;
    else prevofmin->sibling = minnode->sibling;

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
            for (node* ln : levelNodes) keys.push_back(ln->key);
            sort(keys.begin(), keys.end());

            ostringstream oss;              // <-- fresh stream every level
            oss << "Level " << level << ": ";
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i) oss << ' ';
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
            level += 1;
        }
        n = n->sibling;
    }
}

int main(int argc, char* argv[])
{
    if(argc!=3){
        cout<<"Usage: ./a.out input.txt output.txt"<<endl;
        return 1;
    }
    ifstream input(argv[1]);
    ofstream output(argv[2]);

    unordered_map<int, binomial_heap> heaps;
    heaps[1] = binomial_heap();
    heaps[2] = binomial_heap();
    unordered_map<long long, node*> keymap;
    string line;
    while(input>>line){
        if(line=="I"){
            long long x;
            int h;
            input>>h>>x;
            insert_key(heaps[h],x,keymap);
        }
        else if(line=="F"){
            int h;
            input>>h;
            output<<"Find Min returned: " <<findMin(heaps[h])<<endl;
        }
        else if(line=="E"){
            int h;
            input>>h;
            output<<"Extract Min returned: " <<extractMin(heaps[h], keymap)<<endl;
        }
        else if(line=="D"){
            int h,x,y;
            input>>h>>x>>y;
            node* n = keymap[x];
            decreasekey(n, y, keymap);
        }
        else if(line=="R"){
            int h,x;
            input>>h>>x;
            node* n = keymap[x];
            removeKey(heaps[h], n, keymap);
        }
        else if(line=="U"){
            int h1,h2;
            input>>h1>>h2;
            unionHeaps(heaps[h1], heaps[h2]);
        }
        else if(line=="P"){
            int h;
            input>>h;
            printHeap(h, heaps[h], output);
        }
    }
}