#ifndef MYDISAMBIG_H
#define MYDISAMBIG_H

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include "Ngram.h"

using namespace std;

typedef struct
{
    char val[2];
} big5;

vector<vector<big5>> extractlines(const char *fin_path)
{
    ifstream fin(fin_path, ios::binary);
    vector<vector<big5>> sentences;
    string line;

    if (!fin.is_open())
    {
        printf("input file: %s does not exist.\n", fin_path);
        exit(2);
    }

    while (getline(fin, line))
    {
        vector<big5> s;
        line.erase(remove(line.begin(), line.end(), ' '), line.end()); // remove spaces
        for (int i = 0; i < line.length(); i += 2)
        {
            big5 ch;
            ch.val[0] = line[i];
            ch.val[1] = line[i + 1];
            s.push_back(ch);
        }

        sentences.push_back(s);
    }

    fin.close();
    return sentences;
}

void print_sentence(vector<vector<big5>> sentences, int row)
{
    printf("row: %d\n", row + 1);
    for (int i = 0; i < sentences[row].size(); i++)
        printf("%c%c\n", sentences[row][i].val[0], sentences[row][i].val[1]);
    printf("\ncharacter count: %d\n", int(sentences[row].size()));
}

class Viterbi
{
public:
    Viterbi(const char *, const char *, bool);
    ~Viterbi();
    void do_Viterbi(const char *, const char *);
    double prob_ngram(const char *, const char *, const char *);

    friend vector<vector<big5>> extractlines(const char *fin_path);
    friend void print_sentence(vector<vector<big5>> sentences, int row);

private:
    int N; // length of zhuyin_big5
    vector<vector<big5>> zhuyin_big5;
    vector<vector<big5>> sentence_in;
    vector<string> sentence_out;
    Vocab voc;
    Ngram *unigram, *bigram, *trigram;
    bool tri = false;

    string disambiguation_bigram(vector<big5>);
    string disambiguation_trigram(vector<big5>);
};

Viterbi::Viterbi(const char *lm_path, const char *zhuyin_big5_path, bool _tri)
{
    tri = _tri;

    // get language model (bigram)
    unigram = new Ngram(voc, 1);
    bigram = new Ngram(voc, 2);
    if (tri)
        trigram = new Ngram(voc, 3);
    {
        File lmFile(lm_path, "r");
        unigram->read(lmFile);
        fseek(lmFile, 0, SEEK_SET);
        bigram->read(lmFile);
        fseek(lmFile, 0, SEEK_SET);
        if (tri)
            trigram->read(lmFile);
        lmFile.close();
    }

    // get mapping
    ifstream fin(zhuyin_big5_path, ios::binary);
    string line;
    N = 0;
    while (getline(fin, line))
    {
        vector<big5> vec;
        big5 b;

        for (int i = 0; i < line.length(); i += 3)
        {
            b.val[0] = line[i];
            b.val[1] = line[i + 1];
            vec.push_back(b);
        }
        zhuyin_big5.push_back(vec);
    }
    N = zhuyin_big5.size(); // number of states
    fin.close();
};

Viterbi::~Viterbi()
{
    delete unigram;
    delete bigram;
    if (tri)
        delete trigram;
}

double Viterbi::prob_ngram(const char *w1, const char *w2 = nullptr, const char *w3 = nullptr)
{
    if (w2 == nullptr && w3 == nullptr) // unigram
    {
        VocabIndex wid1 = voc.getIndex(w1);

        if (wid1 == Vocab_None) // OOV
            wid1 = voc.getIndex(Vocab_Unknown);
        VocabIndex context[] = {Vocab_None};
        return unigram->wordProb(wid1, context);
    }
    else if (w3 == nullptr) // bigram
    {
        VocabIndex wid1 = voc.getIndex(w1);
        VocabIndex wid2 = voc.getIndex(w2);

        if (wid1 == Vocab_None) // OOV
            wid1 = voc.getIndex(Vocab_Unknown);
        if (wid2 == Vocab_None) // OOV
            wid2 = voc.getIndex(Vocab_Unknown);

        VocabIndex context[] = {wid2, Vocab_None};
        return bigram->wordProb(wid1, context);
    }
    else // trigram
    {
        VocabIndex wid1 = voc.getIndex(w1);
        VocabIndex wid2 = voc.getIndex(w2);
        VocabIndex wid3 = voc.getIndex(w3);

        if (wid1 == Vocab_None) // OOV
            wid1 = voc.getIndex(Vocab_Unknown);
        if (wid2 == Vocab_None) // OOV
            wid2 = voc.getIndex(Vocab_Unknown);
        if (wid3 == Vocab_None) // OOV
            wid3 = voc.getIndex(Vocab_Unknown);

        VocabIndex context[] = {wid2, wid3, Vocab_None};
        return trigram->wordProb(wid1, context);
    }
}

void Viterbi::do_Viterbi(const char *fin_path, const char *fout_path)
{
    sentence_in = extractlines(fin_path);
    for (int i = 0; i < sentence_in.size(); i++) // Viterbis bigram
        if (!tri)
            sentence_out.push_back(disambiguation_bigram(sentence_in[i]));
        else
            sentence_out.push_back(disambiguation_trigram(sentence_in[i]));

    // write output file
    ofstream fout(fout_path, ios::binary);
    for (int i = 0; i < sentence_out.size(); i++)
        fout << sentence_out[i] << "\n";
    fout.close();
}

string Viterbi::disambiguation_bigram(vector<big5> sentence)
{
    vector<vector<tuple<big5, double, int>>> delta; // last char, prob, best last char idx
    const int T = sentence.size();
    vector<tuple<big5, double, int>> col;

    // Viterbi: initialize
    col.clear();
    for (int n = 0; n < N; n++)
    {
        if (sentence[0].val[0] == zhuyin_big5[n][0].val[0] && sentence[0].val[1] == zhuyin_big5[n][0].val[1])
        {
            for (int i = 0; i < zhuyin_big5[n].size() - 1; i++)
            {
                char chararr[2];
                chararr[0] = zhuyin_big5[n][i + 1].val[0];
                chararr[1] = zhuyin_big5[n][i + 1].val[1];

                tuple<big5, double, int> ch(zhuyin_big5[n][i + 1], prob_ngram(chararr, "<s>"), 0);
                col.push_back(ch);
            }
            break;
        }
    }
    if (col.empty())
    {
        tuple<big5, double, int> ch(sentence[0], prob_ngram(sentence[0].val, "<s>"), 0);
        col.push_back(ch);
    }
    delta.push_back(col);

    // Viterbi: iteration
    for (int t = 1; t < T; t++)
    {
        col.clear();
        for (int n = 0; n < N; n++)
        {
            if (sentence[t].val[0] == zhuyin_big5[n][0].val[0] && sentence[t].val[1] == zhuyin_big5[n][0].val[1])
            {
                for (int j = 0; j < zhuyin_big5[n].size() - 1; j++)
                {
                    int max_idx = 0;         // i that cause max prob
                    double max_i = -1000000; // max delta among all candidates

                    char chararr[2];
                    chararr[0] = zhuyin_big5[n][j + 1].val[0];
                    chararr[1] = zhuyin_big5[n][j + 1].val[1];

                    for (int i = 0; i < delta[t - 1].size(); i++)
                    {
                        const big5 last_ch = get<0>(delta[t - 1][i]);
                        double candidate = prob_ngram(chararr, last_ch.val) + get<1>(delta[t - 1][i]);
                        if (candidate > max_i)
                        {
                            max_i = candidate;
                            max_idx = i;
                        }
                    }
                    tuple<big5, double, int> ch(zhuyin_big5[n][j + 1], max_i, max_idx);
                    col.push_back(ch);
                }
                break;
            }
        }
        if (col.empty())
        {
            int max_idx = 0;         // i that cause max prob
            double max_i = -1000000; // max delta among all candidates

            for (int i = 0; i < delta[t - 1].size(); i++)
            {
                const big5 last_ch = get<0>(delta[t - 1][i]);
                double candidate = prob_ngram(sentence[t].val, last_ch.val) + get<1>(delta[t - 1][i]);
                if (candidate > max_i)
                {
                    max_i = candidate;
                    max_idx = i;
                }
            }
            tuple<big5, double, int> ch(sentence[t], max_i, max_idx);
            col.push_back(ch);
        }
        delta.push_back(col);
    }

    // Viterbi: traceback
    int traceback[T];
    int max_T_idx = 0;
    double max_T = -1000000;

    for (int i = 0; i < delta[T - 1].size(); i++)
    {
        const big5 last_ch = get<0>(delta[T - 1][i]);
        double candidate = prob_ngram("</s>", last_ch.val) + get<1>(delta[T - 1][i]);
        if (candidate > max_T)
        {
            max_T = candidate;
            max_T_idx = i;
        }
    }

    traceback[T - 1] = max_T_idx;
    for (int t = T - 1; t >= 1; t--)
        traceback[t - 1] = get<2>(delta[t][traceback[t]]);

    // to string
    string outstr = "";
    for (int t = 0; t < T; t++)
    {
        char c[2];
        c[0] = get<0>(delta[t][traceback[t]]).val[0];
        c[1] = get<0>(delta[t][traceback[t]]).val[1];
        outstr = outstr + string(c) + " ";
    }
    outstr = "<s> " + outstr + "</s>";
    return outstr;
};

string Viterbi::disambiguation_trigram(vector<big5> sentence)
{
    vector<vector<tuple<big5, big5, double, int>>> delta; // last last char, last char, prob, best last last char idx
    const int T = sentence.size();
    vector<tuple<big5, big5, double, int>> col;

    // Viterbi: initialize <s> w0
    col.clear();
    for (int n = 0; n < N; n++)
    {
        if (sentence[0].val[0] == zhuyin_big5[n][0].val[0] && sentence[0].val[1] == zhuyin_big5[n][0].val[1])
        {
            for (int i = 0; i < zhuyin_big5[n].size() - 1; i++)
            {
                char chararr[2];
                chararr[0] = zhuyin_big5[n][i + 1].val[0];
                chararr[1] = zhuyin_big5[n][i + 1].val[1];
                big5 empt;

                tuple<big5, big5, double, int> ch(empt, zhuyin_big5[n][i + 1], prob_ngram(chararr, "<s>"), 0);
                col.push_back(ch);
            }
            break;
        }
    }
    if (col.empty())
    {
        big5 empt;

        tuple<big5, big5, double, int> ch(empt, sentence[0], prob_ngram(sentence[0].val, "<s>"), 0);
        col.push_back(ch);
    }
    delta.push_back(col);

    // Viterbi: iteration
    for (int t = 1; t < T; t++)
    {
        col.clear();
        for (int n = 0; n < N; n++)
        {
            if (sentence[t].val[0] == zhuyin_big5[n][0].val[0] && sentence[t].val[1] == zhuyin_big5[n][0].val[1])
            {
                for (int i = 0; i < zhuyin_big5[n].size() - 1; i++)
                {
                    int max_idx = 0;         // i that cause max prob
                    double max_i = -1000000; // max delta among all candidates

                    char chararr[2];
                    chararr[0] = zhuyin_big5[n][i + 1].val[0];
                    chararr[1] = zhuyin_big5[n][i + 1].val[1];

                    for (int k = 0; k < delta[t - 1].size(); k++)
                    {
                        const big5 last_last_ch = get<0>(delta[t - 1][k]);
                        const big5 last_ch = get<1>(delta[t - 1][k]);
                        double candidate;
                        if (t == 1)
                            candidate = prob_ngram(chararr, last_ch.val, "<s>") + get<2>(delta[t - 1][k]);
                        else
                            candidate = prob_ngram(chararr, last_ch.val, last_last_ch.val) + get<2>(delta[t - 1][k]);

                        if (candidate > max_i)
                        {
                            max_i = candidate;
                            max_idx = k;
                        }
                        // if ((k < delta[t - 1].size() - 1) && ((get<1>(delta[t - 1][k]).val[0] != get<1>(delta[t - 1][k + 1]).val[0]) || (get<1>(delta[t - 1][k]).val[1] != get<1>(delta[t - 1][k + 1]).val[1])))
                        // {
                        //     tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), zhuyin_big5[n][i + 1], max_i, max_idx);
                        //     col.push_back(ch);
                        //     max_i = -1000000;
                        // }
                        // if (k == delta[t - 1].size() - 1)
                        // {
                        //     tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), zhuyin_big5[n][i + 1], max_i, max_idx);
                        //     col.push_back(ch);
                        // }
                    }
                    tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), zhuyin_big5[n][i + 1], max_i, max_idx);
                    col.push_back(ch);
                }
                break;
            }
        }
        if (col.empty())
        {
            int max_idx = 0;         // i that cause max prob
            double max_i = -1000000; // max delta among all candidates

            for (int k = 0; k < delta[t - 1].size(); k++)
            {
                const big5 last_last_ch = get<0>(delta[t - 1][k]);
                const big5 last_ch = get<1>(delta[t - 1][k]);
                double candidate;
                if (t == 1)
                    candidate = prob_ngram(sentence[t].val, last_ch.val, "<s>") + get<2>(delta[t - 1][k]);
                else
                    candidate = prob_ngram(sentence[t].val, last_ch.val, last_last_ch.val) + get<2>(delta[t - 1][k]);

                if (candidate > max_i)
                {
                    max_i = candidate;
                    max_idx = k;
                }
                // if ((k < delta[t - 1].size() - 1) && (get<1>(delta[t - 1][k]).val[0] != get<1>(delta[t - 1][k + 1]).val[0] || get<1>(delta[t - 1][k]).val[1] != get<1>(delta[t - 1][k + 1]).val[1]))
                // {
                //     tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), sentence[t], max_i, max_idx);
                //     col.push_back(ch);
                //     max_i = -1000000;
                // }
                // if (k == delta[t - 1].size() - 1)
                // {
                //     tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), sentence[t], max_i, max_idx);
                //     col.push_back(ch);
                // }
            }
            tuple<big5, big5, double, int> ch(get<1>(delta[t - 1][max_idx]), sentence[t], max_i, max_idx);
            col.push_back(ch);
        }
        delta.push_back(col);
    }

    // Viterbi: traceback
    int traceback[T];
    int max_T_idx = 0;
    double max_T = -1000000;

    for (int k = 0; k < delta[T - 1].size(); k++)
    {
        const big5 last_last_ch = get<0>(delta[T - 1][k]);
        const big5 last_ch = get<1>(delta[T - 1][k]);
        double candidate = prob_ngram("</s>", last_ch.val, last_last_ch.val) + get<2>(delta[T - 1][k]);

        if (candidate > max_T)
        {
            max_T = candidate;
            max_T_idx = k;
        }
    }
    traceback[T - 1] = max_T_idx;
    for (int t = T - 1; t >= 1; t--)
        traceback[t - 1] = get<3>(delta[t][traceback[t]]);

    // to string
    string outstr;
    for (int t = 0; t < T; t++)
    {
        char c[2];
        c[0] = get<1>(delta[t][traceback[t]]).val[0];
        c[1] = get<1>(delta[t][traceback[t]]).val[1];
        outstr = outstr + string(c) + " ";
    }
    outstr = "<s> " + outstr + "</s>";
    return outstr;
};

#endif