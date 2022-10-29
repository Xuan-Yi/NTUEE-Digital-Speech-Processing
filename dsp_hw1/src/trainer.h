#ifndef TRAINER_H
#define TRAINER_H

#include "../inc/hmm.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "stdlib.h"
#include "vector"

using namespace std;

const int seq_num = 10000; // number of sequence

typedef struct
{
    double alpha[MAX_SEQ][MAX_STATE];              // forward variable, (T, state_num)
    double beta[MAX_SEQ][MAX_STATE];               // backward variable, (T, state_num)
    double gamma[MAX_SEQ][MAX_STATE];              // (T, state_num)
    double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE]; // (T, state_num, state_num)
} ParamSet;

class Trainer
{
private:
    int T;                    // sequence length
    int td[seq_num][MAX_SEQ]; // training data
    HMM *hmm;
    vector<ParamSet> params; // params for n-th sequence

    void Compute_Params(int n); // calculate alpha, beta for n-th sequence
    bool Load_Train_Data(char *filename);

public:
    Trainer(char *filename, HMM *_hmm);
    void Update_HMM();
};

#endif