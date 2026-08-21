#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include<chrono>

using namespace std;

long long add_count = 0, add_total = 0;
long long remove_count = 0, remove_total = 0;
long long update_count = 0, update_total = 0;
long long conflict_count = 0, conflict_total = 0;
long long overlaps_count = 0, overlaps_total = 0;
long long at_count = 0, at_total = 0;
long long next_count = 0, next_total = 0;

void print_timing(string operation, long long count, long long total) {
    cout << operation << " , "<< count << " , "<< total << " , ";

    if (count == 0) cout << "N/A";
    else cout << (double)total / count;
    cout << endl;
}

struct node {
    int id;
    int left;
    int right;
    int height;
    int maxend;

    node(int i, int e) {
        id = i;
        left = -1;
        right = -1;
        height = 1;
        maxend = e;
    }
};

vector<node> tree;
int root = -1;

int nextId = 1;

unordered_map<int, pair<int, int>> events;

int get_height(int n) {
    if (n == -1) return 0;
    else return tree[n].height;
}
int get_maxend(int n) {
    if (n == -1) return -1;
    else return tree[n].maxend;
}
int get_bf(int n) {
    if (n == -1) {
        return 0;
    }
    return get_height(tree[n].left) - get_height(tree[n].right);
}

void update_node(int n){
    tree[n].height=1+max(get_height(tree[n].left),get_height(tree[n].right));
    int id = tree[n].id;
    int end = events[id].second;
    tree[n].maxend = max(end, max(get_maxend(tree[n].left), get_maxend(tree[n].right)));
}

bool less_key(int i1,int i2){
    int start1 = events[i1].first;
    int start2 = events[i2].first;
    if(start1!=start2) return start1<start2;
    return i1<i2;
}

int right_rotate(int x){
    int left= tree[x].left;
    int right= tree[left].right;
    tree[left].right = x;
    tree[x].left = right;
    update_node(x);
    update_node(left);
    return left;
}

int left_rotate(int x){
    int right= tree[x].right;
    int left= tree[right].left;
    tree[right].left = x;
    tree[x].right = left;
    update_node(x);
    update_node(right);
    return right;
}

int insert_node(int n,int i){
    if (n==-1) {
        int end = events[i].second;
        tree.push_back(node(i, end));
        return tree.size() - 1;
    }
    if (less_key(i,tree[n].id)){

        tree[n].left = insert_node(tree[n].left, i);
    }
    else{
        tree[n].right =insert_node(tree[n].right, i);
    }
    update_node(n);
    int bf = get_bf(n); 
    if(bf>1 && less_key(i,tree[tree[n].left].id)){
        return right_rotate(n);
    }
    if(bf<-1 && !less_key(i,tree[tree[n].right].id)){
        return left_rotate(n);
    }
    if (bf>1 && !less_key(i,tree[tree[n].left].id)) {

        tree[n].left =left_rotate(tree[n].left);

        return right_rotate(n);
    }
    if (bf<-1 && less_key(i,tree[tree[n].right].id)) {

        tree[n].right = right_rotate(tree[n].right);

        return left_rotate(n);
    }

    return n;
}

void insert_event(int i){
    root = insert_node(root, i);
}

int find_id(int n,int i){
    if (n == -1) return false;

    if (tree[n].id == i) return true;

    if (find_id(tree[n].left, i)) return true;

    return find_id(tree[n].right, i);
}

int erase_node(int n, int id){
    if(n==-1) return n;
    if(id==tree[n].id){
        if(tree[n].left==-1){
            return tree[n].right;
        }
        if(tree[n].right==-1){
            return tree[n].left;
        }
        int succ = tree[n].right;
            while(tree[succ].left!=-1){
                succ = tree[succ].left;
            }
        int succ_id = tree[succ].id;
        tree[n].id = succ_id;
        tree[n].right = erase_node(tree[n].right, succ_id);
    }
    else if(less_key(id,tree[n].id)){
        tree[n].left = erase_node(tree[n].left, id);
    }
    else{
        tree[n].right = erase_node(tree[n].right, id);
    }
    update_node(n);
    int bf = get_bf(n);
    if(bf>1 && get_bf(tree[n].left)>=0){
        return right_rotate(n);
    }
    if (bf > 1 && get_bf(tree[n].left) < 0){
        tree[n].left = left_rotate(tree[n].left);
        return right_rotate(n);
    }
    if (bf < -1 && get_bf(tree[n].right) <= 0) {
    return left_rotate(n);
}
    
    if (bf<-1 && get_bf(tree[n].right)>0) {

        tree[n].right =right_rotate(tree[n].right);

        return left_rotate(n);
    }
    return n;
}


void erase_event(int i){
    root = erase_node(root, i);
}
void print(int n,ostream &out) {
    if(n==-1) return;
    out<<tree[n].id;
    if(tree[n].left==-1 && tree[n].right==-1) return;
    out<<"(";
    print(tree[n].left,out);
    out<<",";
    print(tree[n].right,out);
    out<<")";
}

bool does_overlap(int s1,int e1,int s2,int e2){
    return s1<e2 && s2<e1;
}

int next_search(int n,int t){
    int answer = -1;
    while (n != -1){
        int id = tree[n].id;
        int start = events[id].first;

        if (start >= t){
            answer = id;
            n = tree[n].left;
        }
        else{
            n = tree[n].right;
        }
    }
    return answer;
}

void at_search(int n,int t,vector<int> &result){
    if (n == -1) return;

    int id = tree[n].id;

    int start = events[id].first;
    int end = events[id].second;

    if (tree[n].left != -1 && tree[tree[n].left].maxend > t){
        at_search(tree[n].left,t,result);
    }

    if (start <= t && t < end) {
        result.push_back(id);
    }
    if (start <=t){
        at_search(tree[n].right,t,result);
    }
}

void overlap_search(int n,int s,int e,vector<int> & result){
    if(n==-1) return;
    int id = tree[n].id;
    int e_start = events[id].first;
    int e_end = events[id].second;
    if (tree[n].left != -1 && tree[tree[n].left].maxend > s){
        overlap_search(tree[n].left,s,e,result);
    }
    if (does_overlap(e_start,e_end,s,e)){
        result.push_back(id);
    }
    if (e_start < e){
        overlap_search(tree[n].right,s,e,result);
    }
}

bool conflict_search(int n,int s,int e){
    if(n==-1) return false;
    int id = tree[n].id;

    int e_start = events[id].first;
    int e_end = events[id].second;
    if (does_overlap(e_start,e_end,s,e)){
        return true;
    }

    if (tree[n].left != -1 &&tree[tree[n].left].maxend > s){
        if (conflict_search(tree[n].left,s,e))return true;
    }
    if (e_start >= e) return false;
    return conflict_search(tree[n].right, s, e);
}

int main(int argc,char*argv[]){
    if(argc!=3){
        cout<<"please provide input.txt and output.txt"<<endl;
    }
     ifstream input(argv[1]);
    ofstream output(argv[2]);
    string cmd;
    while(input>>cmd){
        if(cmd=="ADD"){
            int s, e;
            input>>s>>e;
            auto start = chrono::high_resolution_clock::now();
            int id = nextId++;

            events[id] = {s, e};
            insert_event(id);
            print(root, output);

            output<<endl;
            auto finish = chrono::high_resolution_clock::now();
            add_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            add_count++;
        }

        else if(cmd=="REMOVE"){
            int id;
            auto start = chrono::high_resolution_clock::now();
            input>>id;


            if (!events.count(id)) {

                output << "not found"<<endl;
            }
            else {

                erase_event(id);

                events.erase(id);

                print(root, output);

                output<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            remove_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            remove_count++;
        }
        else if(cmd=="UPDATE"){
            int id, s, e;

            input>>id>>s>>e;
            auto start = chrono::high_resolution_clock::now();
            if (!events.count(id)) {
                output<<"not found"<<endl;
            }
            else {
                erase_event(id);
                events[id] = {s, e};

                insert_event(id);

                print(root, output);

                output<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            update_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            update_count++;
        }
        else if(cmd=="CONFLICT"){
            int s, e;
            auto start = chrono::high_resolution_clock::now();
            input>>s>>e;

            if (conflict_search(root, s, e)) output << "yes"<<endl;
            else output<<"no"<<endl;
            auto finish = chrono::high_resolution_clock::now();
            conflict_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            conflict_count++;
        }
        else if(cmd=="OVERLAPS"){
            int s,e;

            input>>s>>e;
            auto start = chrono::high_resolution_clock::now();

            vector<int> result;
            overlap_search(root,s,e,result);


            if (result.empty()) {
                output << "none"<<endl;
            }
            else {

                for (int i = 0;i < (int)result.size();i++) {
                    if (i > 0) output << " ";

                    output << result[i];
                }

                output<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            overlaps_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            overlaps_count++;
        }
        else if(cmd=="AT"){
            int t;

            input>>t;

            auto start = chrono::high_resolution_clock::now();
            vector<int> result;

            at_search(root,t,result);


            if (result.empty()) {
                output<<"none"<<endl;
            }
            else {
                for (int i = 0;i < (int)result.size();i++) {
                    if (i > 0) output << " ";
                    output << result[i];
                }

                output<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            at_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            at_count++;
        }
        else if(cmd=="NEXT"){
            int t;
            input >> t;
            auto start = chrono::high_resolution_clock::now();
            int id = next_search(root, t);


            if (id == -1){

                output << "none"<<endl;
            }
            else {
                output << id << " "<< events[id].first << " "<< events[id].second<< endl;
            }
            auto finish =chrono::high_resolution_clock::now();
            next_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            next_count++;
        }
    }
    print_timing("add",add_count,add_total);
    print_timing("remove",remove_count,remove_total);
    print_timing("update",update_count,update_total);
    print_timing("conflict",conflict_count,conflict_total);
    print_timing("overlaps",overlaps_count,overlaps_total);
    print_timing("at",at_count,at_total);
    print_timing("next",next_count,next_total);
}