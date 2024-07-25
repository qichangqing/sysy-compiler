#pragma once
#include <fstream> 
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>
#include "koopa.h"


using namespace std;
//,const std::string& filename, std::ios_base::openmode mode
void transformToRaw(const char * str,string output, std::ios_base::openmode mode);