//Threaded Programming
#include <iostream>
#include <thread>
using namespace std;

void multiply(int A[][10], int B[][5], int C[][5], int i1, int i2, int j1, int j2, int k) {
    for (int i = i1; i <= i2; i++) {
        for (int j = j1; j <= j2; j++) {
            int ret = 0;
            for (int w = 0; w < k; w++) {
                ret += A[i][w] * B[w][j];
            }
            C[i][j] = ret;
        }
    }
}

int main(){
    int A[5][10], B[10][5], C[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++)
            A[i][j] = 1;
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++)
            B[i][j] = 1;
    }

    thread t1(multiply, A, B, C, 0, 2, 0, 4, 10);
    thread t2(multiply, A, B, C, 0, 2, 5, 9, 10);
    thread t3(multiply, A, B, C, 3, 4, 0, 4, 10);
    thread t4(multiply, A, B, C, 3, 4, 5, 9, 10);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    for (int i = 0; i < 5; i++)
    {
        cout << endl;
        for (int j = 0; j < 5; j++) {
            cout << C[i][j] << " ";
        }
    }
    getchar();
    getchar();
    return 0;
}