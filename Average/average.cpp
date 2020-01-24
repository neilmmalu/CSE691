#include <iostream>
#include <thread>
#include <vector>

using namespace std;

void average(vector<vector<int>> &A, vector<vector<int>> &B, int size, int r1, int r2, int c1, int c2){

}

int main(){
    cout << "Enter the size of the matrix: ";
    int size;
    cin >> size;

    cout << endl;

    vector<vector<int>> A {size, vector<int>(size)};
    vector<vector<int>> B {size, vector<int>(size)};

    for (auto &i : A) {
        for (auto &j : i) { j = rand() % 5; }
    }

    

    return 0;
}