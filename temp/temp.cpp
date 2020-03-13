#include <iostream>
#include <future>
#include <thread>
#include <mutex>

 int Sum(std::future<int> &f) {
     int sum = 0;
     int s;
     s = f.get();
     for (int i = 0; i < s; ++i) sum += i;
     return sum;
}

int main() {

    int x;
    std::promise<int> p;
    std::future<int> f1 = p.get_future();

    std::future<int> f2 = std::async(std::launch::async | std::launch::deferred, Sum, std::ref(f1));
    //std::future<int> fu = std::async(Sum, std::ref(f1));  The same as above
    //Let compiler decide which one to implement as default option

    p.set_value(100);
    std::cout << f2.get() << std::endl;

    return 0;
}