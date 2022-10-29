#include "../inc/trainer.h"

Trainer::Trainer(char *filename, HMM *_hmm)
{
    hmm = _hmm;
    params.reserve(seq_num);

    if (!Load_Train_Data(filename))
        exit(1);
}

bool Trainer::Load_Train_Data(char *filename)
{
    vector<string> fr;
    ifstream f(filename);

    if (!f.is_open())
    {
        cout << "failed to open: " << filename << "\n";
        return false;
    }

    string line;

    fr.clear();
    while (getline(f, line))
        fr.push_back(line);
    f.close();

    T = fr[0].length();

    // load train data
    for (int n = 0; n < seq_num; n++)
    {
        for (int t = 0; t < T; t++)
        {
            td[n][t] = int(fr[n][t] - 'A');
        }
    }
    return true;
};

void Trainer::Compute_Params(int n)
{
    ParamSet *ps = &(params[n]);

    // calculate alpha (forward variable)
    for (int i = 0; i < hmm->state_num; i++)
        ps->alpha[0][i] = hmm->initial[i] * hmm->observation[i][td[n][0]];
    for (int t = 0; t < T - 1; t++)
    {
        for (int j = 0; j < hmm->state_num; j++)
        {
            ps->alpha[t + 1][j] = 0;
            for (int i = 0; i < hmm->state_num; i++)
                ps->alpha[t + 1][j] += ps->alpha[t][i] * hmm->transition[i][j];
            ps->alpha[t + 1][j] *= hmm->observation[j][td[n][t + 1]];
        }
    }

    // calculate P[ observation | model ] = prob_observ
    double prob_observ = 0;
    for (int i = 0; i < hmm->state_num; i++)
        prob_observ += ps->alpha[T - 1][i];

    // calculate beta (backward variable)
    for (int i = 0; i < hmm->state_num; i++)
        ps->beta[T - 1][i] = 1;
    for (int t = T - 2; t >= 0; t--)
    {
        for (int i = 0; i < hmm->state_num; i++)
        {
            ps->beta[t][i] = 0;
            for (int j = 0; j < hmm->state_num; j++)
                ps->beta[t][i] += hmm->transition[i][j] * hmm->observation[j][td[n][t + 1]] * ps->beta[t + 1][j];
        }
    }

    // calculate gamma
    for (int t = 0; t < T; t++)
    {
        for (int i = 0; i < hmm->state_num; i++)
            ps->gamma[t][i] = (ps->alpha[t][i] * ps->beta[t][i]) / prob_observ;
    }

    // calculate epsilon
    for (int t = 0; t < T - 1; t++)
    {
        for (int i = 0; i < hmm->state_num; i++)
        {
            for (int j = 0; j < hmm->state_num; j++)
                ps->epsilon[t][i][j] = (ps->alpha[t][i] * hmm->transition[i][j] * hmm->observation[j][td[n][t + 1]] * ps->beta[t + 1][j]) / prob_observ;
        }
    }
};

void Trainer::Update_HMM()
{
    for (int n = 0; n < seq_num; n++)
        Compute_Params(n);

    // update initial
    for (int i = 0; i < hmm->state_num; i++)
    {
        hmm->initial[i] = 0;
        for (int n = 0; n < seq_num; n++)
            hmm->initial[i] += params[n].gamma[0][i];
        hmm->initial[i] /= seq_num;
    }

    // update transition and observation
    double nt_gamma_sum[hmm->state_num] = {0};

    for (int i = 0; i < hmm->state_num; i++)
    {
        for (int n = 0; n < seq_num; n++)
        {
            for (int t = 0; t < T - 1; t++)
                nt_gamma_sum[i] += params[n].gamma[t][i];
        }
    }

    // update transition
    for (int i = 0; i < hmm->state_num; i++)
    {
        for (int j = 0; j < hmm->state_num; j++)
        {
            hmm->transition[i][j] = 0;
            for (int n = 0; n < seq_num; n++)
            {
                for (int t = 0; t < T - 1; t++)
                    hmm->transition[i][j] += params[n].epsilon[t][i][j];
            }
            hmm->transition[i][j] /= nt_gamma_sum[i];
        }
    }

    // update observation
    for (int i = 0; i < hmm->state_num; i++)
    {
        for (int n = 0; n < seq_num; n++)
            nt_gamma_sum[i] += params[n].gamma[T - 1][i];
    }

    for (int i = 0; i < hmm->state_num; i++)
    {
        for (int k = 0; k < hmm->observ_num; k++)
        {
            hmm->observation[i][k] = 0;
            for (int n = 0; n < seq_num; n++)
            {
                for (int t = 0; t < T - 1; t++)
                {
                    if (td[n][t] == k)
                        hmm->observation[i][k] += params[n].gamma[t][i];
                }
            }
            hmm->observation[i][k] /= nt_gamma_sum[i];
        }
    }
}