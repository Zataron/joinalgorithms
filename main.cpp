#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <memory.h>
#include <algorithm>

#define TABLE_SIZE 1000000

using namespace std;
using namespace std::chrono;

typedef std::vector<int> Table;
typedef std::map<int,bool> IntMap;

// Iterator base type
template <typename DataStructure>
class Iterator {
public:
	typename DataStructure::iterator itr;
	typename DataStructure::iterator end;
	DataStructure* data;

	void operator++() {
		itr++;
	}

	bool atEnd() {
		return itr == end;
	}

	// implementation depends on the data structure
	bool operator<(Iterator<DataStructure>& b);
	int operator*();
	void seek(int val);
};

// Iterator implementation for a vector
template<>
bool Iterator<Table>::operator<(Iterator<Table>& b) {
	return *(this->itr) < *b.itr;
}

template<>
int Iterator<Table>::operator*() {
	return *(this->itr);
}

template<>
void Iterator<Table>::seek(int val) {
	// skip to the next integer that is greater or equal to val
	while (!this->atEnd() && *(this->itr) < val)
		(this->itr)++;
}

// Iterator implementation for a map
template<>
bool Iterator<IntMap>::operator<(Iterator<IntMap>& b) {
	return this->itr->first < b.itr->first;
}

template<>
int Iterator<IntMap>::operator*() {
	return this->itr->first;
}

template<>
void Iterator<IntMap>::seek(int val) {
	// use the lower_bound function of the map
	this->itr = this->data->lower_bound(val);
}

/*** Alternative implementation using polymorphism instead of templates ***/
// VectorIterator: iterator for a vector or list of integers
/*template <typename DataStructure>
class VectorIterator : public Iterator<DataStructure> {
public:
	virtual bool operator<(Iterator<DataStructure>& b) override {
		return *(this->itr) < *b.itr;
	}
	int operator*() {
		return *(this->itr);
	}
	void seek (int val) {
		// skip to the next integer that is greater or equal to val
		while (!this->atEnd() && *(this->itr) < val)
			(this->itr)++;
	}
};

// IndexIterator: iterator for a map or unordered map with int as key type
template <typename DataStructure>
class IndexIterator : public Iterator<DataStructure> {
public:
	bool operator<(Iterator<DataStructure>& b) {
		return this->itr->first < b.itr->first;
	}
	int operator*() {
		return this->itr->first;
	}
	void seek(int val) {
		// use the lower_bound function of the map
		this->itr = this->data->lower_bound(val);
	}
};

typedef VectorIterator<Table> TableIterator;
typedef IndexIterator<IntMap> MapIterator;*/


template <typename DataStructure>
int triejoin(DataStructure** tbl_list, bool print_runtime = true) {
	auto start = high_resolution_clock::now();

	// the vector itr contains an iterator for each table
	std::vector<Iterator<DataStructure>> itr;
	DataStructure* table_itr = tbl_list[0];
	int j = 1;
	while (table_itr != NULL) {
		Iterator<DataStructure> p;
		p.itr = table_itr->begin();
		p.end = table_itr->end();
		p.data = table_itr;
		itr.push_back(p);
		table_itr = tbl_list[j++];
	}

	// sort the iterators
	std::sort(itr.begin(), itr.end());

	int size = itr.size();
	int pos = size-1;	// start at last element
	int hits = 0;
	while (true) {
		int val = *itr[pos];
		pos = (pos+1)%size;

		if (*itr[pos] == val) {
			// val is in the join
			// advance current iterator
			++itr[pos];
			if (itr[pos].atEnd())
				break;

			hits++;
		}
		else {
			// val is not in the join
			// skip to the first element that is greater or equal to val
			itr[pos].seek(val);
			if (itr[pos].atEnd())
				break;
		}
	}

	if (print_runtime) {
		double runtime = duration_cast<duration<double>>(high_resolution_clock::now()-start).count();
		cout << "trie join: size of join = " << hits << ", runtime: " << runtime << std::endl;
	}

	return hits;
}

void triejoin_unsorted(Table** tbl_list) {	
	auto start = high_resolution_clock::now();

	// sort each table
	Table* table_itr = tbl_list[0];
	int j = 1;
	while (table_itr != NULL) {
		std::sort(table_itr->begin(), table_itr->end());
		table_itr = tbl_list[j++];
	}

	// do the join
	int hits = triejoin(tbl_list, false);

	double runtime = duration_cast<duration<double>>(high_resolution_clock::now()-start).count();
	cout << "trie join: size of join = " << hits << ", runtime: " << runtime << std::endl;
}

void init_table(Table& tb) {
	int cur = 0;
	tb.push_back(cur);
	for (int i = 1; i < TABLE_SIZE; i++) {
		cur += 1+rand()%100;
		tb.push_back(cur);
	}
}

void print_table(Table& tb) {
	for (int i = 0; i < tb.size(); i++) {
		cout << tb[i] << " ";
	}
	cout << endl;
}

void hashjoin2(Table** tbl_list) {
	// hashjoin for 2 tables
	auto start = high_resolution_clock::now();
	Table tb1 = *tbl_list[0];
	Table tb2 = *tbl_list[1];

	int hits = 0;
	std::unordered_map<int,bool> hashmap;
	for (int i = 0; i < tb1.size(); i++) {
		hashmap.insert(std::make_pair(tb1[i],true));
	}
	for (int i = 0; i < tb2.size(); i++) {
		auto itr = hashmap.find(tb2[i]);
		if (itr != hashmap.end())
			hits++;
	}

	double runtime = duration_cast<duration<double>>(high_resolution_clock::now()-start).count();
	cout << "hash join: size of join = " << hits << ", runtime: " << runtime << std::endl;
}

void hashjoin3(Table** tbl_list) {
	// hashjoin for 3 tables
	auto start = high_resolution_clock::now();
	Table tb1 = *tbl_list[0];
	Table tb2 = *tbl_list[1];
	Table tb3 = *tbl_list[2];

	int hits = 0;
	std::unordered_map<int,bool> hashmap1;
	for (int i = 0; i < tb1.size(); i++) {
		hashmap1.insert(std::make_pair(tb1[i],true));
	}
	std::unordered_map<int,bool> hashmap2;
	for (int i = 0; i < tb2.size(); i++) {
		hashmap2.insert(std::make_pair(tb2[i],true));
	}
	for (int i = 0; i < tb3.size(); i++) {
		auto itr = hashmap1.find(tb3[i]);
		if (itr != hashmap1.end()) {
			itr = hashmap2.find(tb3[i]);
			if (itr != hashmap2.end())
				hits++;
		}
	}

	double runtime = duration_cast<duration<double>>(high_resolution_clock::now()-start).count();
	cout << "hash join: size of join = " << hits << ", runtime: " << runtime << std::endl;
}

// WIP: A more efficient hashjoin implementation for a variable amount of tables is being worked on
/*void hashjoin(Table** tbl_list) {
	auto start = high_resolution_clock::now();
	Table tb1 = *tbl_list[0];
	Table tb2;
	
	int j = 1;
	while (tbl_list[j] != NULL) {
		tb2 = *tbl_list[j];

		Table result;
		std::unordered_map<int,bool> hashmap;
		for (int i = 0; i < tb1.size(); i++) {
			hashmap.insert(std::make_pair(tb1[i],true));
		}
		for (int i = 0; i < tb2.size(); i++) {
			auto itr = hashmap.find(tb2[i]);
			if (itr != hashmap.end())
				result.push_back(tb2[i]);
		}

		tb1 = result;
		j++;
	}

	double runtime = duration_cast<duration<double>>(high_resolution_clock::now()-start).count();
	cout << "hash join: size of join = " << tb1.size() << ", runtime: " << runtime << std::endl;
}*/

int main() {
	//create tables
	Table tb1;
	Table tb2;
	Table tb3;
	init_table(tb1);
	init_table(tb2);
	init_table(tb3);

	//create index structures
	IntMap map[3];	
	for (int i = 0; i < TABLE_SIZE; i++) {
		map[0].insert(std::make_pair(tb1[i],true));
		map[1].insert(std::make_pair(tb2[i],true));
		map[2].insert(std::make_pair(tb3[i],true));
	}

	// print small tables for debugging
	if (TABLE_SIZE <= 20) {
		cout << "Table 1: ";
		print_table(tb1);
		cout << "Table 2: ";
		print_table(tb2);
		cout << "Table 3: ";
		print_table(tb3);
	}

	Table* two_tables[] = {&tb1, &tb2, NULL};
	IntMap* two_maps[] = {&map[0], &map[1], NULL};
	Table* three_tables[] = {&tb1, &tb2, &tb3, NULL};
	IntMap* three_maps[] = {&map[0], &map[1], &map[2], NULL};

	cout << "TEST 1: Join 2 sorted arrays" << endl;
	hashjoin2(two_tables);
	triejoin(two_tables);
	cout << endl;

	cout << "TEST 2: Join 2 indexed tables" << endl;
	triejoin(two_maps);
	cout << endl;

	cout << "TEST 3: Join 3 sorted arrays" << endl;
	hashjoin3(three_tables);
	triejoin(three_tables);
	cout << endl;

	cout << "TEST 4: Join 3 indexed tables" << endl;
	triejoin(three_maps);
	cout << endl;

	cout << "TEST 5: Join 3 unsorted arrays" << endl;
	std::random_shuffle(tb1.begin(), tb1.end());
	std::random_shuffle(tb2.begin(), tb2.end());
	std::random_shuffle(tb3.begin(), tb3.end());
	hashjoin3(three_tables);
	triejoin_unsorted(three_tables);
	cout << endl;
}