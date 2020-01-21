#include <iostream>
#include <thread>

using namespace std;

void multiply(int A[][80], int B[][150], int C[][150], int r1, int r2, int c1, int c2, int k){
    for(int i = r1; i <= r2; i++){
        for(int j = c1; j <= c2; j++){
            int res = 0;
            for(int w = 0; w < k; w++) res += A[i][w] * B[w][j];
            C[i][j] = res;
        }
    }
}

int main(){
    int A[100][80], B[80][150], C[100][150];
    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 80; j++){
            A[i][j] = 1;
        }
    }

    for(int i = 0; i < 80; i++){
        for(int j = 0; j < 150; j++){
            B[i][j] = 1;
        }
    }

    thread t1 {multiply, A, B, C, 0, 49, 0, 74, 80};
    thread t2 {multiply, A, B, C, 0, 49, 75, 149, 80};
    thread t3 {multiply, A, B, C, 50, 99, 0, 74, 80};
    thread t4 {multiply, A, B, C, 50, 99, 75, 149, 80};

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    for (int i = 0; i < 100; i++){
        cout << endl;
        for (int j = 0; j < 150; j++) {
            cout << C[i][j] << " ";
        }
    }

    return 0;
}