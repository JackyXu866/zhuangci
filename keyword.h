// class to store keywords

#ifndef KEYWORD_H
#define KEYWORD_H

#include <vector>
#include <string>
#include <functional>
#include <utility>

#include "util.h"

// 关键词的详细信息，用来返回给接口
class Description
{
public:
    Description() { adjective = L""; };
    std::wstring adjective;
    std::tm *time;
    std::wstring word;
};

// 关键词类
class Keyword
{
public:
    Keyword(std::wstring word);
    void addSimilarWord(std::wstring similarWord) { similarWords.push_back(similarWord); }
    void setAction(std::function<int(std::shared_ptr<Description>)> action) { this->action = action; }
    void addAdjective(std::wstring adjective);
    void addPrevKeyword(std::shared_ptr<Keyword> prevKeyword) { prevKeywords.push_back(prevKeyword); }
    void addNextKeyword(std::shared_ptr<Keyword> nextKeyword) { nextKeywords.push_back(nextKeyword); }

    std::pair<int, int> match(std::wstring &sentence);
    bool findAdj(std::wstring &sentence, int pos, std::shared_ptr<Description> description);

    std::wstring getWord() const { return word; }
    std::vector<std::wstring> getSimilarWords() const { return similarWords; }
    std::wstring getSimilarWord(int index) const { return similarWords[index]; }
    std::vector<std::wstring> getAdjectiveList() const { return adjectiveList; }
    std::vector<std::shared_ptr<Keyword>> getPrevKeywords() const { return prevKeywords; }
    std::vector<std::shared_ptr<Keyword>> getNextKeywords() { return nextKeywords; }

    int performAction(std::shared_ptr<Description> input) const;

    int ADJ_GAP_SPACE;   // number of characters between adjective and keyword
    int ADJ_SEARCH_TYPE; // 2: both, 0: forward, 1: backward
    bool head, tail, mustAdj;

private:
    std::wstring word;
    std::vector<std::wstring> similarWords;
    std::vector<std::wstring> adjectiveList;
    size_t maxAdjLen;
    std::function<int(std::shared_ptr<Description>)> action; // action to be performed when keyword is detected, takes in adjective

    // 多轮
    // 匹配逻辑：在从数据库读取时获取每个关键词前置及后置关键词
    // 在匹配时，如果当前关键词匹配成功，则将待匹配关键词列表（tempKeywords）等于当前关键词的后置关键词列表(nextKeywords)
    // 如果没有匹配到任意关键词，则将tempKeywords清空
    // 匹配到没有后置的关键词，因为nextKeywords本就是空的，所以tempKeywords也会更新成空的
    // 没有前置的关键词，即head，每轮均会匹配，因为他们加入了headKeywords
    std::vector<std::shared_ptr<Keyword>> prevKeywords;
    std::vector<std::shared_ptr<Keyword>> nextKeywords;
    int repeat;
};

#endif // KEYWORD_H