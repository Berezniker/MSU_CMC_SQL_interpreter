
#ifndef SQL_INTERPRETER_WHERE_CONDITION_H
#define SQL_INTERPRETER_WHERE_CONDITION_H


#include <string>
#include <regex>
#include <set>


#define _LESS 0
#define _EQ_LESS 1
#define _MORE 2
#define _EQ_MORE 2

class Where_condition
{
public:
    bool condition(const std::string &value)
    {
        if ((!compare_set.empty()) xor not_lex)
            return compare_set.find(value) != compare_set.end();
        if (exist_pattern) {
            return std::regex_match(value, pattern);
        }
//        if(!expr.empty()){
//            return count_expr(expr, value);
//        }
        return true;
    }

    void set_set(const std::string &value)
    {
        compare_set.insert(value);
    }

    void set_pattern(const std::string &pattern)
    {
        std::string reg_pattern;
        // перевожу sql'ные регулярки в человеческие
        for (char letter : pattern) {
            if (letter == '%') {
                reg_pattern += '*';
                continue;
            }
            if (letter == '_') {
                reg_pattern += ".";
                continue;
            }
            reg_pattern += letter;
        }
        this->pattern = reg_pattern;
        exist_pattern = true;
    }

//    void set_logical_exp(Identifier item)
//    {
//        expr.push_back(item);
//    }

    void set_not()
    {
        not_lex = true;
    }
private:
    std::set<std::string> compare_set;
    bool not_lex = false;
    std::regex pattern;
    bool exist_pattern = false;
//    std::vector<Identifier> expr;
//
//    bool count_expr(std::vector<Identifier>, std::string value){
//
//    }
};


#endif //SQL_INTERPRETER_WHERE_CONDITION_H
