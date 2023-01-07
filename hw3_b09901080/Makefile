SRIPATH ?= /root/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
INC_PATH ?= inc
SRC_PATH ?= src

CXX = g++
CXXFLAGS = -O2 -I$(SRIPATH)/include -I$(INC_PATH)
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)
vpath %.c $(SRC_PATH)
vpath %.cpp $(SRC_PATH)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
FROM ?= Big5-ZhuYin.map
TO ?= ZhuYin-Big5.map
.PHONY: all clean map

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< 

all: $(TARGET)

map:
	python3 $(SRC_PATH)/mapping.py $(FROM) $(TO)

# lm:
# 	perl separator_big5.pl corpus.txt > temp.txt
# 	ngram-count -text temp.txt -write count.txt -order 3
# 	ngram-count -read count.txt -lm corpus.lm -order 3 -unk
# 	rm temp.txt
	
# disambig:
# 	perl separator_big5.pl test_data/example.txt > temp.txt
# 	disambig -text temp.txt -map ZhuYin-Big5.map -lm bigram.txt -order 3 -keep-unk > test_data/outputs/example.out.txt
# 	rm temp.txt

clean:
	$(RM) $(OBJ) $(TARGET)
