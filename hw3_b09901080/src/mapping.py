import sys

fin_path = sys.argv[1]
fout_path = sys.argv[2]


# read Big5-ZhuYin
try:
    fin = open(fin_path,'r',encoding='big5-hkscs')
except:
    print(fin_path," doesn't exist.")
    exit(2)
Big5_ZhuYin = []

for line in fin.readlines():
    new_list = line.split('/')
    big5 = new_list[0][0]
    new_list[0] = new_list[0][2:]
    for i in range(len(new_list)):
        new_list[i] = new_list[i][0]
    res=[]
    [res.append(x) for x in new_list if x not in res]
    Big5_ZhuYin.append( list(big5) + res )
fin.close()
# print(Big5_ZhuYin[:10])

# mapping
map_idx = {
    # ㄅㄆㄇㄈ  ㄉㄊㄋㄌ    ㄍㄎㄏ  ㄐㄑㄒ  ㄓㄔㄕㄖ    ㄗㄘㄙ  ㄧㄨㄩ  ㄚㄛㄜㄝ  ㄞㄟㄠㄡ    ㄢㄣㄤㄥ    ㄦ
    'ㄅ': 0, 'ㄆ': 1, 'ㄇ': 2, 'ㄈ': 3,
    'ㄉ': 4, 'ㄊ': 5, 'ㄋ': 6, 'ㄌ': 7,
    'ㄍ': 8, 'ㄎ': 9, 'ㄏ':10, 
    'ㄐ': 11, 'ㄑ':12, 'ㄒ':13,
    'ㄓ': 14, 'ㄔ': 15, 'ㄕ': 16, 'ㄖ':17,
    'ㄗ':18, 'ㄘ':19, 'ㄙ':20,
    'ㄧ': 21, 'ㄨ': 22, 'ㄩ': 23,
    'ㄚ': 24, 'ㄛ': 25, 'ㄜ': 26, 'ㄝ': 27,
    'ㄞ' :28, 'ㄟ':29, 'ㄠ': 30, 'ㄡ': 31,
    'ㄢ': 32, 'ㄣ': 33, 'ㄤ': 34, 'ㄥ':35, 'ㄦ': 36
}
ZhuYin_Big5 = []

for key in map_idx.keys():
    ZhuYin_Big5.append([key])
for row in Big5_ZhuYin:
    for i in range(1,len(row)):
        ZhuYin_Big5[map_idx[row[i]]].append(row[0])
for row in Big5_ZhuYin:
    big5 = row[0]
    ZhuYin_Big5.append([big5,big5])
    
# output file
try:
    fout = open(fout_path,'w',encoding='big5-hkscs')
except:
    print(fout_path," doesn't exist.")
    exit(2)

for row in ZhuYin_Big5:
    if(len(row)>=2):
        fout.write(f'{row[0]}\t')
        for i in range(1,len(row)-1):
            fout.write(f'{row[i]} ')
        fout.write(f'{row[len(row)-1]}\n')

fout.close()




