#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <sstream>

using namespace std;
// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
    virtual void GenIR(string &peValue, int *no, bool &isIV) const = 0;
    static fstream *oir;
    static void initOdir(const std::string &filename, std::ios_base::openmode mode);
    // 保存生成的ir字符串序列
    static stringstream ss;
    static string &res;
};
class NumberAST : public BaseAST
{
public:
    int number;
    NumberAST(int _number) : number(_number) {}
    void Dump() const override
    {
        std::cout << this->number << "\n";
        // BaseAST::ss << this->number << "\n";
        *BaseAST::oir << this->number << "\n";
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
    }
    void Num2IR(string &numValue)
    {
        numValue = to_string(number);
    }
};
typedef enum
{
    EXP,
    NUM
} exp_num_t;
class ExpAST;
class PrimaryExpAST : public BaseAST
{
public:
    exp_num_t tag;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> num;
    PrimaryExpAST(exp_num_t _tag, std::unique_ptr<BaseAST> &_expOrNum) : tag(_tag)
    {
        if (_tag == EXP)
        {
            exp = std::move(_expOrNum);
        }
        else
        {
            num = std::move(_expOrNum);
        }
    }

    void Dump() const override
    {
        if (tag == EXP)
        {
            exp->Dump();
        }
        else
        {
            num->Dump();
        }
        string s1 = "  PrimaryExpAST生成的IR代码 ";
        cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->PE2IR(peValue, no, isIV);
    }
    void PE2IR(string &peValue, int *no, bool &isIV) const
    {
        if (tag == EXP)
        {
            // 生成表达式代码
            // std::unique_ptr<ExpAST>(dynamic_cast<ExpAST *>(exp.get()))->Exp2IR(peValue, no, isIV);
            exp->GenIR(peValue, no, isIV);
        }
        else
        {
            // 生成加载立即数的代码
            isIV = true;
            std::unique_ptr<NumberAST>(dynamic_cast<NumberAST *>(num.get()))->Num2IR(peValue);
        }
    }
};
typedef enum
{
    POSITIVE,
    NEGATIVE,
    NOT
} uop_t;
class UnaryOpAST : public BaseAST
{
public:
    uop_t uop;
    UnaryOpAST(uop_t _uop) : uop(_uop)
    {
    }
    void Dump() const override
    {
        string s1 = " UnaryOpAST生成的IR代码 ";
        cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
    }
};
typedef enum
{
    PEXP,
    UOUEXP
} pexp_uouexp_t;

class UnaryExpAST : public BaseAST
{
public:
    pexp_uouexp_t tag;
    std::unique_ptr<BaseAST> pexp;
    std::unique_ptr<BaseAST> unaryOp;
    std::unique_ptr<BaseAST> unaryExp;
    UnaryExpAST(pexp_uouexp_t _tag, std::unique_ptr<BaseAST> &_pexp) : tag(_tag), pexp(std::move(_pexp))
    {
    }
    UnaryExpAST(pexp_uouexp_t _tag, std::unique_ptr<BaseAST> &_unaryOp, std::unique_ptr<BaseAST> &_unaryExp) : tag(_tag), unaryOp(std::move(_unaryOp)), unaryExp(std::move(_unaryExp))
    {
    }
    void Dump() const override
    {
        if (tag == PEXP)
        {
            pexp->Dump();
        }
        else
        {
            unaryOp->Dump();
            unaryExp->Dump();
        }
        string s1 = " UnaryExpAST生成的IR代码 ";
        cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
    }
    void GenIR(string &pv, int *no, bool &isIV) const override
    {
        this->UExp2IR(pv, no, isIV);
    }
    void UExp2IR(string &uexpValue, int *no, bool &isIV) const
    {
        if (tag == PEXP)
        {
            // 基本表达式的代码生成
            // std::unique_ptr<PrimaryExpAST>(dynamic_cast<PrimaryExpAST *>(pexp.get()))->PE2IR(uexpValue, no, isIV);
            pexp->GenIR(uexpValue, no, isIV);
        }
        else
        {
            //+-!运算符连接一元云表达式的代码生成
            // string res="%i=";
            auto op = std::unique_ptr<UnaryOpAST>(dynamic_cast<UnaryOpAST *>(unaryOp.get()));
            // auto uep = std::unique_ptr<UnaryExpAST>(dynamic_cast<UnaryExpAST *>(unaryExp.get()));
            unaryExp->GenIR(uexpValue, no, isIV);
            // uep->UExp2IR(uexpValue, no, isIV);
            string res = "";
            switch (op->uop)
            {
            case NEGATIVE:
                if (isIV)
                {
                    // res = "%" + (*no + 1) + "=sub 0, " + uexpValue + "\n";
                    res += "  %";
                    res += to_string(*no);
                    res += " = sub 0, ";
                    res += uexpValue;
                    res += "\n";
                }
                else
                {
                    // res = "%" + (*no + 1) + "=sub 0, %" + *no + "\n";
                    res += "  %";
                    res += to_string((*no) + 1);
                    res += " = sub 0, %";
                    res += to_string(*no);
                    res += "\n";
                    (*no)++;
                }
                isIV = false;
                // cout<<"no:"<<*no<<endl;
                BaseAST::ss << res;
                // cout << res;
                break;
            case NOT:
                if (isIV)
                {
                    // res = "%" + (*no + 1) + "=eq " + uexpValue + ", 0" + "\n";
                    res += "  %";
                    res += to_string(*no);
                    res += " = eq ";
                    res += uexpValue;
                    res += ", 0";
                    res += "\n";
                }
                else
                {
                    // res = "%" + (*no + 1) + "=eq %" + *no + ", 0" + "\n";
                    res += "  %";
                    res += to_string((*no) + 1);
                    res += " = eq %";
                    res += to_string(*no);
                    res += ", 0";
                    res += "\n";
                    (*no)++;
                }
                isIV = false;
                // cout<<"no:"<<*no<<endl;
                BaseAST::ss << res;
                // cout << res;
                break;
            default:
                break;
            }
        }
    }
};

typedef enum
{
    UEXP,
    UOPMEXP
} uexp_mopuexp_t;

class MulExpAST : public BaseAST
{
public:
    uexp_mopuexp_t tag;
    std::unique_ptr<BaseAST> unaryExp;
    std::unique_ptr<BaseAST> mulExp;
    char op;
    std::unique_ptr<BaseAST> opunaryExp;
    MulExpAST(uexp_mopuexp_t _tag, std::unique_ptr<BaseAST> &_unaryExp) : tag(_tag)
    {
        unaryExp = std::move(_unaryExp);
    }
    MulExpAST(uexp_mopuexp_t _tag, std::unique_ptr<BaseAST> &_mulExp, char _op, std::unique_ptr<BaseAST> &_opunaryExp) : tag(_tag), op(_op)
    {
        mulExp = std::move(_mulExp);
        opunaryExp = std::move(_opunaryExp);
    }
    void Dump() const {

    };

    void GenIR(string &peValue, int *no, bool &isIV) const
    {
        if (tag == UEXP)
        {
            // 乘法表达式只包含一个一元表达式
            unaryExp->GenIR(peValue, no, isIV);
        }
        else
        {
            // 乘法表达式是一个乘法表达式与一元表达式的乘除求余的值
            if (op == '*')
            {
                gen_ir_by_op(peValue, no, isIV, mulExp.get(), "mul", opunaryExp.get());
            }
            else if (op == '/')
            {
                gen_ir_by_op(peValue, no, isIV, mulExp.get(), "div", opunaryExp.get());
            }
            else
            { // 求余%
                gen_ir_by_op(peValue, no, isIV, mulExp.get(), "mod", opunaryExp.get());
            }
        }
    }
    void gen_ir_by_op(string &peValue, int *no, bool &isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            isIV=false;
        }
        else
        {
            res += op;
            res += " %";
            res += to_string(*no);
            res += ",";
            (*no)++;
        }
        rightExp->GenIR(peValue, no, isIV);
        if (isIV)
        {
            res += peValue;
            res += "\n";
            isIV=false;
        }
        else
        {
            res += "%";
            res += to_string(*no);
            res += "\n";
            (*no)++;
        }
        string h = "  %";
        h += to_string(*no);
        h += " = ";
        res = h + res;
        BaseAST::ss << res;
    }
};
typedef enum
{
    MULEXP,
    ADDMULEXP
} mul_addmul_t;
class AddExpAST : public BaseAST
{
public:
    mul_addmul_t tag;
    std::unique_ptr<BaseAST> mulExp;
    std::unique_ptr<BaseAST> addExp;
    char op;
    std::unique_ptr<BaseAST> opmulExp;
    AddExpAST(mul_addmul_t _tag, std::unique_ptr<BaseAST> &_mulExp) : tag(_tag)
    {
        mulExp = std::move(_mulExp);
    }
    AddExpAST(mul_addmul_t _tag, std::unique_ptr<BaseAST> &_addExp, char _op, std::unique_ptr<BaseAST> &_opmulExp) : tag(_tag), op(_op)
    {
        addExp = std::move(_addExp);
        opmulExp = std::move(_opmulExp);
    }
    void Dump() const {

    };
    void GenIR(string &peValue, int *no, bool &isIV) const
    {
        if (tag == MULEXP)
        {
            // 只包含一个乘法表达式
            mulExp->GenIR(peValue, no, isIV);
        }
        else
        {
            if (op == '+')
            {
                gen_ir_by_op(peValue, no, isIV, addExp.get(), "add", opmulExp.get());
            }
            else if (op == '-')
            {
                gen_ir_by_op(peValue, no, isIV, addExp.get(), "sub", opmulExp.get());
            }
        }
    }
    void gen_ir_by_op(string &peValue, int *no, bool &isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            isIV=false;
        }
        else
        {
            res += op;
            res += " %";
            res += to_string(*no);
            res += ",";
            (*no)++;
        }
        rightExp->GenIR(peValue, no, isIV);
        if (isIV)
        {
            res += peValue;
            res += "\n";
            isIV=false;
        }
        else
        {
            res += "%";
            res += to_string(*no);
            res += "\n";
            (*no)++;
        }
        string h = "  %";
        h += to_string(*no);
        h += " = ";
        res = h + res;
        BaseAST::ss << res;
    }
};
class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> addExp;
    ExpAST(std::unique_ptr<BaseAST> &_addExp)
    {
        addExp = std::move(_addExp);
    }
    void Dump() const override
    {
        addExp->Dump();
        string s1 = " ExpAST生成的IR代码 ";
        // cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->Exp2IR(peValue, no, isIV);
    }
    void Exp2IR(string &expValue, int *no, bool &isIV) const
    {
        // exp生成的代码
        //...
        // std::unique_ptr<UnaryExpAST>(dynamic_cast<UnaryExpAST *>(unaryExp.get()))->UExp2IR(expValue, no, isIV);
        addExp->GenIR(expValue, no, isIV);
    }
};
class StmtAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exp;
    StmtAST(std::unique_ptr<BaseAST> &_exp)
    {
        exp = std::move(_exp);
    }
    void Dump() const override
    {
        string s1 = " Stmt生成的IR代码 ";
        exp->Dump();
        // cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->Stmt2IR(peValue, no, isIV);
    }
    void Stmt2IR(string &expValue, int *no, bool &isIV) const
    {
        // std::unique_ptr<ExpAST>(dynamic_cast<ExpAST *>(exp.get()))->Exp2IR(expValue, no, isIV);
        exp->GenIR(expValue, no, isIV);
        string res = "";
        if (isIV)
        {
            res = "  ret ";
            res += expValue;
            res += "\n";
        }
        else
        {
            res = "  ret %";
            res += to_string(*no);
            res += "\n";
        }
        BaseAST::ss << res;
        // cout << res;
    }
};
class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override
    {
        string s1 = "%entry: \n";
        // std::cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
        stmt->Dump();
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->Block2IR(peValue, no, isIV);
    }
    void Block2IR(string &peValue, int *no, bool &isIV) const
    {
        string expValue = "";
        int no1 = 0;
        bool isIV1 = false;
        string entry = "%entry:\n";
        BaseAST::ss << entry;
        // cout<<entry;
        // std::unique_ptr<StmtAST>(dynamic_cast<StmtAST *>(stmt.get()))->Stmt2IR(expValue, &no, isIV);
        stmt->GenIR(peValue, &no1, isIV1);
    }
};
class FuncTypeAST : public BaseAST
{
public:
    std::string ft;
    void Dump() const override
    {
        std::cout << "FuncTypeAST { ";
        std::cout << this->ft;
        std::cout << " }";
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
    }
};
// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override
    {
        // string s1 = "fun @main(): i32 { \n";
        string s1 = "fun @" + ident + "():" + (std::unique_ptr<FuncTypeAST>(dynamic_cast<FuncTypeAST *>(func_type.get()))->ft) + "{ \n";
        // std::cout << s1;
        // BaseAST::ss << s1;
        *BaseAST::oir << s1;
        // func_type->Dump();
        // std::cout << ", " << ident << ", ";
        block->Dump();
        // std::cout << " }";
        // BaseAST::ss << " }";
        *BaseAST::oir << " }";
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->Fun2IR(peValue, no, isIV);
    }
    void Fun2IR(string &peValue, int *no, bool &isIV) const
    {
        // string s1 = "fun @" + ident + "():" + (std::unique_ptr<FuncTypeAST>(dynamic_cast<FuncTypeAST *>(func_type.get()))->ft) + "{ \n";
        string s1 = "fun @" + ident + "(): i32 { \n";
        BaseAST::ss << s1;
        // cout<<s1;
        // std::unique_ptr<BlockAST>(dynamic_cast<BlockAST *>(block.get()))->Block2IR();
        block->GenIR(peValue, no, isIV);
        BaseAST::ss << "}\n";
        // cout<<"}\n";
    }
};
// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override
    {
        // std::cout << "CompUnitAST { ";
        func_def->Dump();
        // std::cout << " }";
    }
    void GenIR(string &peValue, int *no, bool &isIV) const override
    {
        this->CU2IR(peValue, no, isIV);
    }
    void CU2IR(string &peValue, int *no, bool &isIV) const
    {
        // 完成编译单元的翻译工作
        //...
        // 调用函数的翻译单元
        // std::unique_ptr<FuncDefAST>(dynamic_cast<FuncDefAST *>(func_def.get()))->Fun2IR();
        func_def->GenIR(peValue, no, isIV);
    }
};
