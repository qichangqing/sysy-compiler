#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <sstream>
#include <stack>

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

    void GenIR(string &peValue, int *no, int *isIV) const
    {
        // cout << "MulExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
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
    void gen_ir_by_op(string &peValue, int *no, int *isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (*isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            *isIV = 0;
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
        if (*isIV)
        {
            res += peValue;
            res += "\n";
            *isIV = 0;
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
    void GenIR(string &peValue, int *no, int *isIV) const
    {
        // cout << "AddExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
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
    void gen_ir_by_op(string &peValue, int *no, int *isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (*isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            *isIV = 0;
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
        if (*isIV)
        {
            res += peValue;
            res += "\n";
            *isIV = 0;
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
    ADD_EXP,
    REL_OP_ADD_EXP
} rel_type;
class RelExpAST : public BaseAST
{
public:
    rel_type tag;
    std::unique_ptr<BaseAST> addExp;
    std::unique_ptr<BaseAST> relExp;
    string op;
    std::unique_ptr<BaseAST> opAddExp;

    RelExpAST(rel_type _tag, std::unique_ptr<BaseAST> &_addExp) : tag(_tag)
    {
        addExp = std::move(_addExp);
    }
    RelExpAST(rel_type _tag, std::unique_ptr<BaseAST> &_relExp, string _op, std::unique_ptr<BaseAST> &_opAddExp) : tag(_tag), op(_op)
    {
        relExp = std::move(_relExp);
        opAddExp = std::move(_opAddExp);
    }
    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "RelExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        if (tag == ADD_EXP)
        {
            // 只包含一个乘法表达式
            addExp->GenIR(peValue, no, isIV);
        }
        else
        {
            if (op == ">")
            {
                gen_ir_by_op(peValue, no, isIV, relExp.get(), "gt", opAddExp.get());
            }
            else if (op == "<")
            {
                gen_ir_by_op(peValue, no, isIV, relExp.get(), "lt", opAddExp.get());
            }
            else if (op == ">=")
            {
                gen_ir_by_op(peValue, no, isIV, relExp.get(), "ge", opAddExp.get());
            }
            else if (op == "<=")
            {
                gen_ir_by_op(peValue, no, isIV, relExp.get(), "le", opAddExp.get());
            }
        }
    }
    void gen_ir_by_op(string &peValue, int *no, int *isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (*isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            *isIV = 0;
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
        if (*isIV)
        {
            res += peValue;
            res += "\n";
            *isIV = 0;
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
    RELEXP,
    EQ_OP_RELEXP
} eq_type;
class EqExpAST : public BaseAST
{
public:
    eq_type tag;
    std::unique_ptr<BaseAST> relExp;
    std::unique_ptr<BaseAST> eqExp;
    string op;
    std::unique_ptr<BaseAST> opRelExp;

    EqExpAST(eq_type _tag, std::unique_ptr<BaseAST> &_relExp) : tag(_tag)
    {
        relExp = std::move(_relExp);
    }
    EqExpAST(eq_type _tag, std::unique_ptr<BaseAST> &_eqExp, string _op, std::unique_ptr<BaseAST> &_opRelExp) : tag(_tag), op(_op)
    {
        eqExp = std::move(_eqExp);
        opRelExp = std::move(_opRelExp);
    }
    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "EqExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        if (tag == RELEXP)
        {
            // 只包含一个乘法表达式
            relExp->GenIR(peValue, no, isIV);
        }
        else
        {
            if (op == "==")
            {
                gen_ir_by_op(peValue, no, isIV, eqExp.get(), "eq", opRelExp.get());
            }
            else if (op == "!=")
            {
                gen_ir_by_op(peValue, no, isIV, eqExp.get(), "ne", opRelExp.get());
            }
        }
    }
    void gen_ir_by_op(string &peValue, int *no, int *isIV, BaseAST *leftExp, string op, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        if (*isIV)
        {
            res += op;
            res += " ";
            res += peValue;
            res += ",";
            *isIV = 0;
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
        if (*isIV)
        {
            res += peValue;
            res += "\n";
            *isIV = 0;
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
    EQEXP,
    LANDEXP_AND_EQEXP
} landexp_type;
class LAndExpAST : public BaseAST
{
public:
    landexp_type tag;
    std::unique_ptr<BaseAST> eqExp;
    std::unique_ptr<BaseAST> lAndExp;
    string op;
    std::unique_ptr<BaseAST> opEqExp;

    LAndExpAST(landexp_type _tag, std::unique_ptr<BaseAST> &_eqExp) : tag(_tag)
    {
        eqExp = std::move(_eqExp);
    }
    LAndExpAST(landexp_type _tag, std::unique_ptr<BaseAST> &_lAndExp, string _op, std::unique_ptr<BaseAST> &_opEqExp) : tag(_tag), op(_op)
    {
        lAndExp = std::move(_lAndExp);
        opEqExp = std::move(_opEqExp);
    }
    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "LAndExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        if (tag == EQEXP)
        {
            // 只包含一个乘法表达式
            eqExp->GenIR(peValue, no, isIV);
        }
        else
        {
            // 实现逻辑与
            gen_ir_from_land(peValue, no, isIV, lAndExp.get(),  opEqExp.get());
        }
    }
    void gen_ir_from_land(string &peValue, int *no, int *isIV, BaseAST *leftExp, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        stack<int> s_land;
        if (*isIV)
        {
            res+="  %";
            res+=to_string(*no);
            s_land.push(*no);
            res+=" = ne ";
            res += peValue;
            res += ",";
            res+="0\n";
            (*no)++;
            *isIV = 0;
        }
        else
        {
            res += "  %";
            res += to_string((*no)+1);
            s_land.push((*no)+1);
            res+=" = ne %";
            res += to_string(*no);
            res += ",";
            res+="0\n";
            *no=(*no)+2;
        }
        BaseAST::ss<<res;
        res="";
        rightExp->GenIR(peValue, no, isIV);
        if (*isIV)
        {
            res+="  %";
            res+=to_string(*no);
            s_land.push(*no);
            res+=" = ne ";
            res += peValue;
            res += ",";
            res+="0\n";
            (*no)++;
            *isIV = 0;
        }
        else
        {
            res += "  %";
            res += to_string((*no)+1);
            s_land.push((*no)+1);
            res+=" = ne %";
            res += to_string(*no);
            res += ",";
            res+="0\n";
            *no=(*no)+2;
        }
        string num2=to_string(s_land.top());
        s_land.pop();
        string num1=to_string(s_land.top());
        s_land.pop();
        res+="  %";
        res+=to_string(*no);
        res+=" = and %";
        res+=num1;
        res+=", %";
        res+=num2;
        res+="\n";
        BaseAST::ss << res;
    }
};
typedef enum
{
    LANDEXP,
    LOREXP_OR_LANDEXP
} lorexp_type;
class LOrExpAST : public BaseAST
{
public:
    lorexp_type tag;
    std::unique_ptr<BaseAST> lAndExp;
    std::unique_ptr<BaseAST> lOrExp;
    string op;
    std::unique_ptr<BaseAST> opLAndExp;

    LOrExpAST(lorexp_type _tag, std::unique_ptr<BaseAST> &_lAndExp) : tag(_tag)
    {
        lAndExp = std::move(_lAndExp);
    }
    LOrExpAST(lorexp_type _tag, std::unique_ptr<BaseAST> &_lOrExp, string _op, std::unique_ptr<BaseAST> &_opLAndExp) : tag(_tag), op(_op)
    {
        lOrExp = std::move(_lOrExp);
        opLAndExp = std::move(_opLAndExp);
    }
    void GenIR(string &peValue, int *no, int *isIV) const override
    {
        // cout << "LOrExpAST---" << peValue<<"--" << *no <<"--"<< *isIV << endl;
        if (tag == LANDEXP)
        {
            // 只包含一个逻辑与
            lAndExp->GenIR(peValue, no, isIV);
        }
        else
        {
            // 实现逻辑或
            gen_ir_from_lor(peValue, no, isIV, lOrExp.get(), opLAndExp.get());
        }
    }
    void gen_ir_from_lor(string &peValue, int *no, int *isIV, BaseAST *leftExp, BaseAST *rightExp) const
    {
        leftExp->GenIR(peValue, no, isIV);
        string res = "";
        stack<int> s_lor;
        if (*isIV)
        {
            res+="  %";
            res+=to_string(*no);
            s_lor.push(*no);
            res+=" = ne ";
            res += peValue;
            res += ",";
            res+="0\n";
            (*no)++;
            *isIV = 0;
        }
        else
        {
            res += "  %";
            res += to_string((*no)+1);
            s_lor.push((*no)+1);
            res+=" = ne %";
            res += to_string(*no);
            res += ",";
            res+="0\n";
            *no=(*no)+2;
        }
        BaseAST::ss<<res;
        res="";
        rightExp->GenIR(peValue, no, isIV);
        if (*isIV)
        {
            res+="  %";
            res+=to_string(*no);
            s_lor.push(*no);
            res+=" = ne ";
            res += peValue;
            res += ",";
            res+="0\n";
            (*no)++;
            *isIV = 0;
        }
        else
        {
            res += "  %";
            res += to_string((*no)+1);
            s_lor.push((*no)+1);
            res+=" = ne %";
            res += to_string(*no);
            res += ",";
            res+="0\n";
            *no=(*no)+2;
        }
        string num2=to_string(s_lor.top());
        s_lor.pop();
        string num1=to_string(s_lor.top());
        s_lor.pop();
        res+="  %";
        res+=to_string(*no);
        res+=" = or %";
        res+=num1;
        res+=", %";
        res+=num2;
        res+="\n";
        BaseAST::ss << res;
    }
};

class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lOrExp;
    ExpAST(std::unique_ptr<BaseAST> &_lOrExp)
    {
        lOrExp = std::move(_lOrExp);
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
        lOrExp->GenIR(expValue, no, isIV);
    }
};
