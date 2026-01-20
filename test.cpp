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
#include <unordered_map>
#include <unordered_set>
#include <array>

using namespace std;

string generateWord(int maxBlocks = 5) {
    string result = "";
    string xParts[] = {"aa", "ab", "ac"}; 
    string yParts[] = {"bc", "cc", "ac"};  
    int blocks = rand() % (maxBlocks + 1);
    for (int b = 0; b < blocks; ++b) {
        if (rand() % 2 == 0) {
            int xCount = rand() % 5;
            for (int i = 0; i < xCount; ++i)
                result += xParts[rand() % 3];
            result += yParts[rand() % 3];
            result += "bb";
        } else {
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
    for (int i = 0; i < n; ++i)
        word += alphabet[rand() % 3];
    return word;
}

bool checkRegex(const string& word) {
    static const regex re("((aa|ab|ac)*(bc|cc|ac)bb|abacc*)*");
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

int ci(char ch){ return ch=='a'?0:(ch=='b'?1:2); }
bool checkPKA(const unordered_map<string, array<unordered_set<string>, 3>>& transitions, 
                const string& startState, 
                const unordered_set<string>& acceptingStates, 
                const string& word) {
    unordered_set<string> currentStates = {startState};
    for(char symbol : word) {
        unordered_set<string> nextStates;
        int idx = ci(symbol);
        for(const string& st : currentStates) {
            auto it = transitions.find(st);
            if(it == transitions.end()) continue;
            for(const string& t : it->second[idx]) 
                nextStates.insert(t);
        }
        currentStates = nextStates;
        if(currentStates.empty()) return false;
    }
    for(const string& st : currentStates)
        if(acceptingStates.count(st)) return true;
    return false;
}

void PKA(unordered_map<string, array<unordered_set<string>,3>> &a1,
                    unordered_set<string> &acc1,
                    unordered_map<string, array<unordered_set<string>,3>> &a2,
                    unordered_set<string> &acc2,
                    string &start1, string &start2)
{
    start1 = "a1";
    acc1 = {"a1","a8","a10"};
    a1["a1"][ci('a')].insert("a2"); a1["a1"][ci('a')].insert("a3"); a1["a1"][ci('a')].insert("a4");
    a1["a1"][ci('b')].insert("a3"); a1["a1"][ci('c')].insert("a3");

    char chars1[] = {'a','b','c'};
    for(char s : chars1) a1["a2"][ci(s)].insert("a6");

    a1["a3"][ci('c')].insert("a7");
    a1["a4"][ci('b')].insert("a5");
    a1["a5"][ci('a')].insert("a8");

    a1["a6"][ci('a')].insert("a2");
    a1["a6"][ci('a')].insert("a3");
    a1["a6"][ci('b')].insert("a3");
    a1["a6"][ci('c')].insert("a3");

    a1["a7"][ci('b')].insert("a9");
    a1["a8"][ci('a')].insert("a8"); a1["a8"][ci('c')].insert("a8"); a1["a8"][ci('c')].insert("a10");
    a1["a9"][ci('b')].insert("a1");
    a1["a10"][ci('a')].insert("a2"); a1["a10"][ci('a')].insert("a3"); a1["a10"][ci('a')].insert("a4");
    a1["a10"][ci('b')].insert("a3"); a1["a10"][ci('c')].insert("a3");


    
    start2 = "b1";
    acc2 = {"b1", "b8"};

    a2["b1"][ci('a')].insert("b2"); a2["b1"][ci('a')].insert("b3"); a2["b1"][ci('a')].insert("b9");
    a2["b1"][ci('b')].insert("b2"); a2["b1"][ci('b')].insert("b9");
    a2["b1"][ci('c')].insert("b9");

    a2["b2"][ci('a')].insert("b6");
    a2["b2"][ci('b')].insert("b6");
    a2["b2"][ci('c')].insert("b6");

    a2["b3"][ci('b')].insert("b5");

    a2["b4"][ci('b')].insert("b1");

    a2["b5"][ci('a')].insert("b7");

    a2["b6"][ci('a')].insert("b2"); a2["b6"][ci('a')].insert("b9");
    a2["b6"][ci('b')].insert("b2"); a2["b6"][ci('b')].insert("b9");
    a2["b6"][ci('c')].insert("b9");

    a2["b7"][ci('c')].insert("b8");

    a2["b8"][ci('a')].insert("b2"); a2["b8"][ci('a')].insert("b3"); a2["b8"][ci('a')].insert("b9");
    a2["b8"][ci('b')].insert("b2"); a2["b8"][ci('b')].insert("b9");
    a2["b8"][ci('c')].insert("b8"); a2["b8"][ci('c')].insert("b9");

    a2["b9"][ci('a')].insert("b10");
    a2["b9"][ci('b')].insert("b10");
    a2["b9"][ci('c')].insert("b10");

    a2["b10"][ci('b')].insert("b4");
}


int main() {
    srand(time(0));
    unordered_map<string, array<unordered_set<string>,3>> a1, a2;
    unordered_set<string> acc1, acc2;
    string start1, start2;
    PKA(a1, acc1, a2, acc2, start1, start2);

    int totalEqual = 0, mismatches = 0;
    int numWords = 100000;
    int maxLength = 100;

    for(int i=0;i<numWords;++i){
        string word;
        if (rand() % 2 == 0) {
            int length = rand() % maxLength + 1;
            word = generateRandomWord(length);
        } else {
            word = generateWord();
        }
        bool r = checkRegex(word);
        bool dfa = checkDFA(word);
        bool nfa = checkNFA(word);
        bool a1_accept = checkPKA(a1,start1,acc1,word);
        bool a2_accept = checkPKA(a2,start2,acc2,word);
        bool pka = a1_accept && a2_accept;

        if((pka == r) & (dfa == r) & (nfa == r)) totalEqual++;
        else {
            mismatches++;
            cout<<"Word: "<<word<<" | Regex: "<<r<<" | DFA: "<<dfa<<" | NFA: "<<nfa<<" | PKA: "<<pka<<a1_accept<<a2_accept<<endl;
        }
    }

    cout<<"Всего слов: "<<numWords<<endl;
    cout<<"Совпадений: "<<totalEqual<<endl;
    cout<<"Несовпадений: "<<mismatches<<endl;

    return 0;
}
