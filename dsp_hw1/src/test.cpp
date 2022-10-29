#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "../inc/hmm.h"
#include "Viterbi.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4) // check validation
    {
        cout << "Command not valid. It should be in the folowing form:\n"
             << "\t./test <models_list_path> <seq_path> <output_result_path>\n";
        exit(1);
    }

    // parser
    string model_list_path = argv[1];    // path to model_list.txt
    string seq_path = argv[2];           // path to test_seq.txt
    string output_result_path = argv[3]; // path to result.txt

    // basic problem 2
    Viterbi viterbi(seq_path.data(), model_list_path.data());
    const int seq_num = viterbi.get_seq_num();
    result_pair result[seq_num];

    for (int n = 0; n < seq_num; n++)
        result[n] = viterbi.Do_Viterbi(n);

    // read modellist.txt
    vector<string> modelname;
    ifstream f_ml(model_list_path.data()); // read modellist.txt

    if (!f_ml.is_open())
    {
        cout << "failed to open: " << model_list_path.data() << "\n";
        exit(1);
    }

    string line;

    while (getline(f_ml, line))
        modelname.push_back(line);
    f_ml.close();

    // export result.txt
    ofstream fw(output_result_path.data());

    for (int n = 0; n < seq_num; n++)
    {
        string best_model = modelname[result[n].best_model];
        double likelihood = result[n].likelihood;
        fw << modelname[result[n].best_model] << " " << likelihood << "\n";
    }
    fw.close();

    // evaluate accuracy
    const string test_lbl_path = "data/test_lbl.txt"; // path to test_lbl.txt
    vector<string> test_lbl;                          // test labels
    ifstream f_tl(test_lbl_path);
    int count = 0; // number of correctly recognized sequences

    if (!f_tl.is_open())
    {
        cout << "failed to open: " << test_lbl_path << "\n";
        exit(1);
    }
    string label;
    while (getline(f_tl, label))
        test_lbl.push_back(label);
    f_tl.close();

    for (int n = 0; n < seq_num; n++)
    {
        if (test_lbl[n] == modelname[result[n].best_model])
            count++;
    }
    cout << "Accuracy: " << 100 * count / (double)seq_num << "%\n";

    return 0;
}