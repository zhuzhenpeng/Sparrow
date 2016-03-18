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
  //清空栈，然后跳转到新的代码地址
  CALL, 

  //函数返回，返回值（如果有）将压入栈中。弹出调用栈，恢复调用者的代码地址
  RET, 

  //跳转操作，分为无条件跳转，如果为真（假）则跳转
  BR, BRT, BRF, 

  //操作全局（非局部）变量，根据下标从名称索引表中获取名称并从外层环境中查找，
  //将结果压入栈中或将栈顶元素弹出对其进行赋值
  GLOAD, GSTORE, 

  //操作局部变量，根据下标从局部环境中查找，
  //将结果压入栈中或将栈顶元素弹出对其进行赋值
  LOAD, STORE, 
  
  //调用原生函数，根据原生函数的参数个数要求，从操作数栈中弹出相应个数的操作数，
  //如果函数有返回值，则将返回值压栈
  NATIVE_CALL,
  
  //向操作数栈压入空对象
  NIL, 
  
  //弹出栈顶元素的值
  POP,
  
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

  unsigned call(unsigned index);

  unsigned ret();

  unsigned br(unsigned index);

  unsigned brt(unsigned index);

  unsigned brf(unsigned index);

  unsigned gload(unsigned index);

  unsigned gstore(unsigned index);

  unsigned load(unsigned index);

  unsigned store(unsigned index);

  unsigned nativeCall(unsigned index);

  unsigned nil();

  unsigned pop();

  unsigned halt();

  //获取代码
  const std::vector<unsigned> &getCodes() const;

private:
  unsigned push(unsigned code);

private:
  std::vector<unsigned> codes_;
};
using CodePtr = std::shared_ptr<Code>;

#endif
