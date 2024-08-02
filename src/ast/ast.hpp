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
    virtual void GenIR(string &peValue, int *no, int *isIV) const = 0;
    // 保存生成的ir字符串序列
    static stringstream ss;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        func_def->GenIR(peValue, no, isIV);
    }
};
class FuncTypeAST : public BaseAST
{
public:
    std::string ft;

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        peValue = ft;
    }
};
// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        string ft = "";
        func_type->GenIR(peValue, no, isIV);
        if (peValue == "int")
        {
            ft = ": i32";
        }
        else
        {
            ft = " ";
        }
        string res = "fun @" + ident + "()" + ft + " { \n";
        // cout << res;
        BaseAST::ss<<res;
        // std::unique_ptr<BlockAST>(dynamic_cast<BlockAST *>(block.get()))->Block2IR();
        block->GenIR(peValue, no, isIV);
        // cout << "}\n";
        BaseAST::ss<<"}\n";
    }
};

class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        string entry = "%entry:\n";
        // cout<<entry;
        BaseAST::ss << entry;
        stmt->GenIR(peValue, no, isIV);
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

    void GenIR(string &peValue, int *no, int *isIV) const override
    {

        exp->GenIR(peValue, no, isIV);
        string res = "";
        if (*isIV)
        {
            res = "  ret ";
            res += peValue;
            res += "\n";
        }
        else
        {
            res = "  ret %";
            res += to_string(*no);
            res += "\n";
        }
        // cout << res;
        BaseAST::ss<<res;
    }
};
class NumberAST : public BaseAST
{
public:
    int number;

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        peValue = to_string(number);
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

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "PrimaryExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        this->PE2IR(peValue, no, isIV);
    }
    void PE2IR(string &peValue, int *no, int *isIV) const
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
            *isIV = 1;
            num->GenIR(peValue, no, isIV);
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

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "UnaryOpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        switch (uop)
        {
        case POSITIVE:
            peValue = "+";
            break;
        case NEGATIVE:
            peValue = "-";
            break;
        case NOT:
            peValue = "!";
            break;
        default:
            break;
        }
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

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "UnaryExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        this->UExp2IR(peValue, no, isIV);
    }
    void UExp2IR(string &uexpValue, int *no, int *isIV) const
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
            unaryOp->GenIR(uexpValue, no, isIV);
            string uop = uexpValue;
            // auto uep = std::unique_ptr<UnaryExpAST>(dynamic_cast<UnaryExpAST *>(unaryExp.get()));
            unaryExp->GenIR(uexpValue, no, isIV);
            // uep->UExp2IR(uexpValue, no, isIV);
            string res = "";
            if (uop == "-")
            {
                if (*isIV)
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
                *isIV = 0;
                // cout<<"no:"<<*no<<endl;
                // cout << res;
                BaseAST::ss<<res;
            }
            else if (uop == "!")
            {
                if (*isIV)
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
                *isIV = 0;
                // cout<<"no:"<<*no<<endl;
                // cout << res;
                BaseAST::ss<<res;
            }
        }
    }
};

class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> unaryExp;
    ExpAST(std::unique_ptr<BaseAST> &_unaryExp)
    {
        unaryExp = std::move(_unaryExp);
    }

    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "ExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        this->Exp2IR(peValue, no, isIV);
    }
    void Exp2IR(string &expValue, int *no, int *isIV) const
    {
        // exp生成的代码
        //...
        // std::unique_ptr<UnaryExpAST>(dynamic_cast<UnaryExpAST *>(unaryExp.get()))->UExp2IR(expValue, no, isIV);
        unaryExp->GenIR(expValue, no, isIV);
    }
};
