#include "Stack.hpp"
#include "doctest.h"
#include <string>
#include <ctime>
#include <vector>
using namespace std;

TEST_CASE("TOP-POP-PUSH"){
    Stack *s = create();

    PUSH(s,"element 1");
    PUSH(s, "2");
    PUSH(s, "0");


    string rslt{POP(s, nullptr)};
    cout << rslt << endl;
    CHECK(rslt == "0");
    string rslt2{POP(s, nullptr)};
    CHECK(rslt2 == "2");
    string rslt3{POP(s, nullptr)};
    CHECK(rslt3 == "element 1");

    /********* Random Push-Pop *************/
    srand(time(0)); /* Make a random generator*/
    string chars = "abcdefghijklmnopqrstuvwxyz"
                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                   "1234567890"
                   "!@#$%^&*()_+=/~><.";

    vector<string> words;
    int wordsAmount = 1 + (rand() % 100);
    int randLen = 1 + (rand() % 100);
    for (int i = 0; i < wordsAmount; ++i) {
        string randWord;
        for (int j = 0; j < randLen; ++j) {
            int randPos = rand() % chars.length();
            randWord += chars[randPos];
        }
        cout << randWord << endl;
        PUSH(s,randWord.c_str());
        words.push_back(randWord);
    }

    /*CHECK POP*/
    for (int i = 0; i < wordsAmount; ++i) {
        string randResult{POP(s, nullptr)};
        CHECK(randResult == words[wordsAmount-i-1]);
    }

}