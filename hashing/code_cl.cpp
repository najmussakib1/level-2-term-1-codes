// CSE208 - Data Structures and Algorithm Sessional
// Implementation of Hash Table Data Structure
//
// Supports:
//   - Dynamic hash table (insert / search / delete), size always prime
//   - Two hash functions: Hash1 (djb2) and Hash2 (FNV-1a)
//   - Three collision resolution techniques: Chaining, Double Hashing, Custom Probing
//   - Random unique word generator
//   - Report generation (collisions + average search hits) for 10,000 words of length 10
//
// Compile:  g++ -O2 -std=c++17 hashtable.cpp -o hashtable
// Run:      ./hashtable

#include <vector>
#include <list>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <iomanip>
using namespace std;

// =====================================================================
// Single-source configuration parameters (Section 1)
// =====================================================================
struct Config {
    static constexpr long long INITIAL_SIZE   = 13;     // initial table size (must be prime)
    static constexpr double    LOAD_HIGH      = 0.5;    // expand if load factor exceeds this
    static constexpr double    LOAD_LOW       = 0.25;   // compact if load factor falls below this
};

// =====================================================================
// Prime helpers
// =====================================================================
bool isPrime(long long n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

// smallest prime strictly greater than `bound`
long long smallestPrimeAbove(double bound) {
    long long c = (long long)floor(bound) + 1;
    while (!isPrime(c)) c++;
    return c;
}

// largest prime strictly smaller than `bound`
long long largestPrimeBelow(double bound) {
    long long c = (long long)ceil(bound) - 1;
    while (c > 1 && !isPrime(c)) c--;
    if (c < 2) c = 2;
    return c;
}

// =====================================================================
// Hash functions (Section 3)
// =====================================================================

// Hash1: djb2 (Dan Bernstein) - widely used string hash
unsigned long long hash1(const string& key) {
    unsigned long long h = 5381;
    for (unsigned char c : key) h = ((h << 5) + h) + c;   // h*33 + c
    return h;
}

// Hash2: FNV-1a - well established, good avalanche behaviour
unsigned long long hash2(const string& key) {
    unsigned long long h = 14695981039346656037ULL;
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

using HashFunc = function<unsigned long long(const string&)>;

// A simple, independent secondary hash used to build the probing step
// (auxHash(k) in Section 4.2 / 4.3). Kept intentionally simple.
unsigned long long secondaryMix(const string& key) {
    unsigned long long s = 0;
    for (unsigned char c : key) s = s * 31ULL + c;
    return s;
}

// auxHash(k): returns a step size in [1, R] that is coprime-friendly with
// table size N by deriving it from a prime R < N.
long long auxHash(const string& key, long long tableSize) {
    long long R = (tableSize > 3) ? largestPrimeBelow(tableSize) : 1;
    if (R < 1) R = 1;
    long long step = 1 + (long long)(secondaryMix(key) % (unsigned long long)R);
    return step; // in [1, R]  (never 0, so probing always advances)
}

// =====================================================================
// Technique 1: Chaining (Section 4.1)
// =====================================================================
class ChainingHashTable {
public:
    explicit ChainingHashTable(HashFunc hf) : hf(move(hf)) {
        tableSize = Config::INITIAL_SIZE;
        table.assign(tableSize, {});
        count = 0;
        insertsSinceExpansion = 0;
        deletesSinceCompaction = 0;
        collisions = 0;
    }

    // Returns false if key already exists (duplicate discarded)
    bool insert(const string& key, int value) {
        long long idx = (long long)(hf(key) % (unsigned long long)tableSize);
        for (auto& kv : table[idx])
            if (kv.first == key) return false; // duplicate

        if (!table[idx].empty()) collisions++;  // bucket already occupied by another key

        table[idx].push_back({key, value});
        count++;
        insertsSinceExpansion++;
        maybeExpand();
        return true;
    }

    bool remove(const string& key) {
        long long idx = (long long)(hf(key) % (unsigned long long)tableSize);
        auto& lst = table[idx];
        for (auto it = lst.begin(); it != lst.end(); ++it) {
            if (it->first == key) {
                lst.erase(it);
                count--;
                deletesSinceCompaction++;
                maybeCompact();
                return true;
            }
        }
        return false;
    }

    // Returns number of accesses to find key (>=1), or -1 if not found
    long long search(const string& key) const {
        long long idx = (long long)(hf(key) % (unsigned long long)tableSize);
        long long hits = 0;
        for (auto& kv : table[idx]) {
            hits++;
            if (kv.first == key) return hits;
        }
        return -1;
    }

    long long collisionCount() const { return collisions; }
    long long size() const { return tableSize; }
    long long elementCount() const { return count; }

private:
    HashFunc hf;
    vector<list<pair<string,int>>> table;
    long long tableSize, count;
    long long insertsSinceExpansion, deletesSinceCompaction, collisions;

    void maybeExpand() {
        if (tableSize == 0) return;
        double lf = (double)count / (double)tableSize;
        // expand only after at least n/2 insertions since previous expansion
        if (lf > Config::LOAD_HIGH && insertsSinceExpansion >= count / 2) {
            long long newSize = smallestPrimeAbove(2.0 * tableSize);
            rehash(newSize);
            insertsSinceExpansion = 0;
        }
    }
    void maybeCompact() {
        if (tableSize == Config::INITIAL_SIZE) return; // never compact below/at initial size
        double lf = (double)count / (double)tableSize;
        if (lf < Config::LOAD_LOW && deletesSinceCompaction >= count / 2) {
            long long newSize = largestPrimeBelow(tableSize / 2.0);
            if (newSize < Config::INITIAL_SIZE) newSize = Config::INITIAL_SIZE;
            rehash(newSize);
            deletesSinceCompaction = 0;
        }
    }
    void rehash(long long newSize) {
        vector<list<pair<string,int>>> newTable(newSize);
        for (auto& bucket : table)
            for (auto& kv : bucket)
                newTable[(long long)(hf(kv.first) % (unsigned long long)newSize)].push_back(kv);
        table = move(newTable);
        tableSize = newSize;
    }
};

// =====================================================================
// Technique 2 & 3: Open addressing (Double Hashing / Custom Probing)
// Sections 4.2 and 4.3
// =====================================================================
enum class SlotState { EMPTY, OCCUPIED, DELETED };

struct Slot {
    SlotState state = SlotState::EMPTY;
    string key;
    int value = 0;
};

class OpenAddressingHashTable {
public:
    enum class Mode { DOUBLE_HASHING, CUSTOM_PROBING };

    OpenAddressingHashTable(HashFunc hf, Mode mode, long long c1 = 1, long long c2 = 3)
        : hf(move(hf)), mode(mode), C1(c1), C2(c2) {
        tableSize = Config::INITIAL_SIZE;
        table.assign(tableSize, Slot());
        count = 0;
        insertsSinceExpansion = 0;
        deletesSinceCompaction = 0;
        collisions = 0;
    }

    bool insert(const string& key, int value) {
        long long h = (long long)(hf(key) % (unsigned long long)tableSize);
        long long step = auxHash(key, tableSize);

        bool collided = false;
        long long insertPos = -1;

        for (long long i = 0; i < tableSize; i++) {
            long long idx = probe(h, step, i);
            if (table[idx].state == SlotState::OCCUPIED) {
                if (table[idx].key == key) return false; // duplicate
                if (i == 0) collided = true;              // initial probe slot was taken
            } else {
                if (insertPos == -1) insertPos = idx;
                if (table[idx].state == SlotState::EMPTY) break;
            }
        }
        if (insertPos == -1) return false; // table full (should not happen; we resize proactively)

        table[insertPos] = {SlotState::OCCUPIED, key, value};
        if (collided) collisions++;
        count++;
        insertsSinceExpansion++;
        maybeExpand();
        return true;
    }

    long long search(const string& key) const {
        long long h = (long long)(hf(key) % (unsigned long long)tableSize);
        long long step = auxHash(key, tableSize);
        long long hits = 0;
        for (long long i = 0; i < tableSize; i++) {
            long long idx = probe(h, step, i);
            hits++;
            if (table[idx].state == SlotState::EMPTY) return -1;
            if (table[idx].state == SlotState::OCCUPIED && table[idx].key == key) return hits;
        }
        return -1;
    }

    bool remove(const string& key) {
        long long h = (long long)(hf(key) % (unsigned long long)tableSize);
        long long step = auxHash(key, tableSize);
        for (long long i = 0; i < tableSize; i++) {
            long long idx = probe(h, step, i);
            if (table[idx].state == SlotState::EMPTY) return false;
            if (table[idx].state == SlotState::OCCUPIED && table[idx].key == key) {
                table[idx].state = SlotState::DELETED;
                count--;
                deletesSinceCompaction++;
                maybeCompact();
                return true;
            }
        }
        return false;
    }

    long long collisionCount() const { return collisions; }
    long long size() const { return tableSize; }
    long long elementCount() const { return count; }

private:
    HashFunc hf;
    Mode mode;
    long long C1, C2;
    vector<Slot> table;
    long long tableSize, count;
    long long insertsSinceExpansion, deletesSinceCompaction, collisions;

    long long probe(long long h, long long step, long long i) const {
        long long idx;
        if (mode == Mode::DOUBLE_HASHING) {
            idx = h + i * step;                              // doubleHash(k,i)
        } else {
            idx = h + C1 * i * step + C2 * i * i;             // customHash(k,i)
        }
        idx %= tableSize;
        if (idx < 0) idx += tableSize;
        return idx;
    }

    // insert without touching statistics/counters — used only during rehash
    void insertRaw(const string& key, int value) {
        long long h = (long long)(hf(key) % (unsigned long long)tableSize);
        long long step = auxHash(key, tableSize);
        for (long long i = 0; i < tableSize; i++) {
            long long idx = probe(h, step, i);
            if (table[idx].state != SlotState::OCCUPIED) {
                table[idx] = {SlotState::OCCUPIED, key, value};
                return;
            }
        }
    }

    void maybeExpand() {
        double lf = (double)count / (double)tableSize;
        if (lf > Config::LOAD_HIGH && insertsSinceExpansion >= count / 2) {
            long long newSize = smallestPrimeAbove(2.0 * tableSize);
            rehash(newSize);
            insertsSinceExpansion = 0;
        }
    }
    void maybeCompact() {
        if (tableSize == Config::INITIAL_SIZE) return;
        double lf = (double)count / (double)tableSize;
        if (lf < Config::LOAD_LOW && deletesSinceCompaction >= count / 2) {
            long long newSize = largestPrimeBelow(tableSize / 2.0);
            if (newSize < Config::INITIAL_SIZE) newSize = Config::INITIAL_SIZE;
            rehash(newSize);
            deletesSinceCompaction = 0;
        }
    }
    void rehash(long long newSize) {
        vector<Slot> old = move(table);
        tableSize = newSize;
        table.assign(newSize, Slot());
        for (auto& s : old)
            if (s.state == SlotState::OCCUPIED)
                insertRaw(s.key, s.value);
    }
};

// =====================================================================
// Random unique word generator (Section 1.3)
// =====================================================================
class WordGenerator {
public:
    explicit WordGenerator(int length, unsigned seed = random_device{}())
        : len(length), rng(seed), dist(0, 25) {}

    // generate `count` unique words of fixed length `len`
    vector<string> generateUnique(int count) {
        vector<string> words;
        words.reserve(count);
        unordered_set<string> seen;
        seen.reserve(count * 2);
        while ((int)words.size() < count) {
            string w = oneWord();
            if (seen.insert(w).second) words.push_back(w);
        }
        return words;
    }

private:
    int len;
    mt19937 rng;
    uniform_int_distribution<int> dist;

    string oneWord() {
        string w(len, 'a');
        for (int i = 0; i < len; i++) w[i] = char('a' + dist(rng));
        return w;
    }
};

// =====================================================================
// Report generation (Section 5)
// =====================================================================
struct TechniqueResult {
    long long collisions = 0;
    double avgHits = 0.0;
};

void printReportTable(const TechniqueResult r[3][2]) {
    // r[technique][hashFuncIndex]  technique: 0=Chaining,1=DoubleHashing,2=CustomProbing
    const char* names[3] = {"Chaining Method", "Double Hashing", "Custom Probing"};
    cout << "\n";
    cout << left << setw(20) << "Technique"
         << right << setw(18) << "Hash1 Collisions" << setw(14) << "Hash1 AvgHits"
         << setw(20) << "Hash2 Collisions" << setw(14) << "Hash2 AvgHits" << "\n";
    cout << string(86, '-') << "\n";
    for (int t = 0; t < 3; t++) {
        cout << left << setw(20) << names[t]
             << right << setw(18) << r[t][0].collisions
             << setw(14) << fixed << setprecision(3) << r[t][0].avgHits
             << setw(20) << r[t][1].collisions
             << setw(14) << fixed << setprecision(3) << r[t][1].avgHits
             << "\n";
    }
    cout << "\n";
}

int main() {
    ios_base::sync_with_stdio(false);

    cout << "=== CSE208 Hash Table Assignment ===\n\n";

    // ---------------------------------------------------------------
    // Small correctness demo matching the spec's example (Section 2)
    // ---------------------------------------------------------------
    {
        cout << "--- Demo (Section 2 example) ---\n";
        vector<string> demoWords = {"ancient", "puzzled", "benefit", "ancient", "zigzags"};
        ChainingHashTable demo(hash1);
        int seq = 1;
        for (auto& w : demoWords) {
            if (demo.insert(w, seq)) {
                cout << "(" << w << ", " << seq << ")\n";
                seq++;
            } else {
                cout << w << " -> duplicate, discarded\n";
            }
        }
        cout << "\n";
    }

    // ---------------------------------------------------------------
    // Full evaluation: 10,000 unique words of length 10
    // ---------------------------------------------------------------
    const int N_WORDS = 10000;
    const int WORD_LEN = 10;
    const int N_SEARCH = 1000;

    cout << "Generating " << N_WORDS << " unique random words of length "
         << WORD_LEN << " ...\n";
    WordGenerator gen(WORD_LEN, 42);
    vector<string> words = gen.generateUnique(N_WORDS);

    // sanity check: hash-value uniqueness requirement (>=60% unique hash values)
    {
        unordered_set<unsigned long long> uniq1, uniq2;
        for (auto& w : words) { uniq1.insert(hash1(w)); uniq2.insert(hash2(w)); }
        cout << "Hash1 unique hash values: " << uniq1.size() << " / " << N_WORDS
             << " (" << fixed << setprecision(1) << 100.0 * uniq1.size() / N_WORDS << "%)\n";
        cout << "Hash2 unique hash values: " << uniq2.size() << " / " << N_WORDS
             << " (" << fixed << setprecision(1) << 100.0 * uniq2.size() / N_WORDS << "%)\n\n";
    }

    // pick 1000 random words (from the generated set) to search for
    mt19937 rng(123);
    vector<string> searchWords;
    {
        vector<int> idxs(N_WORDS);
        iota(idxs.begin(), idxs.end(), 0);
        shuffle(idxs.begin(), idxs.end(), rng);
        for (int i = 0; i < N_SEARCH; i++) searchWords.push_back(words[idxs[i]]);
    }

    TechniqueResult results[3][2]; // [technique][hashFuncIndex]

    HashFunc funcs[2] = {hash1, hash2};
    const char* funcNames[2] = {"Hash1 (djb2)", "Hash2 (FNV-1a)"};

    for (int hIdx = 0; hIdx < 2; hIdx++) {
        cout << "--- Evaluating with " << funcNames[hIdx] << " ---\n";

        // Chaining
        {
            ChainingHashTable table(funcs[hIdx]);
            int seq = 1;
            for (auto& w : words) if (table.insert(w, seq)) seq++;
            long long totalHits = 0;
            for (auto& w : searchWords) totalHits += table.search(w);
            results[0][hIdx].collisions = table.collisionCount();
            results[0][hIdx].avgHits = (double)totalHits / N_SEARCH;
            cout << "  Chaining      -> collisions: " << table.collisionCount()
                 << ", avg hits: " << results[0][hIdx].avgHits
                 << ", final table size: " << table.size() << "\n";
        }
        // Double Hashing
        {
            OpenAddressingHashTable table(funcs[hIdx], OpenAddressingHashTable::Mode::DOUBLE_HASHING);
            int seq = 1;
            for (auto& w : words) if (table.insert(w, seq)) seq++;
            long long totalHits = 0;
            for (auto& w : searchWords) totalHits += table.search(w);
            results[1][hIdx].collisions = table.collisionCount();
            results[1][hIdx].avgHits = (double)totalHits / N_SEARCH;
            cout << "  Double Hash   -> collisions: " << table.collisionCount()
                 << ", avg hits: " << results[1][hIdx].avgHits
                 << ", final table size: " << table.size() << "\n";
        }
        // Custom Probing
        {
            OpenAddressingHashTable table(funcs[hIdx], OpenAddressingHashTable::Mode::CUSTOM_PROBING, 1, 3);
            int seq = 1;
            for (auto& w : words) if (table.insert(w, seq)) seq++;
            long long totalHits = 0;
            for (auto& w : searchWords) totalHits += table.search(w);
            results[2][hIdx].collisions = table.collisionCount();
            results[2][hIdx].avgHits = (double)totalHits / N_SEARCH;
            cout << "  Custom Probe  -> collisions: " << table.collisionCount()
                 << ", avg hits: " << results[2][hIdx].avgHits
                 << ", final table size: " << table.size() << "\n";
        }
        cout << "\n";
    }

    cout << "=== Final Report (Section 5) ===\n";
    printReportTable(results);

    return 0;
}