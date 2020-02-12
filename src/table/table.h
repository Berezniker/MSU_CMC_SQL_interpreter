#ifndef SQL_INTERPRETER_TABLE_H
#define SQL_INTERPRETER_TABLE_H

#include <string>   // std::string
#include <utility>  // std::pair
#include <vector>   // std::vector
#include <map>      // std::map
#include <table/Where_condition.h>

/* ------------------------------------------------ */
/* -------------------- TABLE --------------------- */
/* ------------------------------------------------ */

enum object_type
{
    NONE,
    TEXT,
    LONG
};

class Table
{
private:
    class Column
    {
    public:
        object_type type;              // тип поля
        std::vector<std::string> data; // содержимое поля

        /**
         * [constructor: default]
         */
        Column();

        /**
         * [constructor: initialize field type]
         */
        Column(object_type type);

        /**
         * [constructor: initialize field type]
         */
        Column(const std::string &stype);
    }; // class Column

    std::string table_name;              // имя таблицы
    std::map<std::string, Column> table; // таблица <имя поля, содержимое>
    std::vector<std::string> ordered_column_names; // здесь хранятся имена полей по порядку

public:
    /**
     * [constructor: default]
     */
    Table();

    /**
     * [constructor: create <table>, initialize <table_name> and field column names]
     * [             columns[0] - column name, columns[1] - column type            ]
     */
    Table(const std::string &table_name, std::vector<std::pair<std::string, std::string>> &columns);

    /**
     * [to_string: represent table as string]
     */
    std::string to_string();

    /**
     * [clear: clear the table]
     */
    void clear();
    /**
     * комментарий: все функции объявлены как друзья класса Table
     *              чтобы иметь доступ к данным в private области
     *              но не дублировать код функций для каждого объекта
     */


    /*-----------------------------------*/
    /* functions for working with tables */
    /*-----------------------------------*/

    /**
     * [NB!]
     * 1. В данные функции поступают уже корректные данные после всех этапов анализа
     *    т.е. пользователи, таблицы и поля таблиц уже зарегистрированы в базе данных <database>
     *    => исключительных ситуаций возникать не должно
     *
     * 2. <key> == client descriptor
     */

    friend void
    select_from_table(int key, const std::string &table_name, std::vector<std::string> &field_names,
                      Where_condition &where,
                      Table &selected_table);

    friend void
    insert_into_table(int key, const std::string &table_name, std::vector<std::string> &new_record);

    friend void
    update_table(int key, const std::string &table_name, std::string &column_name, const std::string &new_value,
                 Where_condition &where);

    friend void
    delete_table(int key, const std::string &table_name, Where_condition &where);

    friend void
    create_table(int key, const std::string &table_name, std::vector<std::pair<std::string, std::string>> &columns);

    friend void
    drop_table(int key, const std::string &table_name);


    /*---------------------------------*/
    /* functions for semantic analysis */
    /*---------------------------------*/

    /**
     * [!NB] данные функции не должны выкидывать исключения
     */

    friend object_type
    get_object_type(int key, const std::string &table_name, const std::string &object_name);

    friend bool
    table_exist(int key, const std::string &table_name);

    friend bool
    object_exist(int key, const std::string &table_name, const std::string &object_name);

    friend void
    check_param(int key, const std::string &table_name, std::vector<std::string> &actual_param);
}; // class Table


/**
 * [select_from_table: TODO добавить описание]
 */
void
select_from_table(int key, const std::string &table_name, std::vector<std::string> &field_names, Where_condition &where,
                  Table &selected_table);


/**
 * [insert_into_table: insert a new entry <new_record> into the table <table_name>]
 */
void insert_into_table(int key, const std::string &table_name, std::vector<std::string> &new_record);


/**
 * [update_table: assign a <new_value> to the fields <field_names> of table <table_name>]
 */
void update_table(int key, const std::string &table_name, std::string &column_name, const std::string &new_value,
                  Where_condition &where);


/**
 * [delete_table: remove from table <table_name> fields <field_names>]
 */
void delete_table(int key, const std::string &table_name, Where_condition &where);


/**
 * [create_table: create object Table]
 */
void create_table(int key, const std::string &table_name, std::vector<std::pair<std::string, std::string>> &columns);


/**
 * [drop_table: delete the table <table_name> completely]
 */
void drop_table(int key, const std::string &table_name);


/**
* [get_object_type: return the object_type by the <object_name> in table <table_name>]
*/
object_type get_object_type(int key, const std::string &table_name, const std::string &object_name);


/**
 * [table_exist: return true, if a table <table_name> already exists; false otherwise]
 */
bool table_exist(int key, const std::string &table_name);


/**
 * [object_exist: return true, if a field <object_name> already exists in table <table_name>]
 */
bool object_exist(int key, const std::string &table_name, const std::string &object_name);


/**
 * [check_param: check the conformity of the number and types of formal and actual felds]
 * [             of table <table_name>                                                  ]
 */
void check_param(int key, const std::string &table_name, std::vector<std::string> &actual_param);

#endif // SQL_INTERPRETER_TABLE_H
