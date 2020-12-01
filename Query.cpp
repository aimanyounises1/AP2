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
vector<string> split(const string &parse, string delimiter)
{
    vector<string> list;
    string s = parse;
    size_t pos = 0;
    string token;
    while ((pos = s.find(delimiter)) != string::npos)
    {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}
shared_ptr<QueryBase> QueryBase::factory(const string &s)
{
    vector<string> v;
    string sText;
    string s1;
    string s2;
    string op;
    int count = 0;
    if (s.find("  ")!=string::npos)
    {
        v = split(s, "  ");
        if(v.size() == 3){
        op = v[0];
        s1 = v[1];
        s2 = v[2];
        }
        count  = 1;
    }
    else if (s.find(" ")!= string::npos && count == 0)
    {
        v = split(s," ");
        if(v.size() == 3){
        op = v[0];
        s1 = v[1];
        s2 = v[2];
        }
    }
    else
    {
        return std::shared_ptr<QueryBase>(new WordQuery(s));
    }

    bool ans = !(s1.empty() && s2.empty());
    if (op == "AND" && !s1.empty() && !s2.empty())
    {

        return std::shared_ptr<QueryBase>(new AndQuery(s1, s2));
    }
    else if (op == "OR" && !s1.empty() && !s2.empty())
    {

        return std::shared_ptr<QueryBase>(new OrQuery(s1, s2));
    }
    else if (op == "AD" && !s1.empty() && !s2.empty())
    {

        return std::shared_ptr<QueryBase>(new AdjacentQuery(s1, s2));
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
        os << "Unrecognized search \n";
    }
    else if (s.substr(0, i) == "AD")
    {
        int count = 0;
        std::vector<int> v;
        for (auto i : *qr.lines)
        {
            v.push_back(i);
        }
        for (int i = 0; i < v.size() - 1; i++)
        {
            if (v[i] + 1 == v[i + 1])
            {
                count++;
            }
        }
        os << "\"" << qr.sought << "\""
           << " occurs " << count << " times:" << std::endl;
        count = 0;
        for (int i = 0; i < v.size(); i++)
        {
            if (v[i] + 1 == v[i + 1])
            {
                if (i + 1 < v.size() - 1)
                {
                    os << "\t(line " << v[i] + 1 << ") "
                       << *(qr.file->begin() + v[i]) + "\n"
                       << "\t(line " << v[i + 1] + 1 << ") " << *(qr.file->begin() + v[i + 1]) + "\n"
                       << endl;
                }
                else
                {
                    os << "\t(line " << v[i] + 1 << ") "
                       << *(qr.file->begin() + v[i]) + "\n"
                       << "\t(line " << v[i + 1] + 1 << ") " << *(qr.file->begin() + v[i + 1]) << endl;
                }
                //i++;
            }
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
/* 2 3 4 5
  line 2
  line 3

  line 3 
  line 4

  line 4
  line 5      
          return std::shared_ptr<QueryBase>(new AndQuery(s1, s2));
                  return std::shared_ptr<QueryBase>(new OrQuery(s1, s2));
                          return std::shared_ptr<QueryBase>(new AdjacentQuery(s1, s2));
                                  return std::shared_ptr<QueryBase>(new WordQuery(s));
                                          return std::shared_ptr<QueryBase>(new WordQuery("Unrecognized search"));




              */