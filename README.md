# My_TinyDB
简单关系数据库实现，加深对MySQL各个模块底层的理解
## 简介

基于B+树的关系型数据库系统的实现,主要用于加深自己对数据库底层实现的理解。

**支持功能：**

- 简单SQL语句解析
- B+树索引加速查询
- 数据持久化

## 运行

```
mkdir build
cd build
cmake ..
make 
cd ..
chmod +x DB
```

## 设计模块

- **交互模块**：为用户提供交互选项，返回查询结果。
- **解析模块**：简单词法、语法、语义分析，解析sql语句。
- **执行模块**：核心模块，执行增删改查操作，执行优化。
- **存储模块**：管理数据表、数据索引在内存和磁盘的组织方式，保证一致性和持久性。

### 交互功能

- **server类**：提供交互界面，提供接收用户输入，转到相应的解析器、执行器完成操作。

### 解析模块

- **词法分析**：以扫描的方式（自动机状态转化的思路），将token划分为关键字和非关键字。

  部分关键字如下：

  ```
  关键字					（种别码，属性值）
  create				  (CREATE，_)
  select					(SELECT，_)
  insert					(INSERT，_)
  drop					  (DROP，_)
  table					  (TABLE, _)
  "&&"        		(SYM, AND_AND_SYM))
  "<"          		(SYM, LT)
  "<="        		(SYM, LE)
  "<>"         		(SYM, NE)
  "!="       			(SYM, NE)
  "="           	(SYM, EQ)
  ">"           	(SYM, GT)
  ">="         		(SYM, GE)
  all         		(SYM, ALL_SYM)   
  where					  (WHERE, _)
  ```

- **语法分析**：语法分析器(parser)从词法分析器输出的token序列中识别出各类短语，从而构造语法分析树(syntax tree)，并判断源程序在结构上是否正确。

  选择使用简单的LL(1)文法：

  ```
  step1:设计LL(1)文法
  step2:构造文法符号的first集
  step3:构造文法符号的follow集
  step4:构造预测分析表，表示文法符号可能的开头符号a
  step5:分析栈
  	  结束符号$，开始符号S入栈；指针index指向分析字符串的首字符a
  	  while(!stack.empty())
  	  	if(stack.top() == a)
  	  		pop();
  	  		index++;
  	  	else if 查表正确
  	  		pop();
  	  		index++;
  	  	else
  	  		error;
  step6:根据语法动作生成语法树
  ```

- **语义分析**：

  设计statement类型，将不同的语义树对应不同的查询请求，省去中间代码生成的部分。

  ```
  class statement 为基类，根据不同语义动态生成相应功能的派生类
  	create_s
  	use_s
  	select_s
  	drop_s
  	show_s
  	insert_s
  	delete_s
  ```

### 执行模块

#### B+树索引

1. 每个节点存储key更多，数据紧密，更好的空间局部性。
2. 数据存储在叶子节点上，查询稳定，避免出现查询抖动。
3. （对比二叉树）多叉树深度低，IO交换少，查询快。
4. （对比B-树）叶子节点间链式存储，有利于区间查询。

#### 执行过程

预处理→优化器→执行器

```
if condition use primary:
	查询主键索引；
else
	全表扫描；
```

### 存储模块

#### Table

**文件格式**：.dat

**类实现**

```
//属性：
string table_name;
vector<Page> pages;
uint32_t cnt_tuple_{0}; //已经存放的tuple数目
uint32_t tuple_per_page_; //每页存放tuple数目
uint32_t tuple_max_num_; //最多tuple数目

//方法：
WriteTuple(Tuple t) 传入tuple，写入表中
ReadTuple() 读出指定tid的tuple
LocateTuple(auto tid) 传入tid,返回写入表位置
```

#### Page

```
//一个表为4MB = 1024页
//一个页为4KB = 4096B
uint32_t page_size = 4096;
uint32_t page_num = 1024;
```

#### Tuple

```
//tuple表示一行数据

//构造函数
Tuple(std::vector<value> &v)

//方法：
write(dst) //将当前数据写入指定位置
read(src) //从src读入到tuple之中
void deserialize(std::vector<Value> &values) //读出当前元组的数据,存到vs中
```

#### Column

```
- | 属性            | 说明                          |
  | --------------- | ----------------------------  |
  | TYPE_ID type_   | 当前列的类型,值为INT或STRING   |
  | string col_name | 当前列名                      |
  | is_primary_     | 当前列是否为主键               |
  | col_size_       | 当前列的大小                  |

```

