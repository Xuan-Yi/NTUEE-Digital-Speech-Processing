#! /bin/bash

make

# # Train
# if [[ -n $1 ]]; then
#     iter=$1
# else
#     iter=100
# fi
# ./train $iter model_init.txt data/train_seq_01.txt model_01.txt &
# ./train $iter model_init.txt data/train_seq_02.txt model_02.txt &
# ./train $iter model_init.txt data/train_seq_03.txt model_03.txt &
# ./train $iter model_init.txt data/train_seq_04.txt model_04.txt &
# ./train $iter model_init.txt data/train_seq_05.txt model_05.txt &
# wait

# Test
./test modellist.txt data/test_seq.txt result.txt

# Accuracy
file1="data/test_lbl.txt"
file2="result.txt"
exec 3< $file1
exec 4< $file2

right=0
total=0
while true; do
    read -r f1 <&3 || break
    read -r f2 <&4 || break
    f2=$(echo $f2 | cut -d " " -f 1)

    if [[ "$f1" == "$f2" ]]; then
        ((right++))
    fi
    ((total++))
done
echo "$right, $(echo "$right/$total" | bc -l | awk '{printf "%.2f%", $0*100}')"

exec 3<&-
exec 4<&-
# make clean
exit