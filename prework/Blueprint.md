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

用户权限类：

```cpp
class UsrPrivilege{
private:
    int privilege;
public:
    UsrPrivilege(int pri);
    UsrPrivilege(const UsrPrivilege &other);
    operator|(const UsrPrivilege &other);
    bool operator<=(const UsrPrivilege &other){
        return (privilege | 1) <= (other.privilege | 1)
            && (privilege | 2) <= (other.privilege | 2)
            && (privilege | 4) <= (other.privilege | 4);
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
class bookType{
    friend Manager;
private:
    ISBNType ISBN;
    bookInfoType bookname, author, keyword;
    int storage;
    double price;
}
class BookDataBase{
    friend Manager;
private:
    Fpointer<bookType> books[cMaxStorage];
}
```

用户信息：

```cpp
class usrType{
    friend Manager;
private:
    usrInfoType usrID, usrname, pwd;
    const UsrPrivilege privilege;
    Fpointer<bookType> selected_book;
}
class UsrDataBase{
    friend Manager;
private:
    usrType usrs[cMaxStorage];
}
// may be either stored in file or memory
class UsrStack{
    friend Manager;
private:
    int stack_size;
    Fpointer<usrType> usr_stack[cMaxStorage];
public:
    bool empty();
    void push();
    void pop(); // remember to change some status of the active user
} usr_stack;
```

控制系统（全程存储在内存）：

```cpp
class Manager{
private:
    usrType active_usr; // modified dynamically with usr_stack
public:
    // exposed functions. For example:
    void login(usrType &usr, usrInfoType &pwd){
        Validate<usrInfoType>(pwd).is(usr.pwd); // implemented later
        Validate<UsrPrivilege>(active_usr.privilege).lesserThan(usr.privilege);
        usr_stack.push(*this);
    }
    void logout(){
        usr_stack.pop();
    }
    void register(usrType &new_usr);
    void change_pwd(const usrInfoType &old_pwd, const usrInfoType &new_pwd);
    void usr_add(const usrType &new_usr);
    void delete_usr(const usrInfoType &usrID);

    void show_book(const ISBNType &ISBN);
    void show_book(const bookInfoType &info, char kwd_type);
    void buy_book(const ISBNType &ISBN, int quantity);
    void select_book(const ISBNType &ISBN);
    void modify_book(const ISBNType &ISBN);
    void modify_book(const bookInfoType &info, char kwd_type);
    void import_book(const ISBNType &ISBN, int quantity);
    
    void show_finance(int count);
    void report_finance();
    void report_employee();
    void log();
}
```

相关指令：

    启动系统：程序开始时立即启动。若检测到用户序列为空则创建root用户。

    关闭系统：quit/exit指令后立即关闭。弹出左右登录用户。

    账户登录：su指令后立即作用。若密码验证通过，在UsrStack中插入用户。

    账户登出：logout指令后作用。弹出UsrStack中的用户。

    用户操作：使用Manager中的接口完成。接口内部有权限确认。
