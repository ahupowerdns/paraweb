#pragma once
#include <stdio.h>
#include <string>

int schdir(const char* path);
int ssystem(const char* path);
std::string getStringFromFP(FILE* fp);
