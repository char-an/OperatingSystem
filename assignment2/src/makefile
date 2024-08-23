PART1_EXEC = part1_searcher.out
PART2_PARTITIONER_EXEC = part2_partitioner.out
PART2_SEARCHER_EXEC = part2_searcher.out

PART1_SOURCE = part1_searcher.cpp
PART2_PARTITIONER_SOURCE = part2_partitioner.cpp
PART2_SEARCHER_SOURCE = part2_searcher.cpp

all: build-part1 build-part2

build-part1:
	$(CXX) $(CXXFLAGS) $(PART1_SOURCE) -o $(PART1_EXEC)

build-part2:
	$(CXX) $(CXXFLAGS) $(PART2_PARTITIONER_SOURCE) -o $(PART2_PARTITIONER_EXEC)
	$(CXX) $(CXXFLAGS) $(PART2_SEARCHER_SOURCE) -o $(PART2_SEARCHER_EXEC)

run-part1:
	./$(PART1_EXEC) file.txt NGTNIJGK 0 67108863

run-part2:
	./$(PART2_PARTITIONER_EXEC) file.txt NGTNIJGK 0 67108863 8388608

clean:
	rm -f $(PART1_EXEC) $(PART2_PARTITIONER_EXEC) $(PART2_SEARCHER_EXEC)
