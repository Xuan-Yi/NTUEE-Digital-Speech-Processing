#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include "../inc/mydisambig.h"

using namespace std;

int main(int argc, char *argv[])
{
    const char *fin_path = argv[1];
    const char *zhuyin_big5_path = argv[2];
    const char *lm_path = argv[3];
    const char *fout_path = argv[4];
    bool tri = false;
    if (argc == 6 && strcmp("--tri", argv[5]) == 0)
        tri = true;
    Viterbi viterbi(lm_path, zhuyin_big5_path, tri);
    viterbi.do_Viterbi(fin_path, fout_path);

    return 0;
}