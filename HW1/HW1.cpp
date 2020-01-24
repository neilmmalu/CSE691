//CIS600/CSE691  HW1
//Due: 11:59PM, Friday(1/31)

/*
Implement the two member functions: merge_sort and merge, as defined below for a sequential merge sort.
Note that the merge will be called by merge_sort.

In implementing both functions, you are only allowed to modify "next" and "previous" of nodes, but not "values" of nodes. 
You are not allowed to use any external structures such as array, linked list, etc.
You are not allowed to create any new node.
You are not allowed to create any new function.


After completing the above sequential version,  create a parallel version, by using two additional threads to speed up the merge sort.
You have to use the two functions you have implemented above.  You are not allowed to create new functions. Extra work will be needed in main function.

In your threaded implementation, you are allowed to introduce an extra node and a global pointer to the node.

It is alright if your implementation does not require the extra node or global pointer to node.

*/

#include <iostream>
#include <thread>

using namespace std;


class node {
public:
    int value;
    node * next;
    node * previous;
    node(int i) { value = i; next = previous = nullptr; }
    node() { next = previous = nullptr; }
};

class doubly_linked_list {
public:
    int num_nodes;
    node * head;
    node * tail;
    doubly_linked_list() { num_nodes = 0; head = tail = nullptr; }
    void make_random_list(int m, int n);
    void print_forward();
    void print_backward();


    //Recursively merge sort i numbers starting at node pointed by p
    void merge_sort(node * p, int i);//in-place recursive merge sort


    //Merge i1 numbers starting at node pointed by p1 with i2 numbers
    //starting at node pointed by p2
    void merge(node * p1, int i1, node * p2, int i2);

    

};

void doubly_linked_list::make_random_list(int m, int n) {

    for (int i = 0; i < m; i++) {
        node * p1 = new node(rand() % n);
        p1->previous = tail;
        if (tail != nullptr) tail->next = p1;
        tail = p1;
        if (head == nullptr) head = p1;
        num_nodes++;
    }
}

void doubly_linked_list::print_forward() {
    cout << endl;
    node * p1 = head;
    while (p1 != nullptr) {
        cout << p1->value << " ";
        p1 = p1->next;
    }
}

void doubly_linked_list::print_backward() {
    cout << endl;
    node * p1 = tail;
    while (p1 != nullptr) {
        cout << p1->value << " ";
        p1 = p1->previous;
    }
}

void merge(node * p1, int i1, node * p2, int i2){
    if(!p1 || i1 == 0) return;

    if(!p2 || i2 == 0) return;

    if(p1->value < p2->value){
        
    }
}

void merge_sort(node * p, int n){
    if(n == 0 || n == 1 || !p || !p->next) return p;

    node* p1 = p;
    for(int i = 1; i <= n/2; i++){
        p1 = p1->next;
    }
    //p points to first list
    //p1 points to second list
    p1->previous->next = nullptr;
    p1->previous = nullptr;
    //List is now split
    merge_sort(p, n/2);
    merge_sort(p1, n/2);

    merge(p, n/2, p1, n/2);
}



int main() {
    /*
    Implement the merge_sort and merge_functions defined above to complete a sequential version of 
    merge sort.
    */

    doubly_linked_list d1, d2;
    d1.make_random_list(30, 20);
    d1.print_forward();
    d1.print_backward();

    d1.merge_sort(d1.head, d1.num_nodes);
    d1.print_forward();
    d1.print_backward();


    d2.make_random_list(50, 40);
    d2.print_forward();
    d2.print_backward();

    /*
    Create two additional threads to speed up the merge sort.
    You have to still use the same merge_sort and merge functions implemented above.
    You will need to do some extra work within main funciton.
    */
    d2.print_forward();
    d2.print_backward();
    return 0;

}