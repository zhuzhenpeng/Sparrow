#ifndef SPARROW_CODE_H_
#define SPARROW_CODE_H_

#include <vector>
#include <memory>

/**函数字节代码
 *  函数（或闭包）经过编译后，产生的字节码存放在该类的实例中，运行时从这里获取
 *字节代码
 *
 *  指令分为无操作数、单操作数、双操作数，分别对应1个、5个、9个字(32位)
 *  下面注释中的“栈”泛指“操作数栈”，如果是“调用栈”会直接标明“调用栈”
 *  函数调用如果没有返回值，则往操作数栈压入空对象（这在函数代码翻译成字节代码
 *时实现）
 */

enum Instruction {
  //四则运算，弹出两个操作数，进行运算并把结果压入栈中
  ADD, SUB, MUL, DIV, MOD, 

  //比较操作，弹出两个操作数进行相应的比较，并把结果压入栈中
  EQ, LT, BT, LE, BE, NEQ, 

  //获取常量池中的常量，转化为相应的类型对象并压入栈中
  SCONST, ICONST, FCONST, 

  //调用函数，将新的栈帧压入调用栈中，将栈中的入参传递给栈帧，
  //进入下一轮循环
  CALL, 

  //函数返回，返回值（如果有）将压入栈中。弹出调用栈，进入下一轮循环
  RET, 

  //跳转操作，分为无条件跳转，如果为真（假）则跳转
  BR, BRT, BRF, 

  //与（或）逻辑，从栈中获取两个对象，执行与（或）逻辑
  AND, OR,

  //操作全局（非局部）变量，根据下标从名称索引表中获取名称并从外层环境中查找，
  //将结果压入栈中或将栈顶元素弹出对其进行赋值
  GLOAD, GSTORE, 

  //操作闭包变量，根据下表从局部环境的上层环境查找
  //将结果压入栈或将栈顶元素弹出对其进行赋值
  CLOAD, CSTORE,

  //操作局部变量，根据下标从局部环境中查找，
  //将结果压入栈中或将栈顶元素弹出对其进行赋值
  LOAD, STORE, 

  //生成数组
  ARRAY_GENERATE,

  //访问数组中的某个元素
  ARRAY_ACCCESS,

  //LAMBDA表达式
  LAMB,
  
  //域访问
  DOT_ACCESS,

  //域赋值
  DOT_ASSIGN,

  //元字符指令，虚拟机从栈帧中获取相应的字符串，该串在语言中不是字符串，
  //但其在被虚拟机处理时需要当作字符串，如域访问、域赋值的后缀(eg. a.b中的b)
  RAW_STRING,

  //创建对象
  NEW_INSTANCE,

  //弹出栈顶元素，取其负值并压入栈
  NEG,
  
  //中止程序
  HALT
};

class Code {
public:
  //生成指令，并返回指令的最后的地址
  unsigned add();

  unsigned sub();

  unsigned mul();

  unsigned div();

  unsigned mod();

  unsigned eq();

  unsigned lt();

  unsigned bt();

  unsigned le();

  unsigned be();

  unsigned neq();

  unsigned sconst(unsigned index);

  unsigned iconst(unsigned index);

  unsigned fconst(unsigned index);

  unsigned call(unsigned paramNum);

  unsigned ret();

  unsigned br(unsigned index);

  unsigned brt(unsigned index);

  unsigned brf(unsigned index);

  unsigned andLogic();

  unsigned orLogic();

  unsigned gload(unsigned index);

  unsigned gstore(unsigned index);

  unsigned cload(unsigned index);

  unsigned cstore(unsigned index);

  unsigned load(unsigned index);

  unsigned store(unsigned index);

  unsigned arrayGenerate(unsigned size);

  unsigned arrayAccess();

  unsigned lamb(unsigned lambSrcIndex);

  unsigned dotAccess();

  unsigned dotAssign();

  unsigned rawString(unsigned index);

  unsigned newInstance();

  unsigned neg();

  unsigned halt();

  //获取代码
  std::vector<unsigned> &getCodes();

  //获取编译时使用的下一个位置
  unsigned nextPosition();

  //设置某个位置的字节码
  void set(size_t index, unsigned code);

  //获取某个位置的字节码
  unsigned get(size_t index) const;

  //返回字节码的长度
  unsigned getCodeSize();
private:
  unsigned push(unsigned code);

private:
  std::vector<unsigned> codes_;
};
using CodePtr = std::shared_ptr<Code>;

#endif
