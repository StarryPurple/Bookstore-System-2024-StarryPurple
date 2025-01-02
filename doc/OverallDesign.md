# ACM2024: Bookstore System 总体设计文档

作者：孔繁亚

## 功能概述

本项目实现有一个书店管理系统，使用类命令行的方式完成交互，支持以顾客，员工与店长三种权限进行购书，进货，修改书籍资料，查询书籍等各种操作，并赋有简单的日志管理。

使用文件存储数据，支持退出系统后保留数据库。

## 主体逻辑说明

将处理过程非为三层：command manager 层处理输入指令，info manager 层将输入指令翻译为基本指令执行，info database 层执行基本指令并反映到数据库/日志中。每层均有一定的指令合法性检查，info manager 层有用户权限检查。

后两层大致分成三个模块：

user manager / user database 处理用户信息相关指令，如注册，登录，注销，修改密码等。维护有登录栈，支持多层登录；

book manager / book database 处理图书信息相关指令，如图书检索，信息修改，当前用户选定书籍，书籍买入/卖出等。

log manager / log database 将各个指令分类存储，支持店长查看记录。

## 代码文件结构

```
|
|---doc/ 存放说明文档
|
|---data/ 存放数据库数据...至少是这么设计的。似乎评测时这么做有点麻烦
|
|---include/ .h文件存放处，项目依赖文件库
|   |
|   |---bookstore_exceptions.h 项目所用异常处理类集合
|   |---filestream.h 文件读写类
|   |---utilities.h 存有In Memory Index方法类，与定长字符串等数据结构
|   |---validator.h 存有一类验证器类，拥有expect函数做应用接口
|   |---infotypes.h 各种bookstore基本信息类
|   |---info_database.h 各种bookstore数据库信息类
|   |---info_manager.h 各种bookstore指令验证与翻译类
|   |---command_manager.h bookstore指令解析类
|
|---src/ .cpp文件存放处，各种非模板函数的实现
|   |
|   |---bookstore_exceptions.cpp
|   |---utilities.cpp
|   |---infotypes.cpp
|   |---info_database.cpp
|   |---info_manager.cpp
|   |---command_manager.cpp
|
|---template/ .tpp文件存放处，各种模板函数的实现
|   |
|   |---filestream.tpp
|   |---utilities.tpp
|   |---validator.tpp
|
|---Main.cpp 工程的main函数所在处
|
|---README.md
|
```

## 功能设计

```
|---指令解析模块：读入指令，解析内容，分发给不同的执行模块
|   |
|   |---用户相关指令执行模块：执行用户信息相关指令。
|   |   |
|   |   |---用户登录指令执行模块:"su"
|   |   |---用户自主注册指令执行模块:"register"
|   |   |---添加用户指令执行模块:"useradd"
|   |   |---用户修改密码指令执行模块:"pwd"
|   |   |---用户登出指令执行模块:"logout"
|   |   |---用户注销指令执行模块:"delete"
|   |
|   |---图书相关指令执行模块：执行与图书信息/交易相关指令
|   |   |
|   |   |---当前用户选书指令执行模块:"select"
|   |   |---信息反查图书指令执行模块:"show"
|   |   |---进货指令执行模块:"import"
|   |   |---向书店买书指令执行模块:"buy"
|   |   |---图书信息修改指令执行模块:"modify"
|   |
|   |---日志相关指令执行模块：执行日志记录/输出相关指令
|   |   |
|   |   |---添加日志指令
|   |   |---查看交易额 执行模块 “show finance”
|   |   |---查看交易记录 执行模块 "report finance"
|   |   |---查看工作记录 执行模块 "report employee"
|   |   |---查看系统记录 执行模块 "log"

```






## 数据库设计

需要在外存长期存储的数据包括：

已注册的用户信息 （以id反查表的形式存储，下同）

userID到用户的反查表

已登记的图书信息 id反查表

ISBN，书名，作者，关键字到图书的反查表

系统财报日志 id反查表

员工工作日志 id反查表

全体系统日志 id反查表

## 类与结构体设计

异常类：class ValidatorException, class FileException,...

文件读写类：class Fstream, class Fpointer 支持内存数据到文件内数据的映射

InMemory Index系统：class Fmultimap 基于文件的类std::multimap查询表

定长字符串类：class ConstStr 一个长度固定的，类std::string数据结构

验证器：class Validator, Validator &expect(T val) 一个简单的格式验证器



用户信息：class UserType 记录用户名，密码等

图书信息：class BookType 记录ISBN，书名等

日志信息：class LogType 记录本条日志对应的历史收支与操作信息



用户登录信息：class LoggedUserType 记录登录的用户信息与TA选定的书籍信息

用户登录栈：class UserStack 支持登录信息记录/更新与询问当前用户

用户数据库：class UserDatabase 记录用户数据，支持基础的用户数据修改相关指令

图书数据库：class BookDatabase 记录图书数据，支持基础的图书数据修改相关指令

日志数据库：class LogDatabase 记录日志数据，支持日志添加与展示



用户管理模块：class UserManager 执行用户相关指令并反映到用户数据库与日志数据库中

图书管理模块：class BookManager 执行图书相关指令并反映到图书数据库与日志数据库中

日志管理模块：class LogManager 执行日志相关指令



总管理模块：class CommandManager 解析指令

## 其他

你可能看到项目库里有一些我没提到的文件，它们是开发副产物，并未参与到这一阶段的具体应用中。
