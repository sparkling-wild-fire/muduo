# bind()函数一例

问题：遇到这行网络通信代码`std::bind(&Acceptor::listen, acceptor_.get());`，给我整不会了

## bind()函数

std::bind 是 C++11 中的一个函数模板，用于将一个可调用对象（函数、函数指针、成员函数、函数对象等）和其参数绑定成一个新的可调用对象。

如：

```c++
#include <iostream>
#include <functional>

void func(int a, int b, int c) {
    std::cout << "a = " << a << ", b = " << b << ", c = " << c << std::endl;
}

int main() {
    auto f = std::bind(func, 1, std::placeholders::_2, 3);
    f(2, 4, 6);
    return 0;
}
```

在上面的示例中，我们定义了一个名为 func 的函数，它接受三个参数。然后我们使用 std::bind 函数将 func 和参数 1 和 3 绑定成一个新的可调用对象 f。在绑定参数的过程中，我们使用了占位符 _2，表示在调用 f 时，第二个参数会被传入。最后，我们调用 f(2, 4, 6)，输出结果为：

`a = 1, b = 4, c = 3`

可以看到，f 的第二个参数被绑定成了 4，而第一个和第三个参数则分别传入了 2 和 6。

## 解释

acceptor_的声明为：`std::unique_ptr<Acceptor> acceptor_;`

&Acceptor::listen() 函数自带一个参数，也就是this，而acceptor_.get()返回只能指针的原始类型，也就是Acceptor*，用于指示被绑定对象的实例

也就是绑定后的新的可调用对象可以像普通函数一样被调用，但是会自动传入被绑定的对象指针作为第一个参数。

通过将 listen 函数和 acceptor_.get() 所管理的原始指针绑定成一个新的可调用对象，可以方便地在需要监听连接请求的时候调用该对象，而不需要每次都写一遍 acceptor_.get()->listen()
