# 程序主体设计

设计有自定义文件读写类 Fstream 与文件元素指针类 Fpointer。

设计有自定义文件索引库数据类 Fmultiset。

## 主体类：

字符串类：

```cpp
template<int length>
class cstr{
private:
    char storage[length];
public:
    cstr();
    cstr(const std::string &str);
    cstr(const cstr &other);    
    // operator < / > / <= / >= / == / !=
}
```

权限类：

```cpp
class UsrPrivilege{
private:
    int privilege;
public:
    UsrPrivilege(int pri);
    UsrPrivilege(const UsrPrivilege &other);
    operator|(const UsrPrivilege &other);
    bool has_privilege(int pri) {return privilege | pri;}
    bool has_privilege(const UsrPrivilege &other){
        return privilege | other.privilege;
    }
} clientPri(1), staffPri(3), rootPri(7);
```

类型约定：

```cpp
constexpr int cMaxStorage = 1000001;
using ISBNType = cstr<21>;
using bookInfoType = cstr<61>;
using usrInfoType = cstr<31>;
using pwdType = cstr<31>;
```

图书信息：

```cpp
class BookInfo{
private:
    ISBNType ISBN;
    bookInfoType bookname, author, keyword;
    int storage;
    double price;
}
class BookDataBase{
private:
    BookInfo books[cMaxStorage];
}
```

用户信息：

```cpp
class UsrInfo{
private:
    usrInfoType usrID, usrname, pwd;
    const UsrPrivilege privilege;
    Fpointer<BookInfo> selected_book;
public:
    // some functions. For example:
    void Login(UsrPrivilege last_prim usrInfoType &password){
        Validate(pwd, password); // implemented later
        Validate(last_pri, privilege);
        usr_stack.`
    }
}
class UsrDataBase{
private:
    UsrInfo usrs[cMaxStorage];
}
class UsrStack{
private:
    int stack_size;
    UsrInfo usr_stack[cMaxStorage];
public:
    bool empty();
    void push();
    void pop();
} usr_stack;
```

系统相关：

    启动系统：程序开始时立即启动。若检测到用户序列为空则创建root用户。

    关闭系统：quit/exit指令后立即关闭。弹出左右登录用户。

用户相关：

    账户登录：su指令后立即作用。若密码验证通过，在UsrStack中插入用户。

    账户登出：logout指令后作用。弹出UsrStack中的用户。

    用户操作：使用UsrInfo中的接口完成。有权限确认。




