#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "unordered_map"
#include "unordered_set"
#include "queue"
#include "stack"
using namespace std;

//c++ does not provide a hash for pairs, so i had to declare it myself
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        return h1 ^ h2;
    }
};

using Unordered_set = std::unordered_set<pair<unsigned, unsigned>, pair_hash>;


class Table {

private:
    // Table contains two maps to match column and line names to indexes. It allows us to find cell by its name in constant time
    unordered_map<string, unsigned> columnsId;
    unordered_map<string, unsigned> linesId;
    vector<vector<string>> table;

    void initColumnNames(string& str) {
        vector<string> elems = split(str);
        if (!elems[0].empty()) {
            throw runtime_error("upper left cell must be empty");
        }
        table.push_back(elems);
        for (int i = 1; i < elems.size(); i++) {
            if (elems[1].empty()) {
                throw runtime_error("column names must not be empty");
            }
            if (columnsId.find(table[0][i]) != columnsId.end()) {
                throw runtime_error("duplicated column names");
            }
            for (char j : elems[i]) {
                if (isdigit(j)) {
                    throw runtime_error("column names must not contain digits");
                }
            }
            columnsId[table[0][i]] = i;
        }
    }

    void readTable(const string &filename) {
        ifstream file;
        file.open(filename);
        if (!file) {
            throw runtime_error("file could not be open");
        }
        if (file.eof()) {
            throw runtime_error("file is empty");
        }
        string str;
        file >> str;
        initColumnNames(str);
        unsigned lineId = 1;
        while (!file.eof()) {
            file >> str;
            vector<string> elems = split(str);
            if (elems.size() != table.back().size()) {
                throw runtime_error("lines must have the same number of elements");
            }
            try {
                if (stoi(elems[0]) <= 0) {
                    throw runtime_error("line numbers should be positive");
                }
            } catch (invalid_argument&) {
                throw runtime_error("line names should be positive numbers");
            }
            if (linesId.find(elems[0]) != linesId.end()) {
                throw runtime_error("duplicated line names");
            }
            linesId[elems[0]] = lineId++;
            table.push_back(elems);

            for (int i = 1; i < elems.size(); i++) {
                if (elems[i].at(0) != '=') {
                    try {
                        stoi(elems[i]);
                    } catch (invalid_argument&) {
                        throw runtime_error(table[0][i] + table[lineId-1][0] + " cell contains neither number nor expression");
                    }
                }
            }
        }
    }

    unsigned getArgColumn(const string& expression, int* start) {
        string columnName;
        while (!isdigit(expression[*start])) {
            columnName += expression[(*start)++];
            if (*start >= expression.length()) {
                throw runtime_error("invalid expression");
            }
        }
        if (columnsId.find(columnName) == columnsId.end()) {
            throw runtime_error("invalid column name in expression");
        }
        return columnsId[columnName];
    }

    unsigned getArgLine(const string& expression, int* start) {
        string lineName;
        while (isdigit(expression[*start]) && *start < expression.length()) {
            lineName += expression[(*start)++];
        }
        if (linesId.find(lineName) == linesId.end()) {
            cout << expression << endl;
            cout << lineName << endl;
            throw runtime_error("invalid line name in expression");
        }
        return linesId[lineName];
    }

    void getArgs(string expression, pair<unsigned, unsigned>* args, char* op) {
        int curCh = 1;
        unsigned columnId = getArgColumn(expression, &curCh);
        unsigned lineId = getArgLine(expression, &curCh);
        args[0] = make_pair(lineId, columnId);
        *op = expression[curCh++];
        if (*op != '+' && *op != '-' && *op != '*' && *op != '/') {
            throw runtime_error("invalid operation in expression");
        }
        columnId = getArgColumn(expression, &curCh);
        lineId = getArgLine(expression, &curCh);
        args[1] = make_pair(lineId, columnId);
    }

    string doOperationWithArgs(char op, pair<unsigned, unsigned>* args) {
        // in this method we have already checked cells, so we can just calculate without checking
        string answer;
        unsigned i0 = args[0].first;
        unsigned j0 = args[0].second;
        unsigned i1 = args[1].first;
        unsigned j1 = args[1].second;
        switch (op) {
            case '+':
                answer = to_string(stoi(table[i0][j0]) + stoi(table[i1][j1]));
                break;
            case '-':
                answer = to_string(stoi(table[i0][j0]) - stoi(table[i1][j1]));
                break;
            case '*':
                answer = to_string(stoi(table[i0][j0]) * stoi(table[i1][j1]));
                break;
            case '/':
                if (stoi(table[i1][j1]) == 0) {
                    throw runtime_error("cell contain division by zero");
                }
                answer = to_string(stoi(table[i0][j0]) / stoi(table[i1][j1]));
                break;
            default:
                throw runtime_error("cell contain invalid operation");
        }
        return answer;
    }

    stack<pair<unsigned, unsigned>> checkRecursion(unsigned i, unsigned j) {
        // to be sure that we have not recursion in expressions (for example A2 cell
        // contains =B1+A1 and B1 contains =A2+A3) i used BFS to find all cells that
        // associated with the cell passed to the method and added them to stack to calculate them from top to bottom
        unsigned i_ = i;
        unsigned j_ = j;
        pair<unsigned, unsigned> args[2];
        queue<pair<unsigned, unsigned>> cells;
        stack<pair<unsigned, unsigned>> stack;
        Unordered_set visited;
        cells.emplace(i, j);
        char op;

        while (!cells.empty()) {
            pair<unsigned, unsigned> cell = cells.front();
            cells.pop();
            if (visited.find(cell) != visited.end()) {
                throw runtime_error("expressions must not have recursion");
            }
            visited.emplace(cell);
            stack.push(cell);
            try {
                string expression = table[cell.first][cell.second];
                getArgs(expression, args, &op);
            } catch (runtime_error& error) {
                throw runtime_error(table[0][cell.second] + table[cell.first][0] + " has " + error.what());
            }
            for (auto arg: args) {
                if (table[arg.first][arg.second][0] == '=') {
                    i_ = arg.first;
                    j_ = arg.second;
                    cells.emplace(i_, j_);
                }
            }
        }
        return stack;
    }

    void calculateCell(stack<pair<unsigned, unsigned>> stack) {
        while (!stack.empty()) {
            unsigned i = stack.top().first;
            unsigned j = stack.top().second;
            stack.pop();
            string expression = table[i][j];
            pair<unsigned, unsigned> args[2];
            char op;

            try {
                getArgs(expression, args, &op);
            } catch (runtime_error& error) {
                throw runtime_error(table[0][j] + table[i][0] + " has " + error.what());
            }

            try {
                table[i][j] = doOperationWithArgs(op, args);
            } catch (runtime_error& error) {
                throw runtime_error(table[0][j] + table[i][0] + error.what());
            }
        }

    }

    void calculateTable() {
        for (int i = 1; i < table.size(); i++) {
            for (int j = 1; j < table[i].size(); j++) {
                if (table[i][j].at(0) == '=') {
                    stack<pair<unsigned, unsigned>> stack = checkRecursion(i, j);
                    calculateCell(stack);
                }
            }
        }
    }

    static vector<string> split(const string &s) {
        stringstream ss(s);
        string item;
        vector<string> elems;
        while (getline(ss, item, ',')) {
            elems.push_back(item);
        }
        return elems;
    }


public:

    explicit Table(const string& filename)  {
        readTable(filename);
        calculateTable();
    }

    string toString() {
        string ans;
        for (auto & i : table) {
            ans += i[0];
            for (int j = 1; j < i.size(); j++) {
                ans += "," + i[j];
            }
            ans += '\n';
        }
        return ans;
    }
};

int main(int argc, char *argv[]) {
    try {
        Table table(argv[1]);
        cout << table.toString();
    } catch (runtime_error& error){
        cout << "Error - " << error.what() << endl;
    }
    return 0;
}