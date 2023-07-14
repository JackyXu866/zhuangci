﻿#define _CRT_SECURE_NO_WARNINGS

/**
 * @brief 关键词类，用于匹配句子中的关键词并执行相应的动作
 *
 */
#include "keyword.h"

Keyword::Keyword(std::wstring word) : word(word)
{
    similarWords = std::vector<std::wstring>();
    similarWords.push_back(word);
    adjectiveList = std::vector<std::wstring>();
    maxAdjLen = 0;
    action = nullptr;
    ADJ_SEARCH_TYPE = 0;
    ADJ_GAP_SPACE = 3;

    prevKeywords = std::vector<std::shared_ptr<Keyword>>();
    nextKeywords = std::vector<std::shared_ptr<Keyword>>();
    head = true;
    tail = true;
    mustAdj = false;
    repeat = 1;
}

void Keyword::addSimilarWord(std::wstring similarWord)
{
    similarWords.push_back(similarWord);
}

void Keyword::setAction(std::function<int(std::shared_ptr<Description>)> action)
{
    this->action = action;
}

void Keyword::addAdjective(std::wstring adjective)
{
    adjectiveList.push_back(adjective);
    maxAdjLen = std::max(maxAdjLen, adjective.length());
}

void Keyword::addPrevKeyword(std::shared_ptr<Keyword> prevKeyword)
{
    prevKeywords.push_back(prevKeyword);
}

void Keyword::addNextKeyword(std::shared_ptr<Keyword> nextKeyword)
{
    nextKeywords.push_back(nextKeyword);
}

bool Keyword::findAdj(std::wstring &sentence, int pos, std::shared_ptr<Description> description)
{
    int begPos, adjLen;
    // 根据ADJ_SEARCH_TYPE调整搜索范围
    switch (ADJ_SEARCH_TYPE)
    {
    case 2:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = (maxAdjLen + ADJ_GAP_SPACE) * 2 + word.length();
        break;
    case 0:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = maxAdjLen + ADJ_GAP_SPACE;
        break;
    case 1:
        begPos = pos + word.length();
        adjLen = maxAdjLen + ADJ_GAP_SPACE;
        break;
    default:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = (maxAdjLen + ADJ_GAP_SPACE) * 2 + word.length();
        break;
    }
    // 如果超出范围，调整搜索范围
    if (begPos < 0)
    {
        adjLen += begPos;
        begPos = 0;
    }
    if ((size_t)(begPos + adjLen) > sentence.length())
    {
        adjLen -= begPos + adjLen - sentence.length();
    }

    std::wstring tmp = sentence.substr(begPos, adjLen);

    for (std::wstring adjective : adjectiveList)
    {
        std::wregex adjectiveMatch(adjective);
        if (std::regex_search(tmp, adjectiveMatch))
        {
            // std::wcout << L"Matched adjective: " << adjective << std::endl;
            description->adjective = adjective;
            return true;
        }
    }

    return false;
}

std::pair<int, int> Keyword::match(std::wstring &sentence)
{
    std::wsmatch match;
    for (int i = 0; i < similarWords.size(); i++)
    {
        std::wstring similarWord = similarWords[i];
        std::wregex similarWordMatch(similarWord);
        if (std::regex_search(sentence, match, similarWordMatch))
        {
            // std::wcout << L"Matched keyword: " << similarWord << std::endl;

            int pos = match.position(0);

            return std::make_pair(pos, i);
        }
    }
    return std::make_pair(-1, -1);
}

std::wstring Keyword::getWord() const
{
    return word;
}

std::vector<std::wstring> Keyword::getSimilarWords() const
{
    return similarWords;
}

std::wstring Keyword::getSimilarWord(int index) const
{
    return similarWords[index];
}

std::vector<std::wstring> Keyword::getAdjectiveList() const
{
    return adjectiveList;
}

std::vector<std::shared_ptr<Keyword>> Keyword::getPrevKeywords() const
{
    return prevKeywords;
}

std::vector<std::shared_ptr<Keyword>> Keyword::getNextKeywords()
{
    return nextKeywords;
}

int Keyword::performAction(std::shared_ptr<Description> input) const
{
    std::wcout << L"技能名称: " << word << std::endl;
    std::wcout << L"具体词语: " << input->word << std::endl;
    std::wcout << L"技能时间: " << std::asctime(input->time);
    if (input->adjective != L"")
        std::wcout << L"技能形容/类别: " << input->adjective << std::endl;
    else
        std::wcout << L"技能形容/类别: " << L"无" << std::endl;
    if (head != true)
        std::wcout << L"经过前置判定" << std::endl;
    std::wcout << std::endl;

    if (action != nullptr)
        action(input);

    return 0;
}
