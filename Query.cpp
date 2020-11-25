#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;
/////////////////////////////////////////////////////////
shared_ptr<QueryBase> QueryBase::factory(const string &s)
{
    int count = 0;
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == ' ')
        {
            count++;
        }
    }   
    string sText;
    string s1;
    string s2;
    bool ans = !(s1.empty() && s2.empty());
    if (s.substr(0, 3) == "AND" && s.at(3) == ' '&& count == 2 )
    {
        sText = s.substr(4, s.length());
        size_t i = sText.find(" ");
        s1 = sText.substr(0, i);
        s2 = sText.substr(i + 1, s.length());
        return std::shared_ptr<QueryBase>(new AndQuery(s1, s2));
    }
    else if (s.substr(0, 2) == "OR" && s.at(2) == ' ' && count  == 2)
    {
        sText = s.substr(3, s.length());
        size_t i = sText.find(" ");
        s1 = sText.substr(0, i);
        s2 = sText.substr(i + 1, s.length());
        return std::shared_ptr<QueryBase>(new OrQuery(s1, s2));
    }
    else if (s.substr(0, 2) == "AD" && s.at(2) == ' '&&count  == 2)
    {
        sText = s.substr(3, s.length());
        size_t i = sText.find(" ");
        s1 = sText.substr(0, i);
        s2 = sText.substr(i + 1, s.length());
        return std::shared_ptr<QueryBase>(new AdjacentQuery(s1, s2));
    }
    else if (s.find(" ") == -1)
    {
        return std::shared_ptr<QueryBase>(new WordQuery(s));
        //exit(1);
    }
    else
    {
        return std::shared_ptr<QueryBase>(new WordQuery("Unrecognized search"));
    }
}
QueryResult AndQuery::eval(const TextQuery &text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>();
    set_intersection(left_result.begin(), left_result.end(),
                     right_result.begin(), right_result.end(),
                     inserter(*ret_lines, ret_lines->begin()));

    return QueryResult(rep(), ret_lines, left_result.get_file());
}
QueryResult OrQuery::eval(const TextQuery &text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>(left_result.begin(), left_result.end());
    ret_lines->insert(right_result.begin(), right_result.end());
    return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult AdjacentQuery::eval(const TextQuery &text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines1 = make_shared<set<line_no>>(left_result.begin(), left_result.end());
    auto ret_lines = make_shared<set<line_no>>(right_result.begin(), right_result.end());
    auto ret = make_shared<set<line_no>>();
    for (int i : *ret_lines1)
    {
        for (int j : *ret_lines)
        {
            if (i == j + 1)
            {
                ret->insert(i);
                ret->insert(j);
                break;
            }
            else if (i == j - 1)
            {
                ret->insert(j);
                ret->insert(i);
                break;
            }
        }
    }
    return QueryResult(rep(), ret, left_result.get_file());
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::ostream &print(std::ostream &os, const QueryResult &qr)
{
    string s = qr.sought;
    size_t i = s.find(" ");
    if (s == "Unrecognized search")
    {
        os<< "Unrecognized search";
    }
   else if (s.substr(0, i) == "AD")
    {
        int count =  0;
        std::vector <int> v;
        for (auto i : *qr.lines)
        {
           v.push_back(i);
        }
       for(int i = 0 ;i < v.size() -1; i++){
           if (v[i] + 1 == v[i+1])
           {
               count++;
           }
       }
        os << "\"" << qr.sought << "\""
           << " occurs " << count << " times:" << std::endl;
        for (auto num : *qr.lines)
        {
            os << "\t(line " << num + 1 << ") "
               << *(qr.file->begin() + num) << std::endl;
        }
    }
    else
    {
        os << "\"" << qr.sought << "\""
           << " occurs " << qr.lines->size() << " times:" << std::endl;
        for (auto num : *qr.lines)
        {
            os << "\t(line " << num + 1 << ") "
               << *(qr.file->begin() + num) << std::endl;
        }
    }
    return os;
}
/////////////////////////////////////////////////////////