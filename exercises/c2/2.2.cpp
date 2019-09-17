// 尝试用折叠表达式实现用于计算均值的函数，传入允许任意参数。

#include <iostream>

template<typename ...Args>
auto average(Args... args) {
    static_assert(sizeof...(args) > 0);
    return (... + args) / (double) sizeof...(args);
}

int main() {
    std::cout << average(1, 0, 1.5, 8.9) << std::endl;
    return 0;
}
