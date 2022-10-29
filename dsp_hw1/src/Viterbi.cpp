#include "Viterbi.h"

Viterbi::Viterbi(char *filename, char *listname)
{
    // load hmms
    load_models(listname, hmms, HMM_NUM);

    // load test data
    if (Load_Test_Data(filename))
    {
        // allocate delta
        for (int k = 0; k < HMM_NUM; k++) // k-th model
        {
            double **delta = new double *[T];
            for (int t = 0; t < T; t++)
                delta[t] = new double[hmms[k].state_num];
            deltas.push_back(delta);
        }
    }
    else
    {
        cout << "failed to allocate arrays\n";
        exit(1); // end program
    }
};

Viterbi::~Viterbi()
{
    for (int n = 0; n < HMM_NUM; n++)
    {
        // deallocate delta
        for (int t = 0; t < T; t++)
            delete[] deltas[n][t];
        delete[] deltas[n];
    }
    // deallocate training data
    for (int t = 0; t < T; t++)
        delete[] td[t];
    delete td;
};

bool Viterbi::Load_Test_Data(char *filename)
{
    vector<string> fr;
    fstream f;
    f.open(filename, ios::in);

    if (!f.is_open())
    {
        cout << "failed to open: " << filename << "\n";
        return false;
    }

    string line;

    fr.clear();
    seq_num = 1;
    while (getline(f, line))
        fr.push_back(line);
    T = fr[0].length();
    seq_num = fr.size();
    f.close();

    // allocate test data
    td = new int *[seq_num];
    for (int n = 0; n < seq_num; n++)
        td[n] = new int[T];

    // turn all characters (A~F) into state index (0~5)
    for (int n = 0; n < seq_num; n++)
    {
        for (int t = 0; t < T; t++)
            td[n][t] = int(fr[n][t] - 'A');
    }
    cout << "testing data loaded successfully\n";
    return true;
};

result_pair Viterbi::Do_Viterbi(int n)
{
    // do Victerbi to each model
    result_pair results[HMM_NUM]; // result for each HMM

    for (int k = 0; k < HMM_NUM; k++)
    {
        HMM *hmm = &hmms[k];
        result_pair *result = &results[k];

        for (int i = 0; i < hmm->state_num; i++)
            deltas[k][0][i] = hmm->initial[i] * hmm->observation[i][td[n][0]];
        for (int t = 1; t < T; t++)
        {
            for (int j = 0; j < hmm->state_num; j++)
            {
                double max = 0;
                for (int i = 0; i < hmm->state_num; i++)
                {
                    double candidate = deltas[k][t - 1][i] * hmm->transition[i][j];
                    if (candidate > max)
                    {
                        max = candidate;
                        result->P = i;
                    }
                }
                deltas[k][t][j] = max * hmm->observation[j][td[n][t]];
            }
        }
        result->likelihood = deltas[k][T - 1][result->P];
    }

    // find the model with most likelihood
    result_pair rots; // result of best expected model
    rots.best_model = 0;

    for (int k = 0; k < HMM_NUM; k++)
    {
        if (results[k].likelihood > results[rots.best_model].likelihood)
            rots.best_model = k;
    }
    rots.P = results[rots.best_model].P;
    rots.likelihood = results[rots.best_model].likelihood;

    return rots;
};

int Viterbi::get_seq_num()
{
    return seq_num;
}