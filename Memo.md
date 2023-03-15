## 线程相关

- 静态构造函数是线程安全的，其余的静态成员函数不是线程安全的；
- 系统调用函数是线程安全的；



## 库相关

- `ssize_t`类型在`<sys/types.h>`中；
- `errno`变量在`<errno.h>`中；
- `strerror()`在`<string.h>`中；
- `open()`在`<fcntl.h>`中；
- `std::string`在`<string>`中，注意不带`.h`；
- `socketpair`：
  - 参考：https://blog.csdn.net/y396397735/article/details/50684558；



## 语法相关

### 1. 指针和整型相加

- 参考：https://www.cnblogs.com/nm90/p/5706785.html；
- 实际上是（指针+整型*指针指向对象的大小），计算的是指针偏移整型个单位；
- 同理：
  - 指针和指针相减，实际上是（（指针-指针）/指针指向对象的大小），得到的是两个指针之间的偏移整型个单位值；



### 2. struct和typedef struct写法区别

- 参考：https://blog.csdn.net/Gary_ghw/article/details/128230430；
- 在c++中两种写法都可以；



### 3. 在类内使用别名

- 参考：https://blog.csdn.net/yang9649/article/details/53503729?locationNum=7&fps=1；
- 放在private就只能类内使用；



### 4. 构造函数的参数初始化列表

- 参考：https://blog.csdn.net/feng19870412/article/details/117731851；
- 先于构造函数体执行；



### 5. 断言

- 参考：
  - https://blog.csdn.net/m0_67168421/article/details/128294344；
  - https://jiuaidu.com/jianzhan/992640/；
- 仅在DEBUG下才会生效，在RELEASE下无效；
- 最好是拆开多个不同作用表达式分别assert；
- 通常放在函数的第一行；
- 为真才能通过断言；



### 6. 构造函数和析构函数

- 只要声明了有参构造函数，就不会自动生成默认无参构造函数；
- 不声明复制构造函数就会自动生成复制构造函数；
- 析构函数也是只声明（即使不定义）就不会自动生成；
- 编译时，会在第一次调用时报错；

```c++ 
class X {
public:
    // ...
    X(); // default constructor
    virtual ~X();            // destructor (virtual if X is meant to be a base class)
    X(const X&);             // copy constructor
    X& operator=(const X&);  // copy assignment
    X(X&&);                  // move constructor
    X& operator=(X&&);       // move assignment
};
```

- 数组如果不是用new申请空间的话，就不需要在析构函数中用delete释放空间；

- 参考：
  - https://blog.csdn.net/weixin_48622537/article/details/109729714；
  - https://01io.tech/cpp-rules-of-three-five-zero/；



### 7. const修饰的类对象

- 任何成员都不能被修改；
- 仅能调用const成员函数；
- 参考：https://www.cnblogs.com/renzhuang/articles/6672664.html；



### 8. 表达式的隐式类型转换

- 参考：
  - https://developer.aliyun.com/article/922575；
  - https://blog.csdn.net/hanchaoman/article/details/7827031；
- long大多数情况下是32位，少数情况下是64位；
- 低于int的转int；
- 有较高的转较高的，无符号的比有符号的高；



### 9. 函数默认参数

- 要在声明处就给定，只在定义处给定不生效；
- 参考：https://blog.csdn.net/RED_STARRR/article/details/122469322；



### 10. 字符串输出

- `%s`是遇到`\0`或者字符串数组结尾才不输出；
- 遇到`\n`还是会继续输出的；



### 11. 函数指针别名

- 定义了函数指针别名后，还需要用别名定义函数指针变量才能指向函数；

```c++
	// 定义函数指针别名
	using read_fn = ssize_t(*)(buffer_t*, void*, size_t);
	//typedef ssize_t(*read_fn)(buffer_t*, void*, size_t);
	//read_fn fnp = BufferIO::buffer_read;
	//read_fn fnp = BufferIO::buffer_read_n;
	read_fn fnp = BufferIO::buffer_read_line;
```



### 12. 静态成员

- 静态函数只能调用静态成员变量；

  - 但如果静态函数调用了构造函数，则构造函数内可以使用非静态成员变量；
  - 如果是调用别的函数，则必须是静态函数；

- 静态成员变量不能在构造函数中初始化，只能在类外初始化（常量整型类型可以在类内初始化）；

  - 类外初始化不需要写static；

  - 即使是私有也可以在类外初始化；

  - 静态成员变量默认会赋初始值；
  - 而且不能在`.h`文件中初始化，因为初始化是定义操作，会造成重复定义的，应该在`.cpp`中初始化，参考：https://ask.csdn.net/questions/655880；
  - 必须在类外初始化；

- 参考：

  - https://blog.csdn.net/hit0803107/article/details/98531978；



### 13. 数组初始化

- 能不用new的就不要用new，因为new还要手动delete，徒增工作量；
- 而且用的是堆空间，而不是栈空间，速度慢；
- 参考：https://blog.csdn.net/hy2014x/article/details/113335480



### 14. 多态和继承

- 基类指针指向派生类后，可以调用派生类重载的虚函数，从而间接调用派生类的成员变量；
- 但基类不能直接调用派生类的成员变量或者成员函数，因为基类中没有这些数据；
- 参考：
  - https://www.cnblogs.com/zhaozhibo/p/14982253.html；
  - https://robot.czxy.com/docs/cpp/day06/01_dynamic/#3；



### 15. 模板类的声明和定义

- 不能分开成`.h`和`.cpp`，必须都写到`.h`中，否则所有用到的函数都会有`error: undefined reference to ×××`的报错；
  - 可能是因为编译器直到首次遇到实例化模板时才会生成真正的类（在扫描`.cpp`s时并不生成），但这个时候只能找到`.h`文件，无法和`.h`对应的`.cpp`文件，也就是说不知道在哪里实现的；
  - 因此这样是没有办法生成真正的类的；
  - 普通的类是因为在扫描到`.cpp`文件的时候就会编译生成了，所以可以不写在`.h`中；
- 另外，在类内和函数内部，可以不写`<T>`，有需要的地方直接用T即可；
- 参考：
  - https://blog.csdn.net/amnesiagreen/article/details/108575310；



### 16. errno

- 仅系统调用发生错误才会修改errno的值；
- 仅在Unix环境下使用；



### 17. 异常机制

- 参考：
  - https://www.runoob.com/cplusplus/cpp-exceptions-handling.html；
  - https://blog.csdn.net/Hello_World_213/article/details/128217964；
- throw后面跟什么类型，就能catch到什么类型；
- 自定义异常类并不需要继承别的基类，可以自由实现，但推荐继承标准的exception类；
- const throw()表示该函数不会抛出任何异常，但仅作声明用，因为它还是可以往外抛出异常的；
- 用于代替发生错误时的终止进程；



### 18. thread传参

- thread可以共享进程的内存空间，thread也拥有自己独立内存；
- 但thread对象只能用初始化时传进去的参数；
- 传：
  - 值，在线程中修改不改变原值，无法实现共享；
  - 引用，用`std::ref()`作为实参才可实现共享；
  - 指针，可以实现共享；
  - 注意detach()后，主线程和各线程脱离，主线程可以先行终止；
    - 因此在各线程使用主线程指针或者引用时必须先检查主线程是否已经释放内存了；
- 参考：
  - https://blog.csdn.net/jinking01/article/details/108619470；
  - https://geek-docs.com/cpp-multi-thread/cpp-thread-management/cpp-the-basics-of-thread-management.html；



### 19. 类的前向引用声明

- 需要使用的类在当前类的声明之后；
- 尤其是那些你中有我我中有你的类定义，比如单例模式下的CRTP实现方式；
- 在要使用的地方之前声明`class`即可；

```
class B; //前向引用声明B
```

- 作用类似于在需要使用全局变量之前声明`extern`扩展作用域；



## 注释规范

- 用的是doxygen注解规范；
- 参考：
  - https://www.cnblogs.com/aspiration2016/p/8433122.html；
  - https://www.guyuehome.com/35640；
- 官方：https://www.doxygen.nl/manual/docblocks.html；



### 1. DEBUG宏注释

- 参考：https://blog.csdn.net/Qidi_Huang/article/details/50405639；





## 进程池设计

- 通过I/O多路复用保证并发；
- 资源之间没有竞争关系，无需考虑线程安全；
- Task类只能用模板实现静态多态，不能用基类指针实现动态多态；
  - 因为参数的传递是通过管道通信的，通信后指针多态就失效了；
  - 因此只能在编译期就绑定，整个进程池都只能执行同一类任务（虽然该任务可以用模板实现静态多态）；





## 线程池设计

- thread类的使用：https://fulin.blog.csdn.net/article/details/115024585；
- 线程池线程共享线程池变量：将线程池`this`传入线程函数；
  - 也就是说，线程可以看作是独立的一个隔离子模块；
  - 参考：https://blog.csdn.net/K_ZhJ18/article/details/128408525；
- Task类可以用基类指针实现多态；
  - 这样能够实现运行期绑定，在逻辑上更加优雅；
  - 整个线程池在运行期可以执行不同的任务类对象，只要它们都继承了统一的执行基类即可；
