#pragma once
#include <fstream> 
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>
#include <sstream>
#include <stack>
#include "koopa.h"


using namespace std;

void Compiler_IR2RISCV(const char *str,const char* output);