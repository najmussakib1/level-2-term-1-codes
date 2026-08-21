#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include <chrono>
using namespace std;

long long insert_count = 0, insert_total = 0;
long long erase_count = 0, erase_total = 0;
long long find_count = 0, find_total = 0;
long long traverse_count = 0, traverse_total = 0;
void print_timing(string operation, long long count, long long total) {
    cout << operation << " , "<< count << " , "<< total << " , ";

    if (count == 0) cout << "N/A";
    else cout << (double)total / count;
    cout << endl;
}

struct node{
    int val;
    int left;
    int right;
    int height;
    node(int k){
        val=k;
        left=-1;
        right =-1;
        height = 1;
    }
};
vector<node> tree;
int root =-1;
int get_height(int n){
    if(n==-1) return 0;
    else return tree[n].height;
}
int get_bf(int n){
    if(n==-1){
        return 0;
    }
    return get_height(tree[n].left)-get_height(tree[n].right);
}
void update_height(int n){
    tree[n].height =1+max(get_height(tree[n].left),get_height(tree[n].right));
}
int right_rotate(int x){
    int left= tree[x].left;
    int right= tree[left].right;
    tree[left].right = x;
    tree[x].left = right;
    update_height(x);
    update_height(left);
    return left;
}

int left_rotate(int x){
    int right= tree[x].right;
    int left= tree[right].left;
    tree[right].left = x;
    tree[x].right = left;
    update_height(x);
    update_height(right);
    return right;
}

int insert_node(int n,int v,bool& inserted){
    if (n==-1) {
        tree.push_back(node(v));
        inserted = true;
        return tree.size() - 1;
    }
    if (v<tree[n].val) {
        tree[n].left =insert_node(tree[n].left, v, inserted);
    }
    else if (v > tree[n].val) {
        tree[n].right =insert_node(tree[n].right, v, inserted);
    }
    else {
        inserted = false;
        return n;
    }
    update_height(n);
    int bf = get_bf(n);
    if(bf>1 && v<tree[tree[n].left].val){
        return right_rotate(n);
    }
    if(bf<-1 && v>tree[tree[n].right].val){
        return left_rotate(n);
    }
    if (bf>1 && v>tree[tree[n].left].val) {

        tree[n].left =left_rotate(tree[n].left);

        return right_rotate(n);
    }
    if (bf<-1 && v <tree[tree[n].right].val) {

        tree[n].right =
            right_rotate(tree[n].right);

        return left_rotate(n);
    }

    return n;
}
bool insert(int key) {
    bool inserted = false;
    root = insert_node(root, key, inserted);
    return inserted;
}

int erase_node(int n, int v, bool & erased){
    if(n==-1) return n;
    if (v < tree[n].val){
tree[n].left = erase_node(tree[n].left, v, erased);
    }

    else if(v>tree[n].val){
        tree[n].right=erase_node(tree[n].right, v, erased);
    }
    else{
        erased = true;
        if(tree[n].left==-1) return tree[n].right;
        else if(tree[n].right==-1) return tree[n].left;
        else{
            int succ = tree[n].right;
            while(tree[succ].left!=-1){
                succ = tree[succ].left;
            }
            tree[n].val = tree[succ].val;
            tree[n].right = erase_node(tree[n].right, tree[succ].val, erased);
        }
    }
    update_height(n);
    int bf = get_bf(n);
    if(bf>1 && get_bf(tree[n].left)>=0){
        return right_rotate(n);
    }
    if(bf<-1 && get_bf(tree[n].left)<0){
        return left_rotate(n);
    }
    if (bf>1 && get_bf(tree[n].right)<=0) {

        tree[n].left =left_rotate(tree[n].left);

        return right_rotate(n);
    }
    if (bf<-1 && get_bf(tree[n].right)>0) {

        tree[n].right =right_rotate(tree[n].right);

        return left_rotate(n);
    }
    return n;
}
bool erase(int v){
    bool erased = false;
    root = erase_node(root, v, erased);
    return erased;
}

void print_inorder(int n, bool &first,ostream &out) {
    if (n == -1)return;
    print_inorder(tree[n].left, first, out);

    if (!first) out << " ";

    out << tree[n].val;
    first = false;

    print_inorder(tree[n].right, first, out);
}

void print(int n,ostream &out) {
    if(n==-1) return;
    out<<tree[n].val;
    if(tree[n].left==-1 && tree[n].right==-1) return;
    out<<"(";
    print(tree[n].left,out);
    out<<",";
    print(tree[n].right,out);
    out<<")";
}

bool find_node(int n, int v) {

    if (n==-1) return false;

    if (v==tree[n].val) return true;

    if (v < tree[n].val) return find_node(tree[n].left, v);

    return find_node(tree[n].right, v);
}


bool find(int key) {
    return find_node(root, key);
}

int main(int argc,char *argv[]){
    if(argc!=3){
        cout<<"give input.txt and output,txt"<<endl;
        return 0;
    }
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    char op;
    int x;
    while(input>>op){
        if(op=='I'){
            input>>x;
            auto start = chrono::high_resolution_clock::now();
            if(insert(x)){
                print(root,output);
                output<<endl;
            }
            else{
                output<<"duplicate"<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();

            insert_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            insert_count++;
        }
        else if(op=='D'){
            input>>x;
            auto start = chrono::high_resolution_clock::now();
            if(erase(x)){
                print(root,output);
                output<<endl;
            }
            else{
                output<<"not found"<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            erase_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            erase_count++;
        }
        else if(op=='T'){
            traverse_count++;
            auto start = chrono::high_resolution_clock::now();
            bool first = true;

            print_inorder(root,first,output);
            output<<endl;
            auto finish = chrono::high_resolution_clock::now();
            traverse_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
        }
        else if(op=='F'){

            input>>x;
            auto start = chrono::high_resolution_clock::now();
            if(find(x)){
                output<<"found"<<endl;
            }
            else{
                output<<"not found"<<endl;
            }
            auto finish = chrono::high_resolution_clock::now();
            find_total += chrono::duration_cast<chrono::nanoseconds> (finish - start).count();
            find_count++;
        }
    }
    print_timing("insert", insert_count, insert_total);
    print_timing("delete", erase_count, erase_total);
    print_timing("find", find_count, find_total);
    print_timing("traverse", traverse_count, traverse_total);
}