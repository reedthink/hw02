/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <cstring>
#include <iostream>

struct Node {
    // 这两个指针会造成什么问题？请修复  //导致循环引用
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？ // 1.显式调用 2.初始化变量 3.列表初始化
    explicit Node(int val) {
        value = val;
        next = nullptr;
        prev.lock() = nullptr;
    }

    // Node(int val)
    //     : value(val)
    // {}

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next;
        node->prev = prev;
        if (prev.lock())
            prev.lock()->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        if (prev.lock())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？ // 13 次，因为拷贝构造函数没有深拷贝
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        // 深拷贝就是要拷贝List的内存数据。。怎么拷贝,只拷贝头显然是不够的。需要新建所有节点
        head = std::make_shared<Node> (other.head->value);
        auto newCurr = head;
        // newCurr = nullptr;
        // std::cout << newCurr << "!!!!!!!!!!" << head << std::endl;
        for (auto curr = other.front()->next.get(); curr; curr = curr->next.get())
        {
            auto node = std::make_shared<Node>(curr->value);
            node->prev = newCurr;
            if (newCurr)
                newCurr->next = node;
            newCurr = node;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    //在程序中没有真正使用拷贝赋值函数，唯一进行了List对象的=操作的List b = a;实际上调用的是拷贝构造函数。

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List const &lst) {  // 有什么值得改进的？//传入常引用
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]
    // std::cout << a.head.use_count() << std::endl;

    List b = a; // 调用了拷贝构造函数

    // std::cout << b.head.get() << " "<< a.head.get() << std::endl;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
