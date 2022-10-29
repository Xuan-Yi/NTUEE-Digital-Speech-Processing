#ifndef VITERBI_H
#define VITERBI_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "../inc/hmm.h"

using namespace std;

typedef struct
{
    int P;             // index of expected state
    double likelihood; // likelihood of expected state
    int best_model;    // optional, mark the index of model with the moss likelihood
} result_pair;

class Viterbi
{
private:
    static const int HMM_NUM = 5;
    int T;                    // sequence length
    int seq_num;              // number of sequences
    int **td;                 // testing data
    HMM hmms[5];              // list of HMM
    vector<double **> deltas; // [HMM_NUM](T, state_num)
    bool Load_Test_Data(char *filename);

public:
    Viterbi(char *filename, char *listname);
    ~Viterbi();
    int get_seq_num();
    result_pair Do_Viterbi(int n);
};

#endif