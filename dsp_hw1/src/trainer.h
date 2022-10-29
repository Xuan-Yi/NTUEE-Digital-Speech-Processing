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

typedef struct
{
    double **alpha;    // forward variable, (state_num, state_num)
    double **beta;     // backward variable, (state_num, state_num)
    double **gamma;    // (T, state_num)
    double ***epsilon; // (T, state_num, state_num)
} ParamSet;

class Trainer
{
private:
    int T;       // sequence length
    int seq_num; // number of sequences
    int **td;    // training data
    HMM *hmm;
    vector<ParamSet> params; // params for n-th sequence

    void Compute_Params(int n); // calculate alpha, beta for n-th sequence
    bool Load_Train_Data(char *filename);

public:
    Trainer(char *filename, HMM *_hmm);
    ~Trainer();
    void Update_HMM();
};

#endif