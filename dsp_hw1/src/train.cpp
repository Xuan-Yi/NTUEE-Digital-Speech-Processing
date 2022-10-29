#include "../inc/hmm.h"
#include "trainer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>

using namespace std;

int T = 50;      // length of sequence
int seq_num = 0; // number of sequences

int main(int argc, char *argv[])
{
    // time_t init_time = time(nullptr);
    time_t init_time, time_count;

    if (argc != 5) // check validation
    {
        cout << "Command not valid. It should be in the folowing form:\n"
             << "\t./train <iter> <model_init_path> <seq_path> <output_model_path>\n";
        exit(1);
    }

    // parser
    int iter;                           // time of training with same sequences
    string model_init_path = argv[2];   // path to model_init.txt
    string seq_path = argv[3];          // path to train_seq_0x.txt
    string output_model_path = argv[4]; // path to model_0x.txt

    sscanf(argv[1], "%d", &iter);

    cout << "iter: " << iter << "\n"
         << "model_init_path: " << model_init_path << "\n"
         << "seq_path: " << seq_path << "\n"
         << "output_model_path: " << output_model_path << "\n"
         << "+==========+==========+==========+\n\n";

    // basic problem 3
    HMM hmm;
    loadHMM(&hmm, model_init_path.data());

    for (int i = 0; i < iter; i++)
    {
        cout << "=== Start Training " << (i + 1) << " ===\n";
        init_time = time(nullptr);
        Trainer trainer(seq_path.data(), &hmm);
        time_count = time(nullptr) - init_time;
        cout << "Reading takes " << time_count << " seconds.\n";
        init_time = time(nullptr);
        trainer.Update_HMM();
        time_count = time(nullptr) - init_time;
        cout << "Processing takes " << time_count << " seconds.\n";
        cout << "=== Finish Training " << (i + 1) << " ===\n\n";
    }

    // export model
    FILE *fp = open_or_die(output_model_path.data(), "w");
    dumpHMM(fp, &hmm);

    /*
    time_t time_count = time(nullptr) - init_time;
    cout << "Take " << time_count << " seconds.\n";
    */

    return 0;
}
