#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <set>
#include <map>
#include <queue>
#include <fstream>

using namespace std;

string generateWord(int maxBlocks = 5) {
    string result = "";
    string xParts[] = {"aa", "ab", "ac"}; 
    string yParts[] = {"bc", "cc", "ac"};  
    int blocks = rand() % (maxBlocks + 1);
    for (int b = 0; b < blocks; ++b) {
        if (rand() % 2 == 0) {
            // (aa|ab|ac)*(ac|bc|cc)bb
            int xCount = rand() % 5;
            for (int i = 0; i < xCount; ++i) {
                result += xParts[rand() % 3];
            }
            result += yParts[rand() % 3];
            result += "bb";
        } else {
            // abacc* 
            result += "aba";
            result += "c";
            int cCount = rand() % 6;
            for (int i = 0; i < cCount; ++i)
                result += 'c';
        }
    }
    return result;
}

string generateRandomWord(int n) {
    string word = "";
    string alphabet = "abc";
    for (int i = 0; i < n; ++i) {
        word += alphabet[rand() % 3];
    }
    return word;
}

bool checkRegex(const string& word) {
    regex re("((aa|ab|ac)*(bc|cc|ac)bb|abacc*)*");
    return regex_match(word, re);
}

int transitionDFA(int state, char symbol) {
    int table[15][3] = {
        {10, 4, 2},   // abac
        {10, 4, 1},   // abaccc
        {10, 12, 1},  // abacc
        {10, 12, 12}, // epsilon
        {13, 3, 8},   // acb
        {13, 3, 13},  // acbcb
        {14, 14, 0},  // aba
        {11, 4, 12},  // ac
        {13, 5, 13},  // acbc
        {6, 12, 12},  // ab
        {14, 9, 7},   // a
        {14, 14, 7},  // aaa
        {13, 13, 8},  // aab
        {13, 13, 13}, // T
        {11, 12, 12}  // aa
    };
    int col = (symbol == 'a' ? 0 : symbol == 'b' ? 1 : 2);
    return table[state][col];
}

bool checkDFA(const string& word) {
    int state = 3; // стартовое состояние
    vector<int> acceptingStates = {0, 1, 2, 3};
    for (char c : word) {
        state = transitionDFA(state, c);
    }
    for (int acc : acceptingStates) {
        if (state == acc) return true;
    }
    return false;
}

set<int> moveNFA(const set<int>& states, char symbol) {
    set<int> result;
    for (int state : states) {
        switch (state) {
            case 0: // eps_b4
                if (symbol == 'a') { result.insert(1); result.insert(2); result.insert(3); }
                if (symbol == 'b') result.insert(2);
                if (symbol == 'c') result.insert(2);
                break;
            case 1: // a1_a3_a4
                result.insert(4); // a2_b1_c1
                break;
            case 2: // a5_b2_c4
                if (symbol == 'c') result.insert(5); // c2_c3_c5
                break;
            case 3: // a6
                if (symbol == 'b') result.insert(6); // b5
                break;
            case 4: // a2_b1_c1
                if (symbol == 'a') { result.insert(1); result.insert(2); }
                if (symbol == 'b') result.insert(2);
                if (symbol == 'c') result.insert(2);
                break;
            case 5: // c2_c3_c5
                if (symbol == 'b') result.insert(7); // b3
                break;
            case 6: // b5
                if (symbol == 'a') result.insert(8); // a7
                break;
            case 7: // b3
                if (symbol == 'b') result.insert(0); // eps_b4_c7
                break;
            case 8: // a7
                if (symbol == 'c') result.insert(9); // c6
                break;
            case 9: // c6_c7
                if (symbol == 'a') { result.insert(1); result.insert(2); result.insert(3); }
                if (symbol == 'b') result.insert(2);
                if (symbol == 'c') { result.insert(2); result.insert(9); }
                break;
        }
    }
    return result;
}

bool checkNFA(const string& word) {
    set<int> currentStates = {0}; // стартовое состояние eps_b4
    set<int> acceptingStates = {0, 9}; // eps_b4 и c6_c7 - финальные
    for (char c : word) {
        currentStates = moveNFA(currentStates, c);
    }
    for (int st : currentStates) {
        if (acceptingStates.count(st)) return true;
    }
    return false;
}

set<int> epsilonClosure(const set<int>& states, const map<int, set<int>>& epsilonTransitions) {
    set<int> closure = states;
    queue<int> q;
    for (int st : states) q.push(st);
    while (!q.empty()) {
        int cur = q.front(); 
        q.pop();
        if (epsilonTransitions.count(cur)) {
            for (int next : epsilonTransitions.at(cur)) {
                if (!closure.count(next)) {
                    closure.insert(next);
                    q.push(next);
                }
            }
        }
    }
    return closure;
}

set<int> moveSwitchingFA(const set<int>& states, char symbol,
                          const map<pair<int,char>, set<int>>& transitions,
                          const map<int, set<int>>& epsilonTransitions) {
    set<int> nextStates;
    for (int st : states) {
        auto it = transitions.find({st, symbol});
        if (it != transitions.end()) {
            nextStates.insert(it->second.begin(), it->second.end());
        }
    }
    return epsilonClosure(nextStates, epsilonTransitions);
}

bool checkSwitchingFA(const string& word) {
    map<pair<int,char>, set<int>> transitions;
    map<int,set<int>> epsilonTrans;

    epsilonTrans[0] = {1,7}; // S -> A0, B0

    // (aa|ab|ac)*
    transitions[{1,'a'}] = {2};
    transitions[{2,'a'}] = {1};
    transitions[{2,'b'}] = {1};
    transitions[{2,'c'}] = {1};

    transitions[{1,'a'}].insert(3);
    transitions[{1,'b'}].insert(3);
    transitions[{1,'c'}].insert(3);
    transitions[{3,'c'}] = {4}; 
    transitions[{4,'b'}] = {5};
    transitions[{5,'b'}] = {6}; 
    epsilonTrans[6] = {0};     

    // abacc*
    transitions[{7,'a'}] = {8};
    transitions[{8,'b'}] = {9};
    transitions[{9,'a'}] = {10};
    transitions[{10,'c'}] = {11};
    transitions[{11,'c'}] = {11};
    epsilonTrans[11] = {0};

    set<int> start = epsilonClosure({0}, epsilonTrans);
    set<int> accepting = {0}; // S — принимающее
    set<int> currentStates = start;
    for (char c : word) {
        currentStates = moveSwitchingFA(currentStates, c, transitions, epsilonTrans);
    }
    for (int st : currentStates) if (accepting.count(st)) return true;
    return false;
}

int main() {
    srand(time(0));

    int numWords = 10000;
    int maxLength = 100;

    int mismatchesDFA=0;
    int mismatchesNFA=0;
    int mismatchesSwitch=0;

    ofstream logFile("mismatches.txt");

    for (int i=0;i<numWords;i++) {
        string word;
        if (rand() % 2 == 0) {
            int length = rand()%maxLength + 10;
            word = generateRandomWord(length);
        } else {
            word = generateWord();
        }
        
        bool regexResult = checkRegex(word);
        if (regexResult) cout << word << endl;
        bool dfaResult = checkDFA(word);
        bool nfaResult = checkNFA(word);
        bool switchResult = checkSwitchingFA(word);

        if (regexResult != dfaResult) mismatchesDFA++;
        if (regexResult != nfaResult) mismatchesNFA++;
        if (regexResult != switchResult) mismatchesSwitch++;

        if (regexResult != dfaResult || regexResult != nfaResult || regexResult != switchResult) {
            logFile << "Word: " << word
                    << " | Regex: " << regexResult
                    << " | DFA: " << dfaResult
                    << " | NFA: " << nfaResult
                    << " | SwitchingFA: " << switchResult << endl;
        }
    }
    logFile.close();
    cout <<  "DFA mismatches: " << mismatchesDFA << endl;
    cout <<  "NFA mismatches: " << mismatchesNFA << endl;
    cout << "Switching FA mismatches: " << mismatchesSwitch << endl;

    return 0;
}
