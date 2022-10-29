#include "../inc/hmm.h"
#include "trainer.h"
#include <iostream>
#include <string>
#include <chrono>

using namespace std;

int main(int argc, char *argv[])
{
    auto begin = chrono::high_resolution_clock::now();

    if (argc != 5) // check validation
    {
        cout << "Command not valid. It should be in the following form:\n"
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
         << "+==========+==========+==========+\n";

    // static variables

    // basic problem 3
    HMM hmm;
    loadHMM(&hmm, model_init_path.data());
    Trainer trainer(seq_path.data(), &hmm);

    for (int i = 0; i < iter; i++)
    {
        if ((i + 1) % 10 == 0)
            cout << (i + 1) << flush;
        else
            cout << "." << flush;
        trainer.Update_HMM();
    }

    // export model
    FILE *fp = open_or_die(output_model_path.data(), "w");
    dumpHMM(fp, &hmm);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    cout << "\t Take " << duration / 1000.0 << " seconds.\n";
    return 0;
}
