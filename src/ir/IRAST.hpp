#pragma once
#include <iostream>
#include <string>
#include <list>

using namespace std;

public class BaseIRAST{
    public:
        virtual ~BaseIRAST() = default;

        virtual void Dump() const = 0;
}
/* 
    FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
    FunParams ::= SYMBOL ":" Type {"," SYMBOL ":" Type};
    FunBody ::= {Block};
    Block ::= SYMBOL ":" {Statement} EndStatement;
    Statement ::= SymbolDef | Store | FunCall;
    EndStatement ::= Branch | Jump | Return;
 */
class FunIRAST : public BaseIRAST{
    public:
        BlockIRAST block;
        void Dump() const override {
            cout << "fun @main:i32{";
            block->Dump();
            cout << " }";
        }
};

class BlockIRAST : public BaseIRAST{
    public:
        ReturnIRAST r;
        void Dump() const override {
            cout << "%entry:";
            r->Dump();
        }
};

class ReturnIRAST : public BaseIRAST{
    public:
        int rval;
        void Dump() const override {
            cout << "ret "<<rval;
        }
};


