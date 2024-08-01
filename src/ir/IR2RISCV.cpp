#include "IR2RISCV.hpp"

stringstream ss_riscv;
string tt[7] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};
bool tt_inuse[7] = {false, false, false, false, false, false, false};
int it = 0;
string ta[8] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
bool ta_inuse[8] = {false, false, false, false, false, false, false, false};
int ia = 0;
stack<string> op_num_stack;
void travel_raw_slice(koopa_raw_slice_t slice);
void Compiler_IR2RISCV(const char *str, const char *output)
{
    it = ia = 0;
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS); // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);
    // 处理 raw program
    // travel_raw_slice(raw.values);
    it = ia = 0;
    ss_riscv << "  .text\n";
    travel_raw_slice(raw.funcs);
    ofstream ofile(output);
    string res = ss_riscv.str();
    ofile << res;
    // cout << res;
    // 使用 for 循环遍历函数列表
    // ss_riscv << "  .text" << endl;
    // for (size_t i = 0; i < raw.funcs.len; ++i)
    // {
    //     // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
    //     // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
    //     assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
    //     // 获取当前函数
    //     koopa_raw_function_t func = (koopa_raw_function_t)raw.funcs.buffer[i];
    //     // 进一步处理当前函数
    //     // ...
    //     cout << "transform:========" << func->name << endl;
    //     if (strcmp(func->name, "@main") == 0)
    //     {
    //         *ORiscV << "  .globl main" << endl;
    //     }
    //     *ORiscV << (func->name + 1) << ":" << endl;
    //     for (size_t j = 0; j < func->bbs.len; ++j)
    //     {
    //         assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    //         koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[j];
    //         // 进一步处理当前基本块
    //         // ...
    //         koopa_raw_slice_t insts = bb->insts;
    //         cout << "insts:=====" << insts.kind << endl;
    //         cout << "insts:=====" << insts.len << endl;
    //         cout << "insts:=====" << insts.buffer << endl;
    //         koopa_raw_value_data *rv = (koopa_raw_value_data *)insts.buffer[0];
    //         if (rv->kind.tag == KOOPA_RVT_RETURN)
    //         {
    //             cout << "tag:=====return" << rv->kind.tag << endl;
    //             cout << "tag:=====return" << KOOPA_RVT_RETURN << endl;
    //             koopa_raw_value_t val = rv->kind.data.ret.value;
    //             int v = val->kind.data.integer.value;
    //             cout << "tag:=====retvalue:" << v << endl;
    //             *ORiscV << "  li a0, " << v << endl;
    //             *ORiscV << "  ret" << endl;
    //         }
    //     }
    // }
}
void gen_risc_from_func(koopa_raw_function_t func)
{
    // 一些相关操作
    //...
    string name = func->name + 1;
    ss_riscv << "  .globl " << name << "\n";
    ss_riscv << name << ":\n";
    travel_raw_slice(func->bbs);
}
void gen_risc_from_block(koopa_raw_basic_block_t block)
{
    travel_raw_slice(block->insts);
}
void gen_alexp(koopa_raw_binary_t bi, string op)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    // if(op=="mul"){
    //     cout<<num1<<" "<<num2<<endl;
    // }
    op_num_stack.pop();
    res += "  " + op + "   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    ss_riscv << res;
    // cout<<res;
    it = (it + 1) % 7;
}
void gen_riscv_lt(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  slt   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_gt(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  slt   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num2;
    res += ", ";
    res += num1;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_le(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  slt   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num2;
    res += ", ";
    res += num1;
    res += "\n";
    string cr = op_num_stack.top();
    op_num_stack.pop();
    res += "  xori   ";
    res += cr;
    op_num_stack.push(cr);
    res += ", ";
    res += "1";
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_ge(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  slt   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    string cr = op_num_stack.top();
    op_num_stack.pop();
    res += "  xori   ";
    res += cr;
    op_num_stack.push(cr);
    res += ", ";
    res += "1";
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_eq(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  xor   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    string xorr = op_num_stack.top();
    op_num_stack.pop();
    res += "  seqz   ";
    res += xorr;
    op_num_stack.push(xorr);
    res += ", ";
    res += xorr;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_not_eq(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  xor   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    string xorr = op_num_stack.top();
    op_num_stack.pop();
    res += "  snez   ";
    res += xorr;
    op_num_stack.push(xorr);
    res += ", ";
    res += xorr;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_and(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  snez   ";
    res += num1;
    op_num_stack.push(num1);
    res += ", ";
    res += num1;
    res += "\n";
    res += "  snez   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num2;
    res += "\n";
    num2 = op_num_stack.top();
    op_num_stack.pop();
    num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  and   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}
void gen_riscv_or(koopa_raw_binary_t bi)
{
    // cout<<op<<endl;
    string res = "";
    bool l_is_int = false;
    bool r_is_int = false;
    if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.lhs->kind.data.integer.value);
        res += "\n";
        l_is_int = true;
    }
    if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (l_is_int)
        {
            it = (it + 1) % 7;
        }
        res += "  li    ";
        res += tt[it];
        op_num_stack.push(tt[it]);
        res += ", ";
        res += to_string(bi.rhs->kind.data.integer.value);
        res += "\n";
        r_is_int = true;
    }
    string num2 = op_num_stack.top();
    op_num_stack.pop();
    string num1 = op_num_stack.top();
    op_num_stack.pop();
    res += "  or   ";
    res += num2;
    op_num_stack.push(num2);
    res += ", ";
    res += num1;
    res += ", ";
    res += num2;
    res += "\n";
    string orr = op_num_stack.top();
    op_num_stack.pop();
    res += "  snez   ";
    res += orr;
    op_num_stack.push(orr);
    res += ", ";
    res += orr;
    res += "\n";
    ss_riscv << res;
    it = (it + 1) % 7;
}

void gen_risc_from_inst(koopa_raw_value_t inst)
{
    if (inst->kind.tag == KOOPA_RVT_BINARY)
    {
        koopa_raw_binary_t bi = inst->kind.data.binary;
        // switch (bi.op)
        // {
        // case KOOPA_RBO_EQ:
        if (bi.op == KOOPA_RBO_EQ)
        {
            // cout << "eqlrtag: " << bi.lhs->kind.tag << " " << bi.rhs->kind.tag << endl;
            // // cout << "binaryAddr:" << &bi << endl;
            // cout << "binaryAddr: " << &bi << " 操作符: " << bi.op << " 左操作数：" << (bi.lhs->kind.data.integer.value) << " 右操作数：" << (bi.rhs->kind.data.integer.value) << endl;
            // 左操作数是立即数
            if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER && bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
            {
                int li = bi.lhs->kind.data.integer.value;
                int ri = bi.rhs->kind.data.integer.value;
                if (ri == 0)
                {
                    while (tt_inuse[it])
                    {
                        it = (it + 1) % 7;
                    }
                    string res = "  li    ";
                    res += tt[it];
                    res += ", ";
                    res += to_string(li);
                    res += "\n";
                    res += "  xor   ";
                    res += tt[it];
                    res += ", ";
                    res += tt[it];
                    res += ", ";
                    res += "x0\n";
                    res += "  seqz  ";
                    res += tt[it];
                    res += ", ";
                    res += tt[it];
                    res += "\n";
                    tt_inuse[it] = true;
                    op_num_stack.push(tt[it]);
                    // cout<<res<<endl;
                    ss_riscv << res;
                }
                else
                {
                    gen_riscv_eq(bi);
                }
            }
            else if (bi.lhs->kind.tag == KOOPA_RVT_BINARY && bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
            { // 左操作数是上一次操作的结果
                string res = "  xor   ";
                res += tt[it];
                res += ", ";
                res += tt[it];
                res += ", ";
                res += "x0\n";
                res += "  seqz  ";
                res += tt[it];
                res += ", ";
                res += tt[it];
                res += "\n";
                tt_inuse[it] = true;
                op_num_stack.push(tt[it]);
                ss_riscv << res;
            }
            else
            {
                gen_riscv_eq(bi);
            }
        }
        else if (bi.op == KOOPA_RBO_SUB)
        {
            // cout<<"sub\n";
            string res = "";
            bool l_is_z = false;
            bool t_inuse = false;
            if (bi.lhs->kind.tag == KOOPA_RVT_INTEGER)
            {
                int li = bi.lhs->kind.data.integer.value;
                if (li == 0)
                {
                    l_is_z = true;
                }
                else
                {
                    res += "  li    ";
                    res += tt[it];
                    op_num_stack.push(tt[it]);
                    res += ", ";
                    res += to_string(li);
                    res += "\n";
                    t_inuse = true;
                }
            }
            // 需要load立即数
            if (bi.rhs->kind.tag == KOOPA_RVT_INTEGER)
            {
                res += "  li    ";
                if (t_inuse == true)
                {
                    it = (it + 1) % 7;
                }
                res += tt[it];
                op_num_stack.push(tt[it]);
                res += ", ";
                res += to_string(bi.rhs->kind.data.integer.value);
                res += "\n";
            }
            string num2 = "";
            string num1 = "";
            res += "  sub   ";
            int i = (it + 1) % 7;
            if (l_is_z)
            {
                num1 = "x0";
                num2 = op_num_stack.top();
                op_num_stack.pop();
            }
            else
            {
                num2 = op_num_stack.top();
                op_num_stack.pop();
                num1 = op_num_stack.top();
                op_num_stack.pop();
            }
            res += tt[i];
            op_num_stack.push(tt[i]);
            res += ", ";
            res += num1;
            res += ", ";
            res += num2;
            res += "\n";
            it = (it + 1) % 7;
            // cout<<res;
            ss_riscv << res;
        }
        else if (bi.op == KOOPA_RBO_ADD)
        {
            gen_alexp(bi, "add");
        }
        else if (bi.op == KOOPA_RBO_MUL)
        {
            gen_alexp(bi, "mul");
        }
        else if (bi.op == KOOPA_RBO_DIV)
        {
            gen_alexp(bi, "div");
        }
        else if (bi.op == KOOPA_RBO_MOD)
        {
            gen_alexp(bi, "mod");
        }
        else if (bi.op == KOOPA_RBO_LT)
        {
            /* code */
            gen_riscv_lt(bi);
        }
        else if (bi.op == KOOPA_RBO_GT)
        {
            gen_riscv_gt(bi);
        }
        else if (bi.op == KOOPA_RBO_LE)
        {
            gen_riscv_le(bi);
        }
        else if (bi.op == KOOPA_RBO_GE)
        {
            gen_riscv_ge(bi);
        }
        else if (bi.op == KOOPA_RBO_NOT_EQ)
        {
            gen_riscv_not_eq(bi);
        }
        else if (bi.op == KOOPA_RBO_AND)
        {
            gen_riscv_and(bi);
        }
        else if (bi.op == KOOPA_RBO_OR)
        {
            gen_riscv_or(bi);
        }
    }
    else if (inst->kind.tag == KOOPA_RVT_RETURN)
    {
        koopa_raw_return_t ret = inst->kind.data.ret;
        if (ret.value->kind.tag == KOOPA_RVT_BINARY)
        {
            // cout << "returnValue: BINARY\n";
            string res = "  mv    a0, ";
            res += op_num_stack.top();
            op_num_stack.pop();
            res += "\n";
            res += "  ret\n";
            ss_riscv << res;
        }
        else
        {
            int ri=ret.value->kind.data.integer.value;
            string res = "";
            res += "  li    ";
            res += "a0, ";
            res += to_string(ri);
            res += "\n";
            res += "  ret\n";
            ss_riscv<<res;
        }
    }
    else
    {
    }
}
void travel_raw_slice(koopa_raw_slice_t slice)
{
    switch (slice.kind)
    {
    case KOOPA_RSIK_FUNCTION:
        for (int i = 0; i < slice.len; i++)
        {
            koopa_raw_function_t fun = (koopa_raw_function_t)slice.buffer[i];
            gen_risc_from_func(fun);
        }
        break;
    case KOOPA_RSIK_BASIC_BLOCK:
        for (int i = 0; i < slice.len; i++)
        {
            koopa_raw_basic_block_t block = (koopa_raw_basic_block_t)slice.buffer[i];
            gen_risc_from_block(block);
        }
        break;
    case KOOPA_RSIK_VALUE:
        for (int i = 0; i < slice.len; i++)
        {
            koopa_raw_value_t inst = (koopa_raw_value_t)slice.buffer[i];
            gen_risc_from_inst(inst);
        }
        break;
    default:
        break;
    }
}
