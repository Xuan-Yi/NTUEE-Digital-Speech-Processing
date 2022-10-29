#include "trainer.h"

Trainer::Trainer(char *filename, HMM *_hmm)
{
    hmm = _hmm;

    if (!this->Load_Train_Data(filename))
        exit(1);

    // allocate param arrays
    params.clear();
    for (int n = 0; n < seq_num; n++)
    {
        ParamSet p;
        // allocate alpha and beta
        p.alpha = new double *[T];
        p.beta = new double *[T];
        for (int t = 0; t < T; t++)
        {
            p.alpha[t] = new double[hmm->state_num];
            p.beta[t] = new double[hmm->state_num];
        }
        // allocate gamma
        p.gamma = new double *[T];
        for (int t = 0; t < T; t++)
            p.gamma[t] = new double[hmm->state_num];
        // allocate epsilon
        p.epsilon = new double **[T];
        for (int t = 0; t < T; t++)
        {
            p.epsilon[t] = new double *[hmm->state_num];
            for (int i = 0; i < hmm->state_num; i++)
                p.epsilon[t][i] = new double[hmm->state_num];
        }
        params.push_back(p);
    }
}

Trainer::~Trainer()
{
    // deallocate param arrays
    for (int n = 0; n < seq_num; n++)
    {
        ParamSet *p = &(params[n]);

        // deallocate alpha and beta
        for (int i = 0; i < hmm->state_num; i++)
        {
            delete[] p->alpha[i];
            delete[] p->beta[i];
        }
        delete[] p->alpha;
        delete[] p->beta;
        // deallocate gamma
        for (int t = 0; t < T; t++)
            delete[] p->gamma[t];
        delete[] p->gamma;
        // deallocate epsilon
        for (int t = 0; t < T; t++)
        {
            for (int i = 0; i < hmm->state_num; i++)
                delete[] p->epsilon[t][i];
            delete[] p->epsilon[t];
        }
        delete p->epsilon;
    }
    params.clear();
    // deallocate training data
    for (int t = 0; t < T; t++)
        delete[] td[t];
    delete td;
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
    seq_num = fr.size();

    // allocate train data
    td = new int *[seq_num];
    for (int n = 0; n < seq_num; n++)
        td[n] = new int[T];

    // turn all characters (A~F) into state index (0~5)
    for (int n = 0; n < seq_num; n++)
    {
        for (int t = 0; t < T; t++)
            td[n][t] = int(fr[n][t] - 'A');
    }
    // cout << "training data loaded successfully\n";
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
    double gamma_sum[T] = {0};

    for (int t = 0; t < T; t++)
    {
        for (int j = 0; j < hmm->state_num; j++)
            gamma_sum[t] += ps->alpha[t][j] * ps->beta[t][j];
    }
    for (int t = 0; t < T; t++)
    {
        for (int i = 0; i < hmm->state_num; i++)
            ps->gamma[t][i] = (ps->alpha[t][i] * ps->beta[t][i]) / gamma_sum[t];
    }

    // calculate epsilon
    double epsilon_sum[T - 1] = {0};

    for (int t = 0; t < T - 1; t++)
    {
        for (int i = 0; i < hmm->state_num; i++)
        {
            for (int j = 0; j < hmm->state_num; j++)
                epsilon_sum[t] += ps->alpha[t][i] * hmm->transition[i][j] * hmm->observation[j][td[n][t + 1]] * ps->beta[t + 1][j];
        }
    }

    for (int t = 0; t < T - 1; t++)
    {
        for (int i = 0; i < hmm->state_num; i++)
        {
            for (int j = 0; j < hmm->state_num; j++)
                ps->epsilon[t][i][j] = (ps->alpha[t][i] * hmm->transition[i][j] * hmm->observation[j][td[n][t + 1]] * ps->beta[t + 1][j]) / epsilon_sum[t];
        }
    }
};

void Trainer::Update_HMM()
{
    // cout << "update HMM\n";
    for (int n = 0; n < seq_num; n++)
        Compute_Params(n);
    cout << "finish calculating alpha, beta, gamma, and epsilon for all sequences\n";

    // update initial
    for (int i = 0; i < hmm->state_num; i++)
    {
        hmm->initial[i] = 0;
        for (int n = 0; n < seq_num; n++)
            hmm->initial[i] += params[n].gamma[0][i];
        hmm->initial[i] /= seq_num;
    }
    cout << "initial updated\n";

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
    cout << "transition updated\n";

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
    cout << "observation updated\n";
}