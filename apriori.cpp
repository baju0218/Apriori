#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#define PERCENTAGE(n, m) (double)(n) / (double)(m) * 100.0
#define FOR_ITEMSET(it, begin, end) for (itemset::iterator (it) = (begin); (it) != (end); (it)++)
#define FOR_TABLE(it, begin, end) for (table::iterator (it) = (begin); (it) != (end); (it)++)

using namespace std;



/*******************************************************/
/******************** Variable Part ********************/
/*******************************************************/

/* Argument variables */
double min_support; // Minimum support
ifstream input; // Input file
ofstream output; // Ouput file

/* Database variables */
typedef vector<int> itemset;
vector<itemset> TDB; // Transaction database

/* Apriori variables */
typedef map<itemset, int> table;
vector<table> C; // C[k] : Candidate itemset of size k
vector<table> L; // L[k] : Frequent itemset of size k



/**********************************************************/
/******************** Transaction Part ********************/
/**********************************************************/

/* Split transaction based on tab */
itemset Split_Transaction(string target) {
	itemset result;

	int position = target.find("\t");
	while (position != -1) {
		string str = target.substr(0, position);
		result.push_back(stoi(str));
		target = target.substr(position + 1);
		position = target.find("\t");
	}
	result.push_back(stoi(target));

	return result;
}

/* Sort transaction in ascending order */
bool Ascending_Order(int a, int b) {
	return a < b;
}



/******************************************************/
/******************** Itemset Part ********************/
/******************************************************/

/* Count the number of the same item */
int Count_Same(itemset a, itemset b) {
	int cnt = 0;

	int i = 0, j = 0;
	while (i < a.size() && j < b.size()) {
		if (a[i] == b[j]) { cnt++; i++; j++; }
		else if (a[i] < b[j]) { i++; }
		else if (a[i] > b[j]) { j++; }
	}

	return cnt;
}

/* Self joining candidates */
void Self_Joining(itemset a, itemset b) {
	int k = a.size();

	/* Pruning : Candidate's size is not k + 1 */
	if (Count_Same(a, b) != k - 1)
		return;

	/* Self joining candidates */
	itemset candidate;

	int i = 0, j = 0;
	while (i != a.size() || j != b.size()) {
		if (i == a.size() || a[i] > b[j]) { candidate.push_back(b[j]); j++; }
		else if (j == b.size() || a[i] < b[j]) { candidate.push_back(a[i]); i++; }
		else if (a[i] == b[j]) { candidate.push_back(a[i]); i++; j++; }
	}

	/* Pruning : Candidate's subset is not in L[k - 1] */
	for (int i = 0; i < candidate.size(); i++) {
		itemset subset = candidate;
		subset.erase(subset.begin() + i);

		if (L[k].find(subset) == L[k].end())
			return;
	}

	/* Generate candidate */
	C[k + 1][candidate] = 0;
}



/******************************************************/
/******************** Apriori Part ********************/
/******************************************************/

/* Generate C[k] */
void Generate_C(int k) {
	C.push_back({}); // Create empty C[k]

	if (k == 1) {
		/* 1st Candidate */
		for (int Tid = 0; Tid < TDB.size(); Tid++) {
			itemset transaction = TDB[Tid];

			FOR_ITEMSET(it, transaction.begin(), transaction.end()) {
				itemset candidate(1, *it);

				if (C[k].find(candidate) == C[k].end())
					C[k][candidate] = 1;// Candidate generation
				else
					C[k][candidate]++; // Candidate counting
			}
		}
	}
	else {
		/* Candidate generation */
		FOR_TABLE(it_a, L[k - 1].begin(), L[k - 1].end()) {
			itemset itemset_a = it_a->first;

			FOR_TABLE(it_b, it_a, L[k - 1].end()) {
				itemset itemset_b = it_b->first;

				Self_Joining(itemset_a, itemset_b);
			}
		}

		/* Candidate counting */
		for (int Tid = 0; Tid < TDB.size(); Tid++) {
			itemset transaction = TDB[Tid];
		
			FOR_TABLE(it, C[k].begin(), C[k].end()) {
				if (Count_Same(transaction, it->first) == k)
					C[k][it->first]++;
			}
		}
	}
}

/* Generate L[k] */
void Generate_L(int k) {
	L.push_back({});; // Create empty L[k]

	/* Frequent generation */
	FOR_TABLE(it, C[k].begin(), C[k].end()) {
		if (PERCENTAGE(it->second, TDB.size()) >= min_support)
			L[k][it->first] = it->second;
	}
}



/***************************************************************/
/******************** Association Rule Part ********************/
/***************************************************************/

/* Print itemset */
void Print_Itemset(itemset items) {
	output << "{";
	FOR_ITEMSET(it, items.begin(), items.end()) {
		if (it != items.begin())
			output << ",";
		output << *it;
	}
	output << "}" << "\t";
}

/* Print probability */
void Print_Probability(double prob, string str) {
	output << fixed;
	output.precision(2);
	output << prob << str;
}

/* Get support */
int Get_Support(itemset items) {
	int k = items.size();

	map<itemset, int>::iterator it = L[k].find(items);

	if (it == L[k].end())
		return 0;
	else
		return it->second;
}

/* Generate association rule */
void Generate_Association_Rule(itemset items, itemset X, itemset Y, int index) {
	/* Print association rule */
	if (items.size() == index) {
		if (X.size() == 0 || Y.size() == 0) return;

		int XYsupport = Get_Support(items);
		int Xsupport = Get_Support(X);

		if (Xsupport == 0) return;

		Print_Itemset(X); // Print X
		Print_Itemset(Y); // Print Y
		Print_Probability(PERCENTAGE(XYsupport, TDB.size()), "\t"); // Print support
		Print_Probability(PERCENTAGE(XYsupport, Xsupport), "\n"); // Print confidence

		return;
	}

	/* Generate association rule */
	X.push_back(items[index]);
	Generate_Association_Rule(items, X, Y, index + 1);
	X.pop_back();
	Y.push_back(items[index]);
	Generate_Association_Rule(items, X, Y, index + 1);
}



/***************************************************/
/******************** Main Part ********************/
/***************************************************/

int main(int argc, char* argv[]) {
	/* Arguments */
	if (argc != 4) {
		cout << "Error : Arguments" << endl;
		cout << argv[1] << " -> Wrong arguments" << endl;

		return 0;
	}



	/* Minimum support */ 
	try {
		min_support = stod(argv[1]);

		if (min_support < 0.0 || min_support > 100.0)
			throw min_support;
	}
	catch (exception & e) {
		cout << "Error : Minimum support" << endl;
		cout << argv[1] << " -> Not number" << endl;

		return 0;
	}
	catch (double d) {
		cout << "Error : Minimum support" << endl;
		cout << argv[1] << " -> Not 0 ~ 100%" << endl;

		return 0;
	}



	/* Input file */
	input.open(argv[2]);

	if (!input.is_open()) {
		cout << "Error : Input file name" << endl;
		cout << argv[2] << " -> Not exist" << endl;

		return 0;
	}

	while (!input.eof()) {
		string input_line;
		getline(input, input_line);

		itemset transaction = Split_Transaction(input_line); // Split transaction
		sort(transaction.begin(), transaction.end(), Ascending_Order); //Sort transaction

		TDB.push_back(transaction); // Store TDB
	}
	
	input.close();



	/* Apriori algorithm */
	C.push_back({}); // Not use C[0]
	L.push_back({}); // Not use L[0]
	Generate_C(1); // Generate C[1]
	Generate_L(1); // Generate L[1]

	for (int k = 1; L[k].size() != 0; k++) {
		Generate_C(k + 1);
		Generate_L(k + 1);
	}



	// Output file //
	output.open(argv[3]);

	if (!output.is_open()) {
		cout << "Error : Output file name" << endl;
		cout << argv[3] << " -> Not exist" << endl;

		return 0;
	}
	
	for (int k = 2; L[k].size() != 0; k++) {
		FOR_TABLE(it, L[k].begin(), L[k].end()) {
			Generate_Association_Rule(it->first, {}, {}, 0);
		}
	}

	output.close();



	return 0;
}