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
- `setsockopt`：
  - 参考：https://www.cnblogs.com/felove2013/articles/4087511.html；
- `struct sockaddr_in`在`<netinet/in.h>`中；
  - 参考：https://blog.csdn.net/u010921682/article/details/79716540；



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



### 20. C常用函数说明

```c++
// 将str2拷贝到str1
// 返回空
strcpy(str1, str2);

// 判断str1中是否含str2
// 返回第一个匹配字母指针
strstr(str1, str2);

// 将str2追加到str1末尾
// 返回空
strcat(str1, str2);

// 从str中按照format格式读入到变量中，用于解析str
int sscanf(const char *str, const char *format, ...);
// 一个例子如下：
strcpy( dtm, "Saturday March 25 1989" );
sscanf( dtm, "%s %s %d  %d", weekday, month, &day, &year );

// 用忽略大小写比较字符串str1和str2
// 相同返回0
strcasecmp(str1, str2);

// 将file_name的文件状态复制到buf中
// 成功返回0，失败返回-1
int stat(const char * file_name, struct stat *buf);

// 设置当前进程的环境变量name: value
// 成功返回0，错误返回-1
int setenv(const char *name,const char * value,int overwrite);
```



### 21. 提前声明

- 可以在当前类中使用后面声明定义的类，只需要在前面增加：

```C++
class 类名;
```



- 但不能在类内函数使用后面声明和定义的类的成员：
  - 如果要使用后面声明和定义的类，函数只能类外定义；
  - 而且函数的定义需要在**所有用到的类**的声明之后；
  - 如果是`.h`和`.cpp`分开写的话就没有问题，但如果是混在一起写就需要把这些需要用到后面声明类的函数放到最后定义和实现；



### 22. 调用无参构造函数

- 调用无参构造函数的方法如下：

```c++
WeakPointer wptr;
WeakPointer* pwptr = new WeakPointer();
WeakPointer* pwptr = new WeakPointer;
```



- 注意，以下的写法是声明了一个函数而非变量，但由于没有函数实现，所以也无法调用：

```C++
WeakPointer wptr();
```



- 在**定义时赋值**是直接调用构造函数而不是先调用构造函数再调用赋值运算符重载函数，如下：

```
WeakPointer wptr = WeakPointer();
// 直接调用复制构造函数，相当于
WeakPointer wptr(WeakPointer());
```



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

  - **warning**：**这个实现是不行的**，因为Task无法通过值传递传到threadpool对象中，只能传指针，也就是说它的生命周期只有主线程的作用域内；
  - Task对象（在栈上）很有可能被销毁或者覆盖，这样后续线程通过指针取到的内存也就不是之前的那块内存了；
  - 神奇的是在复现这个猜想时，想通过正常压栈覆盖对象好像是做不到的，只能取地址然后精确覆盖，这样确实是会导致多线程取到同一个内容；

- 只能用模板然后值传递，在队列中用值拷贝push，因为threadpool实例的生命周期是整个进程，这样能保证Task的内容有效；





## 服务器设计

- http请求如何到服务器进程？
  - 只要服务器进程绑定了ip和端口，并设置ACCEPT监听该端口，则任何发送到该ip和端口的HTTP报文都会直接通过socket_fd被服务器进程读取到；
  - 可以被读取到的内容是HTTP报文的全部内容，包括请求行、消息头和消息体；
  - 同样，服务器发送给客户端的全部内容也是报文头
- SIGPIPE信号：
  - `SIGPIPE`产生的原因是这样的：如果一个 socket 在接收到了 RST packet 之后，程序仍然向这个 socket 写入数据，那么就会产生`SIGPIPE`信号。
  - 如果客户端已经关闭但服务器仍尝试发送数据给客户端，从而引发Broken pipe异常，需要通过接收SIGPIPE信号来处理该异常，避免服务器终止；
  - 在GDB调试时接收到信号无论是否已经绑定了处理函数，都会自动中断；
    - 但实际上，点击继续即可继续执行；
    - 在真实运行时，进程并不会因为中断而停止执行；
  - 参考：http://senlinzhan.github.io/2017/03/02/sigpipe/；
- CGI服务器：
  - 可以创建进程执行CGI程序作为动态内容的服务器；
  - CGI程序是指满足CGI规范的程序：
    - 通过环境变量选项传入参数；
    - 可以由HTTP请求行调用和传参；
    - 请求行中，参数和程序名称之间用"?"隔开，参数之间用"&"隔开；
    - 输出给客户端的内容包括响应报文消息头和消息体；
- 用谷歌浏览器进行测试的时候，谷歌浏览器似乎是会连续刷多一个请求，很容易就把服务器弄崩掉了，最好还是用postman测试，比较稳定；
  - 其实只要处理了SIGPIPE信号就不会崩；
- 使用进程池服务器时，listen_fd的创建一定要在创建子进程前进行，否则子进程看不到打开的listen_fd；使用线程池服务器时，listen_fd的创建最好是在创建子线程前进行，这样比较统一，但非必要；

- 手动发送请求行字符串时，注意URL不要加上**ip地址和端口**，实际的请求行是会自动隐去这两个信息的，因为这些信息并不是在HTTP应用上使用；





## 智能指针设计

- `shared_ptr`并不能保证它指向的内存所有的shared_ptr都正确引用计数；
  - 它仅能保证通过shared_ptr构造传递或者赋值传递的shared_ptr都可以正确引用计数，如果用原生指针构造，有可能会发生重复析构；
  - 通过`make_shared(对象)`会自动为对象构造一份副本，所以它可以保证不发生重复析构，更推荐使用；
  - 原生指针构造是提供了一种C指针的兼容方式，但并没有提供编译期安全保证，所以使用的时候要特别小心；
    - 如果是栈上的地址，如`int a = 2`，传入&a构造，则会两次析构；
    - 如果是堆上的地址，如`int *a = new int(2)`，但多次传入a构造，则也会多次析构；
  - 线程安全：
    - 仅保证引用计数是线程安全的，因为使用了原子变量来计数；
    - 但取地址和取值的过程均不是线程安全的；
    - 参考：
      - https://www.ccppcoding.com/archives/202；
      - https://cloud.tencent.com/developer/article/1688444；
  
- `unique_ptr`同样也不能保证它指向的内存仅由一个unique_ptr指向；
  - 它仅保证智能指针之间的传递时仅有一个unique_ptr指向，如果用原生指针构造，有可能发生重复析构；
  - 原生指针构造是提供了一种C指针的兼容方式，但并没有提供编译期安全保证，所以使用的时候要特别小心；
  - 实现：
    - 需要删掉左值引用的复制构造函数和赋值运算符重载函数；
    - 因为涉及到对象的转移和释放，都需要加锁保证操作的完整性；
  
- 引用计数用的变量需要是原子的，这样做的目的是：
  - 避免多线程读写引起该变量的数据错误；
  - 但不是说这样就不需要加锁了，在增加shared引用的时候还是要加锁，否则可能会导致重复析构；
  - 只是说不需要每次++或者--都加锁，比如weak引用的++和--就不需要加锁；
  
- 为三种智能指针增加一个cblock，记录：
  - shared_ptr计数，原子变量；
    - 为0时释放指向对象的内存空间；
  - weak_ptr计数，原子变量；
    - 为0时释放cblock空间；
  - 和指向对象绑定的互斥量；
  
- 赋值运算符重载一定要记得**先释放资源**再修改资源指向；

  复制构造函数则不需要，因为原本就是没有指向资源；

- 参考：
  - https://github.com/tacgomes/smart-pointers；
