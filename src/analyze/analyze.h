#ifndef SQL_INTERPRETER_ANALYZE_H
#define SQL_INTERPRETER_ANALYZE_H

#include <iostream> // std::ostream
#include <sstream>  // std::istringstream
#include <string>   // std::string
#include <vector>   // std::vector
#include <set>      // std::set
#include <table/table.h>

/* ------------------------------------------------ */
/* ------------------- ANALYZE -------------------- */
/* ------------------------------------------------ */


/* -------------------- class Identifier -------------------- */

enum type_of_lex
{
    LEX_NULL,
    /* служебные слова */
    LEX_SELECT,
    LEX_FROM,
    LEX_INSERT,
    LEX_INTO,
    LEX_UPDATE,
    LEX_SET,
    LEX_DELETE,
    LEX_CREATE,
    LEX_TABLE,
    LEX_TEXT,
    LEX_LONG,
    LEX_DROP,
    LEX_WHERE,
    LEX_NOT,
    LEX_LIKE,
    LEX_IN,
    LEX_AND,
    LEX_OR,
    LEX_ALL,
    /* служебные символы */
    LEX_FIN, 
    LEX_COMMA,
    LEX_STAR,
    LEX_QUOTE,
    LEX_OPEN_BRACKET,
    LEX_CLOSE_BRACKET,
    LEX_PLUS,
    LEX_MINUS,
    LEX_SLASH,
    LEX_PERCENT,
    LEX_EQUAL,
    LEX_GREATER,
    LEX_LESS,
    LEX_GREATER_OR_EQUAL,
    LEX_LESS_OR_EQUAL,
    LEX_NOT_EQUAL,
    /* имя пользователя и числовая константа */
    LEX_NUM,
    LEX_ID,
    LEX_STRING
}; // enum type_of_lex

class Identifier
{
public:
    type_of_lex ident_type;  // тип идентификатора
    std::string ident_name;  // имя идентификатора

    /**
     * [constructor: default]
     */
    Identifier();

    /**
     * [constructor: creates an object of type <Identifier>]
     */
    Identifier(type_of_lex type, const std::string &name);

    /**
     * [overloaded operator==: compares the <ident_name> with <str_name>      ]
     * [!NB Эта функция нужна для std::find() в Analyze::Scanner::put_to_TID()]
     */
    bool operator==(const std::string &str_name) const noexcept;

    /**
     * [overloaded friend operator<<: prints information about <this>]
     */
    friend std::ostream &operator<<(std::ostream &sout, Identifier &ident);
}; // class Identifier



/* --------------------- class Analyze ---------------------- */

class Analyze
{
public:
    /**
     * [constructor: creates an analyzer object for the <query>]
     */
    Analyze(int key, const std::string &query);

    /**
     * [destructor: clears the memory from under static objects]
     */
    ~Analyze();

    /**
     * [start: run command analysis]
     */
    static void start();


    /**
    * [get_table_text: return table in string representation]
    */
    std::string get_table_text();
    static bool table_is_actual; // обновленная или мусорная таблица сейчас находится в selected_table
    
    static int table_access_key;             // ключ доступа к таблицам -- дескриптор клиента
    static std::string command;              // команда для анализа

    static const char * TABLE_OF_LEXEME[];   // таблица лексем по type_of_lex
    static const char * TABLE_OF_KEYWORDS[]; // таблица служебных слов
    static const char * TABLE_OF_DELIMS[];   // таблица служебных символов
    static std::vector<Identifier> TID;      // таблица идентификаторов
    static std::vector<Identifier> TOKENS;   // таблица токенов: запрос, разбитый на лексемы
    static std::vector<Identifier> POLIS;    // таблица внутреннего представления запроса (ПОЛИЗ)
    static Table selected_table;             // таблица, сгенерированная запросом или подзапросом
                                             // (если обращение подразумеват генерацию таблицы)
private:

    /* --------------------- class Scanner --------------------- */

    class Scanner
    {
    public:
        /**
         * [constructor: creates the object of type <Scanner> out of the <Analyze::command>]
         */
        Scanner();

        /**
         * [lexical_analyze: provides lexical analysis, puts Identifiers into <Analyze::TOKENS>]
         */
        void lexical_analyze();

        /**
         * [print_TABLE: prints <Analyze::TID>, <Analyze::POLIZ>, <Analyze::TOKENS>]
         */
        static void print_TABLE(std::vector<Identifier> TABLE, const char *table_name = "TABLE");

    private:
        char c;                 // текущий считываемый из команды символ
        std::istringstream sin; // поток ввода, сформированный из команды Analyze::command

        /**
         * [get_char: reads a symbol from <Scanner::sin>]
         */
        void get_char();

        /**
         * [get_lex: returns the lex with its type, value and name; put Identifiers into <Analyze::TID>]
         */
        const Identifier get_lex();

        /**
         * [look: searches for the <buf> in <table>; returns its position if found and 0 otherwise]
         */
        static int look(const std::string &lex, const char **table) ;

        /**
         * [put_to_TID: puts Users' Identifiers to <Analyze::TID>]
         */
        static int put_to_TID(const std::string &lex);
    }; // class Scanner


    /* ---------------------- class Parer ---------------------- */

    class Parser {
    public:
        /**
         * [constructor: default]
         */
        Parser();

        /**
         * [syntactic_analyze: provides syntactic analysis]
         */
        void syntactic_analyze();

    private:
        /* for syntactic analysis: */
        Identifier current_lex; // текущий анализируемый идентификатор
        int pos = 0;            // позиция <current_lex> в <Analyze::TOKENS>

        /* for semantic analysis: */
        std::string table_head;                // имя таблицы
        std::set<std::string> obj_list;        // список полей (для SELECT)
        std::vector<std::string> actual_param; // вектор типов фактических параметров (для INSERT)


        /**
         * [get_lex: writes the next token fron <Analyze::TOKENS> to the <current_lex>]
         */
        void get_lex();

        /**
         * [syntactic + semantic analysis]
         */
        void SQL();
            void SELECT();
                void object_list();
                    void object_name();
                void FROM();
                void table_name();
            void INSERT();
                void INTO();
                void open_bracket();
                void close_bracket();
                void object_value();
                    void string();
            void UPDATE();
                void SET();
                void EQUAL();
            void DELETE();
            void CREATE();
                void TABLE();
                void new_table_name();
                void list_of_object_expression();
                    void object_description();
                        void new_object_name();
                        void object_type();
                            void unsigned_int();
            void DROP();

        void WHERE_clause();
            void WHERE();
            void LIKE();
            void IN();
            int expression(); // возвращаемый тип должен быть object_type из библиотеки Table
                void long_expression();
                    void long_term();
                        void long_multiplier();
                            void long_value();
                void text_expression();
                void list_of_constant();

            void logical_expression();
                void logical_term();
                    void logical_multiplier();
                        void relation();
                            void text_relation();
                            void long_relation();
                                void comparison_operation();
            bool is_subquery();
            void subquery();
    }; // class Parser


    /* --------------------- class Executor --------------------- */

    class Executor
    {
    public:
        /**
         * [constructor: default]
         */
        Executor();

        /**
         * [interpreter: executes user request by <Analyze::POLIS>]
         */
        void interpreter();

    private:
        /**
         * [to_POLIS: translate the request from <Analyze::TOKENS> to the <Analyze::POLIS>]
         */
        static void to_POLIS();

        /**
         * [fill_where: fill class where_condition]
         */
        void fill_where(Where_condition & where);

        /**
         * [priority: give priority to <operation>]
         */
        static int priority(type_of_lex operation);
    }; // class Executor

}; // class Analyze

#endif // SQL_INTERPRETER_ANALYZE_H
