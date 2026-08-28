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
// Compile:  g++ -O2 -std=c++17 hashtable_v2.cpp -o hashtable
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
struct config {
    static constexpr long long init_size = 13;    // initial table size (must be prime)
    static constexpr double    load_high = 0.5;   // expand if load factor exceeds this
    static constexpr double    load_low  = 0.25;  // shrink if load factor falls below this
};

// =====================================================================
// Prime helpers
// =====================================================================
bool is_prime(long long n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

// smallest prime strictly bigger than num
long long next_prime(double num) {
    long long p = (long long)floor(num) + 1;
    while (!is_prime(p)) p++;
    return p;
}

// biggest prime strictly smaller than num
long long prev_prime(double num) {
    long long p = (long long)ceil(num) - 1;
    while (p > 1 && !is_prime(p)) p--;
    if (p < 2) p = 2;
    return p;
}

// =====================================================================
// Hash functions (Section 3)
// =====================================================================

// Hash1: djb2 (Dan Bernstein) - widely used string hash
unsigned long long hash_one(const string& key) {
    unsigned long long h = 5381;
    for (unsigned char c : key) h = ((h << 5) + h) + c;   // h*33 + c
    return h;
}

// Hash2: FNV-1a - well established, good avalanche behaviour
unsigned long long hash_two(const string& key) {
    unsigned long long h = 14695981039346656037ULL;
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

using hash_func = function<unsigned long long(const string&)>;

// A simple, independent secondary hash used to build the probing step
// (auxHash(k) in Section 4.2 / 4.3). Kept intentionally simple.
unsigned long long mix_hash(const string& key) {
    unsigned long long s = 0;
    for (unsigned char c : key) s = s * 31ULL + c;
    return s;
}

// aux_hash(k): returns a step size in [1, r] built from a prime r < table_size
long long aux_hash(const string& key, long long table_size) {
    long long r = (table_size > 3) ? prev_prime(table_size) : 1;
    if (r < 1) r = 1;
    long long step = 1 + (long long)(mix_hash(key) % (unsigned long long)r);
    return step; // in [1, r] (never 0, so probing always moves forward)
}

// =====================================================================
// Technique 1: Chaining (Section 4.1)
// =====================================================================
class chain_table {
public:
    explicit chain_table(hash_func f) : hf(move(f)) {
        table_size = config::init_size;
        arr.assign(table_size, {});
        cnt = 0;
        ins_after_grow = 0;
        del_after_shrink = 0;
        coll = 0;
    }

    // returns false if key already exists (duplicate discarded)
    bool insert(const string& key, int val) {
        long long idx = (long long)(hf(key) % (unsigned long long)table_size);
        for (auto& kv : arr[idx])
            if (kv.first == key) return false; // duplicate

        if (!arr[idx].empty()) coll++;  // bucket already had someone else in it

        arr[idx].push_back({key, val});
        cnt++;
        ins_after_grow++;
        grow_if_needed();
        return true;
    }

    bool remove(const string& key) {
        long long idx = (long long)(hf(key) % (unsigned long long)table_size);
        auto& bucket = arr[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                cnt--;
                del_after_shrink++;
                shrink_if_needed();
                return true;
            }
        }
        return false;
    }

    // returns number of accesses to find key (>=1), or -1 if not found
    long long search(const string& key) const {
        long long idx = (long long)(hf(key) % (unsigned long long)table_size);
        long long hit = 0;
        for (auto& kv : arr[idx]) {
            hit++;
            if (kv.first == key) return hit;
        }
        return -1;
    }

    long long get_coll() const { return coll; }
    long long get_size() const { return table_size; }
    long long get_cnt() const { return cnt; }

private:
    hash_func hf;
    vector<list<pair<string,int>>> arr;
    long long table_size, cnt;
    long long ins_after_grow, del_after_shrink, coll;

    void grow_if_needed() {
        if (table_size == 0) return;
        double load = (double)cnt / (double)table_size;
        // grow only after at least n/2 inserts since last grow
        if (load > config::load_high && ins_after_grow >= cnt / 2) {
            long long new_size = next_prime(2.0 * table_size);
            rebuild(new_size);
            ins_after_grow = 0;
        }
    }
    void shrink_if_needed() {
        if (table_size == config::init_size) return; // never shrink below/at start size
        double load = (double)cnt / (double)table_size;
        if (load < config::load_low && del_after_shrink >= cnt / 2) {
            long long new_size = prev_prime(table_size / 2.0);
            if (new_size < config::init_size) new_size = config::init_size;
            rebuild(new_size);
            del_after_shrink = 0;
        }
    }
    void rebuild(long long new_size) {
        vector<list<pair<string,int>>> new_arr(new_size);
        for (auto& bucket : arr)
            for (auto& kv : bucket)
                new_arr[(long long)(hf(kv.first) % (unsigned long long)new_size)].push_back(kv);
        arr = move(new_arr);
        table_size = new_size;
    }
};

// =====================================================================
// Technique 2 & 3: Open addressing (Double Hashing / Custom Probing)
// Sections 4.2 and 4.3
// =====================================================================
enum class slot_state { empty_slot, used_slot, deleted_slot };

struct slot {
    slot_state state = slot_state::empty_slot;
    string key;
    int val = 0;
};

class open_table {
public:
    enum class probe_mode { double_hash, custom_probe };

    open_table(hash_func f, probe_mode m, long long c1 = 1, long long c2 = 3)
        : hf(move(f)), mode(m), c1(c1), c2(c2) {
        table_size = config::init_size;
        arr.assign(table_size, slot());
        cnt = 0;
        ins_after_grow = 0;
        del_after_shrink = 0;
        coll = 0;
    }

    bool insert(const string& key, int val) {
        long long start = (long long)(hf(key) % (unsigned long long)table_size);
        long long step = aux_hash(key, table_size);

        bool bumped = false;
        long long put_here = -1;

        for (long long i = 0; i < table_size; i++) {
            long long idx = probe(start, step, i);
            if (arr[idx].state == slot_state::used_slot) {
                if (arr[idx].key == key) return false; // duplicate
                if (i == 0) bumped = true;              // first-choice slot was taken
            } else {
                if (put_here == -1) put_here = idx;
                if (arr[idx].state == slot_state::empty_slot) break;
            }
        }
        if (put_here == -1) return false; // table full (shouldn't happen, we grow ahead of time)

        arr[put_here] = {slot_state::used_slot, key, val};
        if (bumped) coll++;
        cnt++;
        ins_after_grow++;
        grow_if_needed();
        return true;
    }

    long long search(const string& key) const {
        long long start = (long long)(hf(key) % (unsigned long long)table_size);
        long long step = aux_hash(key, table_size);
        long long hit = 0;
        for (long long i = 0; i < table_size; i++) {
            long long idx = probe(start, step, i);
            hit++;
            if (arr[idx].state == slot_state::empty_slot) return -1;
            if (arr[idx].state == slot_state::used_slot && arr[idx].key == key) return hit;
        }
        return -1;
    }

    bool remove(const string& key) {
        long long start = (long long)(hf(key) % (unsigned long long)table_size);
        long long step = aux_hash(key, table_size);
        for (long long i = 0; i < table_size; i++) {
            long long idx = probe(start, step, i);
            if (arr[idx].state == slot_state::empty_slot) return false;
            if (arr[idx].state == slot_state::used_slot && arr[idx].key == key) {
                arr[idx].state = slot_state::deleted_slot;
                cnt--;
                del_after_shrink++;
                shrink_if_needed();
                return true;
            }
        }
        return false;
    }

    long long get_coll() const { return coll; }
    long long get_size() const { return table_size; }
    long long get_cnt() const { return cnt; }

private:
    hash_func hf;
    probe_mode mode;
    long long c1, c2;
    vector<slot> arr;
    long long table_size, cnt;
    long long ins_after_grow, del_after_shrink, coll;

    long long probe(long long start, long long step, long long i) const {
        long long idx;
        if (mode == probe_mode::double_hash) {
            idx = start + i * step;                        // doubleHash(k,i)
        } else {
            idx = start + c1 * i * step + c2 * i * i;       // customHash(k,i)
        }
        idx %= table_size;
        if (idx < 0) idx += table_size;
        return idx;
    }

    // put a key back in without touching stats/counters — used only while rebuilding
    void put_no_count(const string& key, int val) {
        long long start = (long long)(hf(key) % (unsigned long long)table_size);
        long long step = aux_hash(key, table_size);
        for (long long i = 0; i < table_size; i++) {
            long long idx = probe(start, step, i);
            if (arr[idx].state != slot_state::used_slot) {
                arr[idx] = {slot_state::used_slot, key, val};
                return;
            }
        }
    }

    void grow_if_needed() {
        double load = (double)cnt / (double)table_size;
        if (load > config::load_high && ins_after_grow >= cnt / 2) {
            long long new_size = next_prime(2.0 * table_size);
            rebuild(new_size);
            ins_after_grow = 0;
        }
    }
    void shrink_if_needed() {
        if (table_size == config::init_size) return;
        double load = (double)cnt / (double)table_size;
        if (load < config::load_low && del_after_shrink >= cnt / 2) {
            long long new_size = prev_prime(table_size / 2.0);
            if (new_size < config::init_size) new_size = config::init_size;
            rebuild(new_size);
            del_after_shrink = 0;
        }
    }
    void rebuild(long long new_size) {
        vector<slot> old_arr = move(arr);
        table_size = new_size;
        arr.assign(new_size, slot());
        for (auto& s : old_arr)
            if (s.state == slot_state::used_slot)
                put_no_count(s.key, s.val);
    }
};

// =====================================================================
// Random unique word generator (Section 1.3)
// =====================================================================
class word_gen {
public:
    explicit word_gen(int length, unsigned seed = random_device{}())
        : len(length), rng(seed), dist(0, 25) {}

    // makes `how_many` unique words, each of fixed length `len`
    vector<string> make_words(int how_many) {
        vector<string> words;
        words.reserve(how_many);
        unordered_set<string> seen;
        seen.reserve(how_many * 2);
        while ((int)words.size() < how_many) {
            string w = one_word();
            if (seen.insert(w).second) words.push_back(w);
        }
        return words;
    }

private:
    int len;
    mt19937 rng;
    uniform_int_distribution<int> dist;

    string one_word() {
        string w(len, 'a');
        for (int i = 0; i < len; i++) w[i] = char('a' + dist(rng));
        return w;
    }
};

// =====================================================================
// Report generation (Section 5)
// =====================================================================
struct result_row {
    long long coll = 0;
    double avg_hit = 0.0;
};

void print_table(const result_row res[3][2]) {
    // res[technique][hash_index]  technique: 0=Chaining, 1=DoubleHashing, 2=CustomProbing
    const char* names[3] = {"Chaining Method", "Double Hashing", "Custom Probing"};
    cout << "\n";
    cout << left << setw(20) << "Technique"
         << right << setw(18) << "Hash1 Collisions" << setw(14) << "Hash1 AvgHits"
         << setw(20) << "Hash2 Collisions" << setw(14) << "Hash2 AvgHits" << "\n";
    cout << string(86, '-') << "\n";
    for (int t = 0; t < 3; t++) {
        cout << left << setw(20) << names[t]
             << right << setw(18) << res[t][0].coll
             << setw(14) << fixed << setprecision(3) << res[t][0].avg_hit
             << setw(20) << res[t][1].coll
             << setw(14) << fixed << setprecision(3) << res[t][1].avg_hit
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
        vector<string> demo_words = {"ancient", "puzzled", "benefit", "ancient", "zigzags"};
        chain_table demo(hash_one);
        int seq = 1;
        for (auto& w : demo_words) {
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
    const int word_count = 10000;
    const int word_len = 10;
    const int search_count = 1000;

    cout << "Generating " << word_count << " unique random words of length "
         << word_len << " ...\n";
    word_gen gen(word_len, 42);
    vector<string> words = gen.make_words(word_count);

    // sanity check: hash-value uniqueness requirement (>=60% unique hash values)
    {
        unordered_set<unsigned long long> u1, u2;
        for (auto& w : words) { u1.insert(hash_one(w)); u2.insert(hash_two(w)); }
        cout << "Hash1 unique hash values: " << u1.size() << " / " << word_count
             << " (" << fixed << setprecision(1) << 100.0 * u1.size() / word_count << "%)\n";
        cout << "Hash2 unique hash values: " << u2.size() << " / " << word_count
             << " (" << fixed << setprecision(1) << 100.0 * u2.size() / word_count << "%)\n\n";
    }

    // pick 1000 random words (from the generated set) to search for
    mt19937 rng(123);
    vector<string> search_words;
    {
        vector<int> idx_list(word_count);
        iota(idx_list.begin(), idx_list.end(), 0);
        shuffle(idx_list.begin(), idx_list.end(), rng);
        for (int i = 0; i < search_count; i++) search_words.push_back(words[idx_list[i]]);
    }

    result_row res[3][2]; // [technique][hash_index]

    hash_func funcs[2] = {hash_one, hash_two};
    const char* func_names[2] = {"Hash1 (djb2)", "Hash2 (FNV-1a)"};

    for (int h = 0; h < 2; h++) {
        cout << "--- Evaluating with " << func_names[h] << " ---\n";

        // Chaining
        {
            chain_table t(funcs[h]);
            int seq = 1;
            for (auto& w : words) if (t.insert(w, seq)) seq++;
            long long total = 0;
            for (auto& w : search_words) total += t.search(w);
            res[0][h].coll = t.get_coll();
            res[0][h].avg_hit = (double)total / search_count;
            cout << "  Chaining      -> collisions: " << t.get_coll()
                 << ", avg hits: " << res[0][h].avg_hit
                 << ", final table size: " << t.get_size() << "\n";
        }
        // Double Hashing
        {
            open_table t(funcs[h], open_table::probe_mode::double_hash);
            int seq = 1;
            for (auto& w : words) if (t.insert(w, seq)) seq++;
            long long total = 0;
            for (auto& w : search_words) total += t.search(w);
            res[1][h].coll = t.get_coll();
            res[1][h].avg_hit = (double)total / search_count;
            cout << "  Double Hash   -> collisions: " << t.get_coll()
                 << ", avg hits: " << res[1][h].avg_hit
                 << ", final table size: " << t.get_size() << "\n";
        }
        // Custom Probing
        {
            open_table t(funcs[h], open_table::probe_mode::custom_probe, 1, 3);
            int seq = 1;
            for (auto& w : words) if (t.insert(w, seq)) seq++;
            long long total = 0;
            for (auto& w : search_words) total += t.search(w);
            res[2][h].coll = t.get_coll();
            res[2][h].avg_hit = (double)total / search_count;
            cout << "  Custom Probe  -> collisions: " << t.get_coll()
                 << ", avg hits: " << res[2][h].avg_hit
                 << ", final table size: " << t.get_size() << "\n";
        }
        cout << "\n";
    }

    cout << "=== Final Report (Section 5) ===\n";
    print_table(res);

    return 0;
}