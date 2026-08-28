#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <utility>
using namespace std;

// ============================================================
// SINGLE-SOURCE CONFIGURATION
// ============================================================

const int INITIAL_SIZE = 13;
const double EXPAND_LOAD = 0.50;
const double COMPACT_LOAD = 0.25;

const int C1 = 1;
const int C2 = 1;

// ============================================================
// PRIME NUMBER UTILITIES
// ============================================================

bool isPrime(int n)
{
    if (n < 2)
        return false;

    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;

    for (int i = 3; i * i <= n; i += 2)
    {
        if (n % i == 0)
            return false;
    }

    return true;
}

int nextPrime(int n)
{
    while (!isPrime(n))
        n++;

    return n;
}

int previousPrime(int n)
{
    while (n >= 2 && !isPrime(n))
        n--;

    return n;
}

// ============================================================
// HASH FUNCTIONS
// ============================================================

// Hash1: Polynomial rolling hash
unsigned long long Hash1(const string &key)
{

    const unsigned long long BASE = 31;

    unsigned long long hash = 0;

    for (char c : key)
    {
        hash = hash * BASE + (unsigned char)c;
    }

    return hash;
}

// Hash2: FNV-1a
unsigned long long Hash2(const string &key)
{

    const unsigned long long OFFSET =
        1469598103934665603ULL;

    const unsigned long long PRIME =
        1099511628211ULL;

    unsigned long long hash = OFFSET;

    for (char c : key)
    {
        hash ^= (unsigned char)c;
        hash *= PRIME;
    }

    return hash;
}

// Auxiliary hash for probing
int auxHash(unsigned long long hash, int tableSize)
{

    // Since tableSize is prime,
    // this guarantees a non-zero step.
    return 1 + (hash % (tableSize - 1));
}

// ============================================================
// NODE FOR CHAINING
// ============================================================

struct ChainNode
{

    string key;
    int value;

    ChainNode *next;

    ChainNode(string k, int v)
    {
        key = k;
        value = v;
        next = nullptr;
    }
};

// ============================================================
// SLOT FOR OPEN ADDRESSING
// ============================================================

enum State
{
    EMPTY,
    OCCUPIED,
    DELETED
};

struct Slot
{

    string key;
    int value;

    State state;

    Slot()
    {
        state = EMPTY;
        value = 0;
    }
};

// ============================================================
// HASH TABLE CLASS
// ============================================================

class HashTable
{

private:
    int tableSize;
    int numberOfElements;

    // Number of insertions since last expansion
    int insertionsSinceExpansion;

    // Number of deletions since last compaction
    int deletionsSinceCompaction;

    // Chaining table
    vector<ChainNode *> chainTable;

    // Open addressing table
    vector<Slot> openTable;

    // Which hash function is being used
    int hashType;

    // 1 = chaining
    // 2 = double hashing
    // 3 = custom probing
    int collisionMethod;

    // --------------------------------------------------------
    // Get hash value
    // --------------------------------------------------------

    unsigned long long getHash(const string &key) const
    {

        if (hashType == 1)
            return Hash1(key);

        return Hash2(key);
    }

    // --------------------------------------------------------
    // Index from hash
    // --------------------------------------------------------

    int getIndex(const string &key) const
    {

        return getHash(key) % tableSize;
    }

    // --------------------------------------------------------
    // Probe position
    // --------------------------------------------------------

    int probeIndex(
        const string &key,
        int i) const
    {

        unsigned long long h = getHash(key);

        long long index;

        if (collisionMethod == 2)
        {

            // Double hashing
            index =
                h + 1LL * i * auxHash(h, tableSize);
        }

        else
        {

            // Custom probing
            index =
                h + 1LL * C1 * i * auxHash(h, tableSize) + 1LL * C2 * i * i;
        }

        return index % tableSize;
    }

    // --------------------------------------------------------
    // Create new table
    // --------------------------------------------------------

    void initializeTables(int size)
    {

        tableSize = size;

        chainTable.assign(
            tableSize,
            nullptr);

        openTable.assign(
            tableSize,
            Slot());
    }

    // --------------------------------------------------------
    // Resize
    // --------------------------------------------------------

    void resizeTable(int newSize)
    {

        newSize = nextPrime(newSize);

        // Store all existing elements
        vector<pair<string, int>> elements;

        if (collisionMethod == 1)
        {

            // Chaining
            for (int i = 0; i < tableSize; i++)
            {

                ChainNode *cur =
                    chainTable[i];

                while (cur != nullptr)
                {

                    elements.push_back({cur->key,
                                        cur->value});

                    cur = cur->next;
                }
            }
        }

        else
        {

            // Open addressing
            for (int i = 0; i < tableSize; i++)
            {

                if (openTable[i].state == OCCUPIED)
                {

                    elements.push_back({openTable[i].key,
                                        openTable[i].value});
                }
            }
        }

        // Delete old chaining nodes
        if (collisionMethod == 1)
        {

            for (int i = 0; i < tableSize; i++)
            {

                ChainNode *cur =
                    chainTable[i];

                while (cur != nullptr)
                {

                    ChainNode *temp = cur;

                    cur = cur->next;

                    delete temp;
                }
            }
        }

        // Create new table
        initializeTables(newSize);

        // Reinsert elements
        for (auto &p : elements)
        {

            insertWithoutResize(
                p.first,
                p.second);
        }
    }

    // --------------------------------------------------------
    // Insert without checking resize
    // --------------------------------------------------------

    bool insertWithoutResize(
        const string &key,
        int value)
    {

        if (collisionMethod == 1)
        {

            int index = getIndex(key);

            ChainNode *cur =
                chainTable[index];

            while (cur != nullptr)
            {

                if (cur->key == key)
                {

                    return false;
                }

                cur = cur->next;
            }

            ChainNode *node =
                new ChainNode(key, value);

            node->next =
                chainTable[index];

            chainTable[index] = node;

            numberOfElements++;

            return true;
        }

        // Open addressing
        for (int i = 0; i < tableSize; i++)
        {

            int index =
                probeIndex(key, i);

            if (openTable[index].state == OCCUPIED)
            {

                if (openTable[index].key == key)
                    return false;
            }
            else
            {

                openTable[index].key = key;
                openTable[index].value = value;
                openTable[index].state = OCCUPIED;

                numberOfElements++;

                return true;
            }
        }

        return false;
    }

    // --------------------------------------------------------
    // Should expand?
    // --------------------------------------------------------

    bool shouldExpand() const
    {

        double load =
            (double)numberOfElements / tableSize;

        if (load <= EXPAND_LOAD)
            return false;

        /*
           At least n/2 insertions since previous expansion.

           We use the number of elements present immediately
           after the previous resize as the reference n.
        */

        int required =
            max(1, tableSize / 2);

        return insertionsSinceExpansion >= required;
    }

    // --------------------------------------------------------
    // Should compact?
    // --------------------------------------------------------

    bool shouldCompact() const
    {

        if (tableSize == INITIAL_SIZE)
            return false;

        double load =
            (double)numberOfElements / tableSize;

        if (load >= COMPACT_LOAD)
            return false;

        int required =
            max(1, tableSize / 2);

        return deletionsSinceCompaction >= required;
    }

public:
    // --------------------------------------------------------
    // Constructor
    // --------------------------------------------------------

    HashTable(
        int hashType,
        int collisionMethod)
    {

        this->hashType = hashType;

        this->collisionMethod =
            collisionMethod;

        numberOfElements = 0;

        insertionsSinceExpansion = 0;

        deletionsSinceCompaction = 0;

        initializeTables(INITIAL_SIZE);
    }

    // --------------------------------------------------------
    // Destructor
    // --------------------------------------------------------

    ~HashTable()
    {

        if (collisionMethod == 1)
        {

            for (int i = 0; i < tableSize; i++)
            {

                ChainNode *cur =
                    chainTable[i];

                while (cur != nullptr)
                {

                    ChainNode *temp = cur;

                    cur = cur->next;

                    delete temp;
                }
            }
        }
    }

    // --------------------------------------------------------
    // INSERT
    // --------------------------------------------------------

    bool insert(
        const string &key,
        int value)
    {

        bool inserted =
            insertWithoutResize(
                key,
                value);

        if (!inserted)
            return false;

        insertionsSinceExpansion++;

        // Check expansion
        if (shouldExpand())
        {

            int newSize =
                nextPrime(2 * tableSize + 1);

            resizeTable(newSize);

            insertionsSinceExpansion = 0;
        }

        return true;
    }

    // --------------------------------------------------------
    // SEARCH
    // --------------------------------------------------------

    pair<bool, int> search(
        const string &key,
        long long &hits) const
    {

        hits = 0;

        // ----------------------------------------------------
        // CHAINING
        // ----------------------------------------------------

        if (collisionMethod == 1)
        {

            int index =
                getIndex(key);

            ChainNode *cur =
                chainTable[index];

            while (cur != nullptr)
            {

                hits++;

                if (cur->key == key)
                {

                    return {
                        true,
                        cur->value};
                }

                cur = cur->next;
            }

            return {
                false,
                -1};
        }

        // ----------------------------------------------------
        // OPEN ADDRESSING
        // ----------------------------------------------------

        for (int i = 0; i < tableSize; i++)
        {

            int index =
                probeIndex(key, i);

            hits++;

            if (openTable[index].state == EMPTY)
            {

                return {
                    false,
                    -1};
            }

            if (
                openTable[index].state == OCCUPIED &&
                openTable[index].key == key)
            {

                return {
                    true,
                    openTable[index].value};
            }
        }

        return {
            false,
            -1};
    }

    // --------------------------------------------------------
    // DELETE
    // --------------------------------------------------------

    bool remove(
        const string &key)
    {

        bool removed = false;

        // ----------------------------------------------------
        // CHAINING
        // ----------------------------------------------------

        if (collisionMethod == 1)
        {

            int index =
                getIndex(key);

            ChainNode *cur =
                chainTable[index];

            ChainNode *prev =
                nullptr;

            while (cur != nullptr)
            {

                if (cur->key == key)
                {

                    if (prev == nullptr)
                    {

                        chainTable[index] =
                            cur->next;
                    }
                    else
                    {

                        prev->next =
                            cur->next;
                    }

                    delete cur;

                    numberOfElements--;

                    removed = true;

                    break;
                }

                prev = cur;

                cur = cur->next;
            }
        }

        // ----------------------------------------------------
        // OPEN ADDRESSING
        // ----------------------------------------------------

        else
        {

            for (int i = 0; i < tableSize; i++)
            {

                int index =
                    probeIndex(key, i);

                if (openTable[index].state == EMPTY)
                    break;

                if (
                    openTable[index].state == OCCUPIED &&
                    openTable[index].key == key)
                {

                    openTable[index].state =
                        DELETED;

                    openTable[index].key.clear();

                    numberOfElements--;

                    removed = true;

                    break;
                }
            }
        }

        if (removed)
        {

            deletionsSinceCompaction++;

            // Check compaction
            if (shouldCompact())
            {

                int newSize =
                    max(
                        INITIAL_SIZE,
                        previousPrime(
                            tableSize / 2));

                if (newSize >= INITIAL_SIZE)
                {

                    resizeTable(newSize);
                }

                deletionsSinceCompaction = 0;
            }
        }

        return removed;
    }

    // --------------------------------------------------------
    // GETTERS
    // --------------------------------------------------------

    int size() const
    {
        return tableSize;
    }

    int count() const
    {
        return numberOfElements;
    }

    double loadFactor() const
    {

        return (double)numberOfElements / tableSize;
    }
};

// ============================================================
// RANDOM WORD GENERATOR
// ============================================================

string generateWord(int length)
{

    static const string letters =
        "abcdefghijklmnopqrstuvwxyz";

    static random_device rd;

    static mt19937 gen(rd());

    static uniform_int_distribution<int> dist(
        0,
        letters.size() - 1);

    string word;

    for (int i = 0; i < length; i++)
    {

        word += letters[dist(gen)];
    }

    return word;
}

// ============================================================
// GENERATE UNIQUE WORDS
// ============================================================

vector<string> generateUniqueWords(
    int number,
    int length)
{

    vector<string> words;

    unordered_set<string> used;

    while ((int)words.size() < number)
    {

        string word =
            generateWord(length);

        if (used.insert(word).second)
        {

            words.push_back(word);
        }
    }

    return words;
}

// ============================================================
// COUNT HASH COLLISIONS
// ============================================================

long long countCollisions(
    const vector<string> &words,
    int hashType,
    int tableSize)
{

    unordered_map<int, int> frequency;

    long long collisions = 0;

    for (const string &word : words)
    {

        unsigned long long h;

        if (hashType == 1)
            h = Hash1(word);
        else
            h = Hash2(word);

        int index =
            h % tableSize;

        frequency[index]++;
    }

    /*
       If k different keys map to the same
       hash value, there are k-1 collisions.
    */

    for (auto &p : frequency)
    {

        if (p.second > 1)
        {

            collisions +=
                p.second - 1;
        }
    }

    return collisions;
}

// ============================================================
// EXPERIMENT
// ============================================================

struct Result
{

    long long collisions;

    double averageHits;
};

Result runExperiment(
    const vector<string> &words,
    int hashType,
    int collisionMethod)
{

    HashTable table(
        hashType,
        collisionMethod);

    // --------------------------------------------------------
    // INSERT 10,000 UNIQUE WORDS
    // --------------------------------------------------------

    for (int i = 0; i < (int)words.size(); i++)
    {

        table.insert(
            words[i],
            i + 1);
    }

    // --------------------------------------------------------
    // RANDOMLY SELECT 1,000 WORDS
    // --------------------------------------------------------

    vector<int> indices(
        words.size());

    iota(
        indices.begin(),
        indices.end(),
        0);

    random_device rd;

    mt19937 gen(rd());

    shuffle(
        indices.begin(),
        indices.end(),
        gen);

    long long totalHits = 0;

    int searches = 1000;

    for (int i = 0; i < searches; i++)
    {

        long long hits;

        table.search(
            words[indices[i]],
            hits);

        totalHits += hits;
    }

    double averageHits =
        (double)totalHits / searches;

    /*
       Collision definition in the assignment:

       "number of instances in which a hash function
       maps distinct keys to the same hash value."
    */

    int evaluationTableSize =
        INITIAL_SIZE;

    // For collision comparison, use the initial
    // hash table size as the common hash range.
    long long collisions =
        countCollisions(
            words,
            hashType,
            evaluationTableSize);

    return {
        collisions,
        averageHits};
}

// ============================================================
// MAIN
// ============================================================

int main()
{

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // --------------------------------------------------------
    // USER INPUT
    // --------------------------------------------------------

    int wordLength;

    cout << "Enter word length: ";

    cin >> wordLength;

    // --------------------------------------------------------
    // GENERATE 10,000 UNIQUE WORDS
    // --------------------------------------------------------

    cout << "\nGenerating words...\n";

    vector<string> words =
        generateUniqueWords(
            10000,
            wordLength);

    cout << "10000 unique words generated.\n";

    // --------------------------------------------------------
    // RUN ALL 6 EXPERIMENTS
    // --------------------------------------------------------

    Result h1Chain =
        runExperiment(
            words,
            1,
            1);

    Result h1Double =
        runExperiment(
            words,
            1,
            2);

    Result h1Custom =
        runExperiment(
            words,
            1,
            3);

    Result h2Chain =
        runExperiment(
            words,
            2,
            1);

    Result h2Double =
        runExperiment(
            words,
            2,
            2);

    Result h2Custom =
        runExperiment(
            words,
            2,
            3);

    // --------------------------------------------------------
    // REPORT
    // --------------------------------------------------------

    cout << "\n\n";
    cout << "============================================================\n";
    cout << "                    HASH TABLE REPORT\n";
    cout << "============================================================\n\n";

    cout << left
         << setw(25) << "Method"
         << setw(20) << "Hash1 Collisions"
         << setw(20) << "Hash1 Avg Hits"
         << setw(20) << "Hash2 Collisions"
         << setw(20) << "Hash2 Avg Hits"
         << '\n';

    cout << string(105, '-') << '\n';

    cout << left
         << setw(25) << "Chaining"
         << setw(20) << h1Chain.collisions
         << setw(20) << fixed << setprecision(3)
         << h1Chain.averageHits
         << setw(20) << h2Chain.collisions
         << setw(20)
         << h2Chain.averageHits
         << '\n';

    cout << left
         << setw(25) << "Double Hashing"
         << setw(20) << h1Double.collisions
         << setw(20)
         << h1Double.averageHits
         << setw(20) << h2Double.collisions
         << setw(20)
         << h2Double.averageHits
         << '\n';

    cout << left
         << setw(25) << "Custom Probing"
         << setw(20) << h1Custom.collisions
         << setw(20)
         << h1Custom.averageHits
         << setw(20) << h2Custom.collisions
         << setw(20)
         << h2Custom.averageHits
         << '\n';

    cout << "\n============================================================\n";

    return 0;
}