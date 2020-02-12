#include <iostream>  // std::cout, std::endl, std::ostream
#include <string>    // std::string: push_back(), size(), clear()
#include <iomanip>   // std::setw(), std::left
#include <utility>   // std::pair
#include <vector>    // std::vector: emplace_pack(), push_back(), size(), begin(), end(), clear()
#include <stack>     // std::stack: push(), top(), pop(), clear()
#include <set>       // std::set<std::string>: insert(), clear()
#include <iterator>  // std::iterator
#include <sstream>   // std::istringstream: putback(), get()
#include <algorithm> // find()
#include <ctype.h>   // isspace(), isalpha(), isdigit()
      // functions for semantic analysis and for working with tables
#include <exeption/exception.h> // AnalyzeError(), std::exception


#include "analyze.h" // прототипы всех функций, описанных в этом файле

#define ACTIVATE true
#define OFF     false
/* configuration */
#define LEXICAL  ACTIVATE /* лексический анализ */
#define SYNTAX   ACTIVATE /* синтаксический анализ: без LEXICAL ACTIVATE не запустится */
#define SEMANTIC ACTIVATE /* семантический анализ: без SYNTAX ACTIVATE не запустится */
#define EXECUTOR ACTIVATE /* исполнение запроса: без SEMANTIC ACTIVATE не запустится */
#define DEBUG    ACTIVATE /* отладочная печать */


/* -------------------- class Identifier -------------------- */

Identifier::Identifier() = default;


Identifier::Identifier(type_of_lex type, const std::string &name) : ident_type(type), ident_name(name)
{}


bool Identifier::operator==(const std::string &str_name) const noexcept
{
    return ident_name == str_name;
}


std::ostream &operator<<(std::ostream &sout, Identifier &ident)
{
    sout << '(' << ident.ident_name << ", " << Analyze::TABLE_OF_LEXEME[ident.ident_type] << ");";
    return sout;
}



/* -------------------- class Analyze ---------------------- */

// объявляем статические переменные
const char *Analyze::TABLE_OF_LEXEME[] =
        {
                "LEX_NULL", "LEX_SELECT", "LEX_FROM", "LEX_INSERT", "LEX_INTO", "LEX_UPDATE", "LEX_SET",
                "LEX_DELETE", "LEX_CREATE", "LEX_TABLE", "LEX_TEXT", "LEX_LONG", "LEX_DROP", "LEX_WHERE",
                "LEX_NOT", "LEX_LIKE", "LEX_IN", "LEX_AND", "LEX_OR", "LEX_ALL", "LEX_FIN", "LEX_COMMA",
                "LEX_STAR", "LEX_QUOTE", "LEX_OPEN_BRACKET", "LEX_CLOSE_BRACKET", "LEX_PLUS", "LEX_MINUS",
                "LEX_SLASH", "LEX_PERCENT", "LEX_EQUAL", "LEX_GREATER", "LEX_LESS", "LEX_GREATER_OR_EQUAL",
                "LEX_LESS_OR_EQUAL", "LEX_NOT_EQUAL", "LEX_NUM", "LEX_ID", "LEX_STRING", nullptr
        };

const char *Analyze::TABLE_OF_KEYWORDS[] =
        {
                "SELECT", "FROM", "INSERT", "INTO", "UPDATE", "SET", "DELETE", "CREATE", "TABLE",
                "TEXT", "LONG", "DROP", "WHERE", "NOT", "LIKE", "IN", "AND", "OR", "ALL", nullptr
        };

const char *Analyze::TABLE_OF_DELIMS[] =
        {
                ";", ",", "*", "\'", "(", ")", "+", "-", "/", "%", "=", ">", "<", "<=", ">=", nullptr
        };

int Analyze::table_access_key;

std::string Analyze::command;

std::vector<Identifier> Analyze::TID;

std::vector<Identifier> Analyze::POLIS;

std::vector<Identifier> Analyze::TOKENS;

Table Analyze::selected_table = Table(); //todo constructor with name for this table

bool  Analyze::table_is_actual = false;


Analyze::Analyze(int key, const std::string &query)
{
    Analyze::table_access_key = key;
    Analyze::command = query;
    /* инициализация в теле цикла, т.к. переменные статические */
}

std::string Analyze::get_table_text(){
    return Analyze::table_is_actual? selected_table.to_string(): "";
}


Analyze::~Analyze()
{
    // отчищаем статический члены класса:
    Analyze::command.clear(); // буфер команды
    Analyze::TOKENS.clear();  // таблицу токенов
    Analyze::POLIS.clear();   // таблицу ПОЛИЗа
    Analyze::TID.clear();     // таблицу идентификаторов
}


void Analyze::start()
{
#if DEBUG
    std::cout << "command:\n" << Analyze::command << std::endl;
#endif

#if LEXICAL
    Scanner().lexical_analyze();      // запускаем лексический анализатор
#if SYNTAX
    Parser().syntactic_analyze(); // запускаем синтаксический + семантический анализатор
#if SEMANTIC && EXECUTOR
    Executor().interpreter(); // запускаем перевод в ПОЛИЗ + исполнитель запроса
#endif
#endif
#if DEBUG
    std::cout << "Analyze succeeded" << std::endl;
#endif
#endif
}


/* --------------------- class Scanner --------------------- */

Analyze::Scanner::Scanner() : sin(Analyze::command)
{
    // сразу проверяем завершающий символ
    if (Analyze::command.back() != ';') {
        throw AnalyzeError("LEXICAL ERROR: no semicolon at the end of the query",
                           Analyze::command, ";");
    }
}


const Identifier Analyze::Scanner::get_lex()
{
    enum state
    {
        START, IDENTIFIER, NUMBER, COMMENT, MINUS, STRING, COMPARE_SIGN, NOT_EQUAL, ERROR
    } current_state = START;
    std::string lex;
    int pos;
    static bool is_first_quote = false;
    static bool is_second_quote = false;
    std::string error_description = "LEXICAL ERROR: ";

    while (true) {
        get_char();
        switch (current_state) {
            case START: // начальное состояние
                if (is_first_quote) {
                    sin.putback(c); //вернули символ, чтобы потом не прочитать его случайно еще раз
                    current_state = STRING;
                    is_first_quote = false; // открывающая кавычка прошла => далее будет закрывающая
                    is_second_quote = true;
                } else if (isspace(c)) { // пропускаем все пробельные последовательности
                    break;
                } else if (isalpha(c) ||
                           c == '_') { // встретили букву => имеем дело с идентификатором и нужно его дальше собирать
                    lex.push_back(c); //собираем строку - имя текущей лексемы (здесь и далее)
                    current_state = IDENTIFIER;
                } else if (isdigit(c)) { //встретили цифру => имеем дело с числом
                    lex.push_back(c); // для красивого вывода ошибок
                    current_state = NUMBER;
                } else if (c == '#') { //комментарий начинается => нужно удалить весь текст
                    current_state = COMMENT;
                } else if (c == '<' || c == '>') { //встретили знак <, > => учитываем, что они могут являться
                    //частью составных знаков сравнения
                    lex.push_back(c);
                    current_state = COMPARE_SIGN;
                } else if (c == ';') {
                    return Identifier(LEX_FIN, ";"); //конечная лексема ; обязательно будет присутствовать
                } else if (c == '!') { //встретили ! => далее может быть только = (лексема !=)
                    lex.push_back(c);
                    current_state = NOT_EQUAL;
                } else if (c == '-') {
                    lex.push_back(c);
                    current_state = MINUS;
                } else if (c == '\'') {
                    lex.push_back(c);
                    pos = look(lex, TABLE_OF_DELIMS);
                    is_first_quote = !is_first_quote && !is_second_quote;
                    is_second_quote = false;
                    return Identifier((type_of_lex) (pos + (int) LEX_FIN - 1), lex);
                } else { // если встретили любой другой символ, определяем, принадлежит ли он алфавиту допустимых символов
                    lex.push_back(c);
                    if (pos = look(lex, TABLE_OF_DELIMS)) { //просматриваем таблицу разделителей
                        return Identifier((type_of_lex) (pos + (int) LEX_FIN - 1), lex);
                    } else {
                        //выбрасываем исключение, если не находим такого разделителя
                        error_description.append("symbol out of the alphabet");
                        current_state = ERROR;
                    }
                }
                break;

            case IDENTIFIER: // состояние для считывания идентификатора
                if (isalpha(c) || isdigit(c) || c == '_') { //символы, из которых мождет состоять идентификатор
                    lex.push_back(c);
                } else { // закончился идентификатор => выяснить, является он пользовательским или служебным
                    sin.putback(c);
                    if (pos = look(lex, TABLE_OF_KEYWORDS)) {
                        return Identifier((type_of_lex) pos,
                                          lex); //нашелся в таблице ключевых слов => является служебным
                    } else { //иначе является пользовательским
                        put_to_TID(lex); // заносим в TID сразу
                        return Identifier(LEX_ID, lex);
                    }
                }
                break;

            case NUMBER: //встретили цифру => продолжаем считывать число
                if (isdigit(c)) {
                    lex.push_back(c);
                } else {
                    if (isalpha(c)) {// буква точно не может идти
                        // (принимаем соглашение о том, что 1a - некорректное имя таблицы)
                        lex.push_back(c);
                        error_description.append("incorrect identifier");
                        current_state = ERROR;
                    } else { //если встретили не букву => нет никаких ошибок, нужно вернуть используемую числовую константу
                        sin.putback(c);
                        return Identifier(LEX_NUM, lex);
                    }
                }
                break;

            case COMMENT: // комментарий
                while (c != ';') { // попали сюда, если #<>, либо -- <>
                    get_char(); //просто считываем до ;
                }
                sin.putback(c);
                current_state = START;
                break;

            case MINUS:
                if (c == '-') { // потворный минус => комменатрий ?
                    lex.push_back(c);
                    get_char();
                    if (c == ' ') { // если встретили комбинацию -- <>
                        current_state = COMMENT;
                    } else {
                        lex.push_back(c);
                        error_description.append("missing space in <-- > comment");
                        current_state = ERROR;
                    }
                } else {
                    sin.putback(c); // вернули в строку символ 
                    pos = look(lex, TABLE_OF_DELIMS); // нашли позицию <-> в таблице 
                    return Identifier((type_of_lex) (pos + (int) LEX_FIN - 1), lex);
                }
                break;

            case STRING:
                while (c != '\'' && c != ';') {
                    lex.push_back(c);
                    get_char();
                }
                sin.putback(c);
                if (c == ';') {
                    error_description.append("close quote missing");
                    current_state = ERROR;
                } else /* c == '\'' */ {
                    return Identifier(LEX_STRING, lex);
                }
                break;

            case COMPARE_SIGN: //выясняем, встретились ли одиночные знаки <, > или <=, >=
                if (c == '=') {
                    lex.push_back(c);
                } else {
                    sin.putback(c);
                }
                pos = look(lex, TABLE_OF_DELIMS);
                return Identifier((type_of_lex) (pos + (int) LEX_FIN - 1), lex);

            case NOT_EQUAL: //проверяем: знак != возвращаем, иначе бросаем исключение
                if (c == '=') {
                    lex.push_back(c);
                    return Identifier(LEX_NOT_EQUAL, lex);
                } else {
                    sin.putback(c);
                    error_description.append("symbol out of the alphabet");
                    current_state = ERROR;
                }
                break;

            case ERROR:
                while (!isspace(c) && c != ';') {
                    lex.push_back(c);
                    get_char();
                }
                throw AnalyzeError(error_description, Analyze::command, lex);
        }
    }
}


inline void Analyze::Scanner::get_char()
{
    c = sin.get(); // считываем очередной символ из запроса
}


int Analyze::Scanner::look(const std::string &lex, const char **table)
{
    for (int i = 0; table[i] != nullptr; ++i) { // пока не закончится таблица <table>
        if (lex == table[i])                    // ищем в ней идентификатор <lex>
            return i + 1;
    }

    return 0;  // возвращаем 0, если не нашли
}


int Analyze::Scanner::put_to_TID(const std::string &lex)
{
    std::vector<Identifier>::iterator k;

    if ((k = std::find(Analyze::TID.begin(), Analyze::TID.end(), lex)) != Analyze::TID.end()) {
        return k - Analyze::TID.begin(); // ищём в TID лексему <lex>; возвращаем позицию
    }

    Analyze::TID.emplace_back(LEX_ID, lex); // иначе заносим лексему в TID
    return Analyze::TID.size() - 1;
}

void Analyze::Scanner::print_TABLE(std::vector<Identifier> TABLE, const char *table_name)
{
    if (!TABLE.empty()) {
        std::cout << std::endl;
        std::cout << "   vvv   " << table_name << "   vvv   " << std::endl;
        for (int k = 0; k != TABLE.size(); ++k) {
            std::cout << std::setw(3) << std::left << k << TABLE[k] << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cout << "***" << table_name << "is empty" << std::endl;
    }
}


void Analyze::Scanner::lexical_analyze()
{
    Identifier current_ID; // текущая лексема

    do {
        current_ID = get_lex();                // формируем новую лексему + заполняем Analyze::TID
        Analyze::TOKENS.push_back(current_ID); // заполняем Analyze::TOKENS
    } while (current_ID.ident_type != LEX_FIN);
#if DEBUG
    Analyze::Scanner::print_TABLE(Analyze::TOKENS, "TOKENS");
#endif
}


/* ---------------------- class Parser ---------------------- */

Analyze::Parser::Parser() = default;


inline void Analyze::Parser::get_lex()
{
    current_lex = Analyze::TOKENS[pos++];
}


void Analyze::Parser::syntactic_analyze()
{
    /**
     * the syntactic analyzer corresponds to the grammar 
     * described in the file assistive/SQL_BNF.txt
     */

    get_lex();
    SQL(); // SQL_preposition
    // проверяем завершающий символ
    if (current_lex.ident_type != LEX_FIN) {
        throw AnalyzeError("SYNTAX ERROR: expected token ;",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::SQL()
{
    if (current_lex.ident_type == LEX_SELECT) {
        get_lex();
        SELECT(); // SELECT_preposition
    } else if (current_lex.ident_type == LEX_INSERT) {
        get_lex();
        INSERT(); // INSERT_preposition
    } else if (current_lex.ident_type == LEX_UPDATE) {
        get_lex();
        UPDATE(); // UPDATE_preposition
    } else if (current_lex.ident_type == LEX_DELETE) {
        get_lex();
        DELETE(); // DELETE_preposition
    } else if (current_lex.ident_type == LEX_CREATE) {
        get_lex();
        CREATE(); // CREATE_preposition
    } else if (current_lex.ident_type == LEX_DROP) {
        get_lex();
        DROP();   //   DROP_preposition
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected token SELECT|INSERT|UPDATE|DELETE|CREATE|DROP",
                           Analyze::command, current_lex.ident_name);
    }
}


/* ---------- SELECT ---------- */

void Analyze::Parser::SELECT()
{
    /* SELECT */
    object_list();

    FROM();
    table_name();
#if SEMANTIC
    for (const auto &field_name : obj_list) {
        if (!object_exist(Analyze::table_access_key, table_head, field_name)) {
            throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                               Analyze::command, field_name);
        }
    }
#endif
    obj_list.clear();

    WHERE_clause();
}

void Analyze::Parser::object_list()
{
    if (current_lex.ident_type == LEX_STAR) {
        get_lex();
    } else {
        object_name();
        while (current_lex.ident_type == LEX_COMMA) {
            get_lex();
            object_name();
        }
    }
}

void Analyze::Parser::object_name()
{
    if (current_lex.ident_type != LEX_ID) {
        throw AnalyzeError("SYNTAX ERROR: expected token ID",
                           Analyze::command, current_lex.ident_name);
    }
#if SEMANTIC
    /**
     * пояснение:
     * std::set.insert() возвращает пару типа <итератор, bool>
     * в этой паре второй компонент second указывает на успешность операции:
     *  true  - новое значение успешно добавлено к множеству,
     *  false - значение уже присутствует в множестве
     */
    if (!obj_list.insert(current_lex.ident_name).second) {
        throw AnalyzeError("SEMANTIC ERROR: repeated description",
                           Analyze::command, current_lex.ident_name);
    }
#endif
    get_lex();
}

void Analyze::Parser::FROM()
{
    if (current_lex.ident_type != LEX_FROM) {
        throw AnalyzeError("SYNTAX ERROR: expected token FROM",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::table_name()
{
    if (current_lex.ident_type != LEX_ID) {
        throw AnalyzeError("SYNTAX ERROR: expected token ID",
                           Analyze::command, current_lex.ident_name);
    }
#if SEMANTIC
    if (!table_exist(Analyze::table_access_key, current_lex.ident_name)) {
        throw AnalyzeError("SEMANTIC ERROR: table with the given name does not exist",
                           Analyze::command, current_lex.ident_name);
    }
    table_head = current_lex.ident_name;
#endif
    get_lex();
}


/* ---------- INSERT ---------- */

void Analyze::Parser::INSERT()
{
    /* INSERT */
    INTO();
    table_name();

    open_bracket();

    object_value();
    while (current_lex.ident_type == LEX_COMMA) {
        get_lex();
        object_value();
    }

    close_bracket();

#if SEMANTIC
    try {
        check_param(table_access_key, table_head, actual_param);
    }
    catch (std::exception &err) {
        throw AnalyzeError(std::string("SEMANTIC ERROR: ") + err.what(),
                           Analyze::command, "(");
    }
#endif
    actual_param.clear();
}

void Analyze::Parser::INTO()
{
    if (current_lex.ident_type != LEX_INTO) {
        throw AnalyzeError("SYNTAX ERROR: expected token INTO",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::open_bracket()
{
    if (current_lex.ident_type != LEX_OPEN_BRACKET) {
        throw AnalyzeError("SYNTAX ERROR: expected token OPEN_BRACKET",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::close_bracket()
{
    if (current_lex.ident_type != LEX_CLOSE_BRACKET) {
        throw AnalyzeError("SYNTAX ERROR: expected token CLOSE_BRACKET",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::object_value()
{
    if (current_lex.ident_type == LEX_NUM) { // long_integer
        actual_param.emplace_back("LONG");
        get_lex();
    } else if (current_lex.ident_type == LEX_QUOTE) {
        actual_param.emplace_back("TEXT");
        string();
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected token STRING | long_integer",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::string()
{
    if (current_lex.ident_type != LEX_QUOTE) {
        throw AnalyzeError("SYNTAX ERROR: expected token \'",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();

    if (current_lex.ident_type != LEX_STRING) {
        throw AnalyzeError("SYNTAX ERROR: expected token STRING",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();

    if (current_lex.ident_type != LEX_QUOTE) {
        throw AnalyzeError("SYNTAX ERROR: expected token \'",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}


/* ---------- UPDATE ---------- */

void Analyze::Parser::UPDATE()
{
    /* UPDATE */
    table_name();

    SET();
    object_name();
    std::string obj_name = *(obj_list.begin());

#if SEMANTIC
    if (!object_exist(table_access_key, table_head, obj_name)) {
        throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                           Analyze::command, obj_name);
    }
#endif

    EQUAL();

#if SEMANTIC
    if (get_object_type(table_access_key, table_head, obj_name) != expression()) {
        throw AnalyzeError("SEMANTIC ERROR: type mismatch",
                           Analyze::command, obj_name);
    }
#else
    expression();
#endif
    obj_list.clear();

    WHERE_clause();
}

void Analyze::Parser::SET()
{
    if (current_lex.ident_type != LEX_SET) {
        throw AnalyzeError("SYNTAX ERROR: expected token SET",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::EQUAL()
{
    if (current_lex.ident_type != LEX_EQUAL) {
        throw AnalyzeError("SYNTAX ERROR: expected token =",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}


/* ---------- DELETE ---------- */

void Analyze::Parser::DELETE()
{
    /* DELETE */
    FROM();
    table_name();
    WHERE_clause();
}


/* ---------- CREATE ---------- */

void Analyze::Parser::CREATE()
{
    /* CREATE */
    TABLE();
    new_table_name();
    open_bracket();
    list_of_object_expression();
    close_bracket();
}

void Analyze::Parser::TABLE()
{
    if (current_lex.ident_type != LEX_TABLE) {
        throw AnalyzeError("SYNTAX ERROR: expected token TABLE",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::new_table_name()
{
    if (current_lex.ident_type != LEX_ID) {
        throw AnalyzeError("SYNTAX ERROR: expected token ID",
                           Analyze::command, current_lex.ident_name);
    }
#if SEMANTIC
    if (table_exist(table_access_key, current_lex.ident_name)) {
        throw AnalyzeError("SEMANTIC ERROR: table with the given name already exist",
                           Analyze::command, current_lex.ident_name);
    }
    table_head = current_lex.ident_name;
#endif
    get_lex();
}

void Analyze::Parser::list_of_object_expression()
{
    object_description();
    while (current_lex.ident_type == LEX_COMMA) {
        get_lex();
        object_description();
    }
}

void Analyze::Parser::object_description()
{
    new_object_name();
    object_type();
}

void Analyze::Parser::new_object_name()
{
    if (current_lex.ident_type != LEX_ID) {
        throw AnalyzeError("SYNTAX ERROR: expected token ID",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::object_type()
{
    if (current_lex.ident_type == LEX_LONG) {
        get_lex();
    } else if (current_lex.ident_type == LEX_TEXT) {
        get_lex();
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected token TEXT | LONG",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::unsigned_int()
{
    if (current_lex.ident_type != LEX_NUM) {
        throw AnalyzeError("SYNTAX ERROR: expected token NUMBER",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}


/* ---------- DROP ---------- */

void Analyze::Parser::DROP()
{
    /* DROP */
    TABLE();
    table_name();
}


/* --------- WHERE --------- */

void Analyze::Parser::WHERE_clause()
{
    WHERE();

    if (current_lex.ident_type == LEX_ALL) {
        get_lex();
        return;
    }

    // определение where-условия с просмотром наперёд:
    enum where_state
    {
        ERROR,
        SIMPLE,
        EXPRESSION,
        LOGICAL_EXPRESSION
    } where_condition = ERROR;

    for (int k = Analyze::Parser::pos; Analyze::TOKENS[k].ident_type != LEX_FIN; ++k) {
        type_of_lex lex_type = Analyze::TOKENS[k].ident_type;

        if (lex_type == LEX_GREATER ||
            lex_type == LEX_LESS ||
            lex_type == LEX_GREATER_OR_EQUAL ||
            lex_type == LEX_LESS_OR_EQUAL ||
            lex_type == LEX_NOT_EQUAL ||
            lex_type == LEX_EQUAL) {
            where_condition = LOGICAL_EXPRESSION;
            break;
        }
        if (lex_type == LEX_LIKE) {
            where_condition = SIMPLE;
            break;
        }
        if (lex_type == LEX_IN) {
            where_condition = EXPRESSION;
            break;
        }
    }

    switch (where_condition) {
        case SIMPLE:
            new_object_name();

            if (current_lex.ident_type == LEX_NOT) {
                get_lex();
            }
            LIKE();

            string();
            break;

        case EXPRESSION:
            expression();

            if (current_lex.ident_type == LEX_NOT) {
                get_lex();
            }
            IN();

            open_bracket();
            is_subquery() ? subquery() : list_of_constant();
            close_bracket();
            break;

        case LOGICAL_EXPRESSION:
            logical_expression();
            break;

        case ERROR:
            throw AnalyzeError("SYNTAX ERROR: incorrect WHERE-preposition",
                               Analyze::command, "WHERE");
    } // switch ()
}

void Analyze::Parser::WHERE()
{
    if (current_lex.ident_type != LEX_WHERE) {
        throw AnalyzeError("SYNTAX ERROR: expected token WHERE",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::LIKE()
{
    if (current_lex.ident_type != LEX_LIKE) {
        throw AnalyzeError("SYNTAX ERROR: expected token LIKE",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

void Analyze::Parser::IN()
{
    if (current_lex.ident_type != LEX_IN) {
        throw AnalyzeError("SYNTAX ERROR: expected token IN",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

int Analyze::Parser::expression()
{
    if (current_lex.ident_type == LEX_NUM || current_lex.ident_type == LEX_OPEN_BRACKET) {
        long_expression();
        return LONG;
    } else if (current_lex.ident_type == LEX_QUOTE) {
        text_expression();
        return TEXT;
    } else if (current_lex.ident_type == LEX_ID) {
        if (!object_exist(Analyze::table_access_key, table_head, current_lex.ident_name)) {
            throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                               Analyze::command, current_lex.ident_name);
        }
        ::object_type lex_type = get_object_type(Analyze::table_access_key, table_head, current_lex.ident_name);
        lex_type == LONG ? long_expression() : text_expression();
        return lex_type;
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected <long_expression> | <text_expression>",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::long_expression()
{
    long_term();
    while (current_lex.ident_type == LEX_PLUS ||
           current_lex.ident_type == LEX_MINUS) {
        get_lex();
        long_term();
    }
}

void Analyze::Parser::long_term()
{
    long_multiplier();
    while (current_lex.ident_type == LEX_STAR ||
           current_lex.ident_type == LEX_SLASH ||
           current_lex.ident_type == LEX_PERCENT) {
        get_lex();
        long_multiplier();
    }
}

void Analyze::Parser::long_multiplier()
{
    if (current_lex.ident_type == LEX_OPEN_BRACKET) {
        open_bracket();
        long_expression();
        close_bracket();
    } else {
        long_value();
    }
}

void Analyze::Parser::long_value()
{
    if (current_lex.ident_type == LEX_NUM) {
        get_lex(); // long_integer
    } else {
        // LONG_object_name
        if (current_lex.ident_type != LEX_ID) {
            throw AnalyzeError("SYNTAX ERROR: expected token ID",
                               Analyze::command, current_lex.ident_name);
        }
#if SEMANTIC
        if (!object_exist(Analyze::table_access_key, table_head, current_lex.ident_name)) {
            throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                               Analyze::command, current_lex.ident_name);
        }
        if (get_object_type(Analyze::table_access_key, table_head, current_lex.ident_name) != LONG) {
            throw AnalyzeError("SEMANTIC ERROR: type mismatch, LONG type field expected",
                               Analyze::command, current_lex.ident_name);
        }
#endif
        get_lex();
    }
}

void Analyze::Parser::text_expression()
{
    if (current_lex.ident_type == LEX_QUOTE) {
        string();
    } else {
        // TEXT_object_name
        if (current_lex.ident_type != LEX_ID) {
            throw AnalyzeError("SYNTAX ERROR: expected token ID",
                               Analyze::command, current_lex.ident_name);
        }
#if SEMANTIC
        if (!object_exist(Analyze::table_access_key, table_head, current_lex.ident_name)) {
            throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                               Analyze::command, current_lex.ident_name);
        }
        if (get_object_type(Analyze::table_access_key, table_head, current_lex.ident_name) != TEXT) {
            throw AnalyzeError("SEMANTIC ERROR: type mismatch, TEXT type field expected",
                               Analyze::command, current_lex.ident_name);
        }
#endif
        get_lex();
    }
}

void Analyze::Parser::list_of_constant()
{
    if (current_lex.ident_type == LEX_QUOTE) {
        string();
        while (current_lex.ident_type == LEX_COMMA) {
            get_lex();
            string();
        }
    } else if (current_lex.ident_type == LEX_NUM) {
        get_lex();
        while (current_lex.ident_type == LEX_COMMA) {
            get_lex();
            unsigned_int(); // long_integer
        }
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected token \' | NUMBER",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::logical_expression()
{
    logical_term();
    while (current_lex.ident_type == LEX_OR) {
        get_lex();
        logical_term();
    }
}

void Analyze::Parser::logical_term()
{
    logical_multiplier();
    while (current_lex.ident_type == LEX_AND) {
        get_lex();
        logical_multiplier();
    }
}

void Analyze::Parser::logical_multiplier()
{
    if (current_lex.ident_type == LEX_NOT) {
        get_lex();
        logical_multiplier();
    } else if (current_lex.ident_type == LEX_OPEN_BRACKET) {
        open_bracket();
        logical_expression();
        close_bracket();
    } else {
        relation();
    }
}

void Analyze::Parser::relation()
{
    if (current_lex.ident_type == LEX_NUM || current_lex.ident_type == LEX_OPEN_BRACKET) {
        long_relation();
    } else if (current_lex.ident_type == LEX_QUOTE) {
        text_relation();
    } else if (current_lex.ident_type == LEX_ID) {
        if (!object_exist(Analyze::table_access_key, table_head, current_lex.ident_name)) {
            throw AnalyzeError("SEMANTIC ERROR: this field does not exist in the specified table",
                               Analyze::command, current_lex.ident_name);
        }
        ::object_type lex_type = get_object_type(Analyze::table_access_key, table_head, current_lex.ident_name);
        if (lex_type == LONG) {
            long_relation();
        } else if (lex_type == TEXT) {
            text_relation();
        }
    } else {
        throw AnalyzeError("SYNTAX ERROR: expected <long_expression> | <text_expression>",
                           Analyze::command, current_lex.ident_name);
    }
}

void Analyze::Parser::text_relation()
{
    text_expression();
    comparison_operation();
    is_subquery() ? subquery() : text_expression();
}

void Analyze::Parser::long_relation()
{
    long_expression();
    comparison_operation();
    is_subquery() ? subquery() : long_expression();
}

void Analyze::Parser::comparison_operation()
{
    if (current_lex.ident_type != LEX_EQUAL &&
        current_lex.ident_type != LEX_GREATER &&
        current_lex.ident_type != LEX_LESS &&
        current_lex.ident_type != LEX_GREATER_OR_EQUAL &&
        current_lex.ident_type != LEX_LESS_OR_EQUAL &&
        current_lex.ident_type != LEX_NOT_EQUAL) {
        throw AnalyzeError("SYNTAX ERROR: expected token = | > | < | >= | <= | !=",
                           Analyze::command, current_lex.ident_name);
    }
    get_lex();
}

bool Analyze::Parser::is_subquery()
{
    return current_lex.ident_type == LEX_SELECT ||
           current_lex.ident_type == LEX_INSERT ||
           current_lex.ident_type == LEX_UPDATE ||
           current_lex.ident_type == LEX_DELETE ||
           current_lex.ident_type == LEX_CREATE ||
           current_lex.ident_type == LEX_DELETE ||
           current_lex.ident_type == LEX_DROP;
}

void Analyze::Parser::subquery()
{
    table_head.clear();
    SQL();
}

//pol(){
//    if (LIKE)
//
//}
/* --------------------- class Executor --------------------- */

Analyze::Executor::Executor() = default;

void Analyze::Executor::interpreter()
{
    to_POLIS();
    try {
        /**
         * TODO
         * выполнение запроса
         *  все входные данные уже корректны после этапа анализа,
         *  т.ч. этот try-блок, суорее всего, даже не понадобится
         */
        Analyze::table_is_actual = false;
        Where_condition cur_where = Where_condition();
        Identifier current_command = Analyze::POLIS.back();
        Analyze::POLIS.pop_back();
        if(current_command.ident_type == LEX_WHERE){
            fill_where(cur_where);
            current_command = Analyze::POLIS.back();
            Analyze::POLIS.pop_back();
        }
        switch (current_command.ident_type) {
            case LEX_CREATE: {
                std::string table_name;
                std::vector<std::pair<std::string, std::string>> arguments;
                for (int i = Analyze::POLIS.size() - 1; i > 1; i -= 2) {
                    // заполняю имена и типы столбцов
                    arguments.emplace_back(Analyze::POLIS[i - 1].ident_name, Analyze::POLIS[i].ident_name);
                    Analyze::POLIS.pop_back();
                    Analyze::POLIS.pop_back();
                }
                table_name = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                create_table(Analyze::table_access_key, table_name, arguments);
            }
                break;

            case LEX_SELECT:{
                // пропускаю FROM
                Analyze::POLIS.pop_back();
                std::string table_name = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                std::vector<std::string> column_names;
                while (!Analyze::POLIS.empty()) {
                    column_names.push_back(Analyze::POLIS.back().ident_name);
                    Analyze::POLIS.pop_back();
                }

                select_from_table(Analyze::table_access_key,
                        table_name,
                        column_names,
                        cur_where,
                        Analyze::selected_table);
                table_is_actual = true;
            }
                break;
            case LEX_INSERT:{
                std::vector<std::string> new_record;
                while (!Analyze::POLIS.empty()){
                    // заполняю поля столбцов
                    new_record.push_back(Analyze::POLIS.back().ident_name);
                    Analyze::POLIS.pop_back();
                }
                // заполняю имя таблицы
                std::string table_name = new_record.back();
                new_record.pop_back();
                insert_into_table(Analyze::table_access_key, table_name, new_record);
            }
                break;
            case LEX_UPDATE:{
                // пропускаю (SET, LEX_SET);
                Analyze::POLIS.pop_back();
                // пропускаю (=, LEX_EQUAL);
                Analyze::POLIS.pop_back();

                std::string value = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                std::string col_name = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                std::string table_name = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                update_table(Analyze::table_access_key, table_name, col_name, value, cur_where);
            }
                break;
            case LEX_DELETE:{
                std::string table_name = Analyze::POLIS.back().ident_name;
                Analyze::POLIS.pop_back();
                delete_table(Analyze::table_access_key, table_name, cur_where);
            }
                break;
            case LEX_DROP: {
                std::string table_name = Analyze::POLIS.back().ident_name;
                drop_table(Analyze::table_access_key, table_name);
            }
                break;
        }

    }
    catch (std::exception &err) {
        throw AnalyzeError(std::string("RUN TIME ERROR: ") + err.what(),
                           Analyze::command, ";");
    }
}

void Analyze::Executor::fill_where(Where_condition & where){
    Identifier current_command = Analyze::POLIS.back();
    Analyze::POLIS.pop_back();
    if (current_command.ident_type == LEX_ALL){
        return;
    }
    std::cout<<"я это еще не сделала!\n";
}

void Analyze::Executor::to_POLIS()
{
    std::stack<Identifier> stack_of_operations;

    for (int cur_pos = 0; Analyze::TOKENS[cur_pos].ident_type != LEX_FIN; ++cur_pos) {
        switch (Analyze::TOKENS[cur_pos].ident_type) {
            case LEX_ID:
            case LEX_NUM:
            case LEX_ALL:
            case LEX_TEXT:
            case LEX_LONG:
            case LEX_STRING:
                // операнды
                Analyze::POLIS.push_back(Analyze::TOKENS[cur_pos]);
                break;

            case LEX_OPEN_BRACKET:
                // открывающая скобка
                stack_of_operations.push(Analyze::TOKENS[cur_pos]);
                break;

            case LEX_CLOSE_BRACKET:
                // закрывающая скобка
                while (stack_of_operations.top().ident_type != LEX_OPEN_BRACKET) {
                    Analyze::POLIS.push_back(stack_of_operations.top());
                    stack_of_operations.pop();
                }
                stack_of_operations.pop(); // выкинули открывающую скобку
                break;

            case LEX_QUOTE:
            case LEX_INTO:
            case LEX_TABLE:
            case LEX_COMMA:
                // в ПОЛИЗ не переводим
                break;

            default:
                // операция
                while (!stack_of_operations.empty() &&
                       priority(Analyze::TOKENS[cur_pos].ident_type) <=
                       priority(stack_of_operations.top().ident_type)) {
                    Analyze::POLIS.push_back(stack_of_operations.top());
                    stack_of_operations.pop();
                }
                stack_of_operations.push(Analyze::TOKENS[cur_pos]);

                if (Analyze::TOKENS[cur_pos].ident_type == LEX_DELETE) {
                    cur_pos++; // пропуск FROM после DELETE
                    // отдельно, т.к. FROM является операцией в SELECT-предложении
                }
                break;
        } // end switch
    }

    while (!stack_of_operations.empty()) { // освобождаем стек
        Analyze::POLIS.push_back(stack_of_operations.top());
        stack_of_operations.pop();
    }
#if DEBUG
    Analyze::Scanner::print_TABLE(Analyze::POLIS, "POLIS");
#endif
}

int Analyze::Executor::priority(type_of_lex operation)
{
    switch (operation) {
        case LEX_OPEN_BRACKET:
        case LEX_CLOSE_BRACKET:
            return 0;

        case LEX_WHERE:
            return 1;

        case LEX_SELECT:
        case LEX_INSERT:
        case LEX_UPDATE:
        case LEX_DELETE:
        case LEX_CREATE:
        case LEX_DROP: 
            return 2;

        case LEX_FROM:
        case LEX_SET:
        case LEX_LIKE:
        case LEX_IN:
            return 3;

        case LEX_OR:
            return 4;

        case LEX_AND:
            return 5;

        case LEX_EQUAL:
        case LEX_NOT_EQUAL:
            return 6;

        case LEX_LESS:
        case LEX_GREATER:
        case LEX_LESS_OR_EQUAL:
        case LEX_GREATER_OR_EQUAL:
            return 7;

        case LEX_PLUS:
        case LEX_MINUS:
            return 8;

        case LEX_STAR:
        case LEX_SLASH:
        case LEX_PERCENT:
            return 9;

        case LEX_NOT:
            return 10;

        default:
            return -1;
    } // end switch()
}
