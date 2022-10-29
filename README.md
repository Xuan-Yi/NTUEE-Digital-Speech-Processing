# NTUEE-Digital-Speech-Processing

## HW1
### Commands
* All commands are typed in the root of dsp_hw1.
```=shell
# Compiling
make

＃ Training
./train <iter> <model_init_path> <seq_path> <output_model_path>
./train 100 model_init.txt data/train_seq_01.txt model_01.txt

# Testing
./test <models_list_path> <seq_path> <output_result_path>
./test modellist.txt data/test_seq.txt result.txt
```