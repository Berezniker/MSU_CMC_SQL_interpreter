#include <string>    // std::string
#include <utility>   // std::move(), std::pair, std::make_pair()
#include <vector>    // std::vector: push_back()
#include <map>       // std::map: find(), end(), emplace(), erase(), insert()
#include <exception> // std::runtime_error(), std::out_of_range

#include "table.h"   // прототипы всех функций, описанных в этом файле


/*----------------------------------------------------------------*/
/*---*/std::map<int, std::map<std::string, Table> > database;/*---*/
/**              ^       ^          ^       ^
 * {client descriptor}   |   {table name}   |
 *              {all user tables}   {specific table}
 *----------------------------------------------------------------*/


/* -------------------- class Column -------------------- */

Table::Column::Column() = default;


Table::Column::Column(object_type type) : type(type)
{}


Table::Column::Column(const std::string &stype)
{
    if (stype == "TEXT") {
        type = TEXT;
    } else if (stype == "LONG") {
        type = LONG;
    } else {
        type = NONE;
    }
}


/* -------------------- class Table -------------------- */

Table::Table() = default;


// columns.first - column name, columns.second - column type
Table::Table(const std::string &table_name, std::vector<std::pair<std::string, std::string>> &columns)
        : table_name(table_name)
{
    for (const auto &column : columns) {
        if (table.find(column.first) != table.end()) { // поле с данным именем уже существует
            throw std::runtime_error("column name \"" + column.first + "\" is redeclare");
        }
        table.emplace(column.first, column.second);
        ordered_column_names.push_back(column.first);
    }
}

void Table::clear()
{
    this->table.clear();
    this->ordered_column_names.clear();
    this->table_name.clear();
}

void
select_from_table(int key, const std::string &table_name,
                  std::vector<std::string> &field_names,
                  Where_condition &where,
                  Table &selected_table)
{
    auto table = database.at(key).at(table_name).table;
    std::vector<std::pair<std::string, std::string>> columns;
    selected_table.clear();
    selected_table.table_name = table_name;
    for (auto &col_name : field_names) {
        Table::Column new_col = Table::Column(table[col_name]);
        std::vector<std::string> new_values;
        for (const std::string &item: new_col.data) {
            if (where.condition(item))
                new_values.push_back(item);
        }
        new_col.data = new_values;
        selected_table.table.emplace(col_name, new_col);
    }
}


void insert_into_table(int key, const std::string &table_name, std::vector<std::string> &new_record)
{
    Table &user_table = database.at(key).at(table_name); // получаем доступ к таблице <table_name> клиента <key>
    int i = 0;
    for (auto &col_name : user_table.ordered_column_names) {
        user_table.table[col_name].data.push_back(
                new_record[i++]); // добавляем новую запись из <new_record> в поля таблицы <table_name>
    }
}


void update_table(int key, const std::string &table_name, std::string &column_name, const std::string &new_value,
                  Where_condition &where)
{
    Table &user_table = database.at(key).at(table_name); // получаем доступ к таблице <table_name> клиента <key>
    Table::Column column = user_table.table[column_name];
    for (int i = 0; i < column.data.size(); ++i) {
        std::string changed_field = column.data[i];
        if (where.condition(changed_field))
            column.data[i] = new_value;   // вносим изменения в указанные поля таблицы
    }
}


void delete_table(int key, const std::string &table_name, Where_condition &where)
{
    Table &user_table = database.at(key).at(table_name); // получаем доступ к таблице <table_name> клиента <key>
    //todo
    for (auto &column:user_table.table) {
        column.second.data.clear();
    }
}


void create_table(int key, const std::string &table_name,
                  std::vector<std::pair<std::string, std::string>> &columns)
{
    auto &user_database = database[key]; // получаем доступ к таблицам пользователя с ключом <key>
    // если <key> в <database> не существует, создастся новый элемент
    if (user_database.find(table_name) !=
        user_database.end()) { // таблица с именем <table_name> уже существует в базе данных клиента
        throw std::runtime_error("table with name \'" + table_name + "\' already exist");
    }
    // создаём таблицу с именем <table_name> и инициализируем поля таблицы в классе Table
    user_database.insert(std::make_pair(table_name, Table(table_name, columns)));
}

void drop_table(int key, const std::string &table_name)
{
    database.at(key).erase(table_name); // удаляем таблицу <table_name> у клиента <key>
}


object_type get_object_type(int key, const std::string &table_name, const std::string &object_name)
{
    try {
        return database.at(key).at(table_name).table.at(object_name).type;
    }
    catch (std::out_of_range &error) {
        return NONE;
    }
}


bool table_exist(int key, const std::string &table_name)
{
    auto user_it = database.find(key); // возвращает pair<key, map<...>>
    if (user_it == database.end()) {   // клиента нет в базе данных
        return false;
    }
    if ((*user_it).second.find(table_name) == (*user_it).second.end()) {
        return false;                  // таблицы <table_name> нет у пользователя
    }

    return true;
}


bool object_exist(int key, const std::string &table_name, const std::string &object_name)
{
    auto user_it = database.find(key); // возвращает pair<key, map<...>>
    if (user_it == database.end()) {   // клиента с <key> нет в базе данных
        return false;
    }
    auto table_it = (*user_it).second.find(table_name); // возвращает pair<table_name, Table>
    if (table_it == (*user_it).second.end()) {          // таблицы <table_name> нет у пользователя
        return false;
    }
    if ((*table_it).second.table.find(object_name) == (*table_it).second.table.end()) {
        return false;                  // поля <object_name> нет в таблице <table_name>
    }

    return true;
}


void check_param(int key, const std::string &table_name, std::vector<std::string> &actual_param)
{
    if (!table_exist(key, table_name)) {
        throw std::runtime_error("table with the given name does not exist");
    }

    auto column_names = database.at(key).at(table_name).ordered_column_names;

    if (actual_param.size() != column_names.size()) {
        throw std::runtime_error("mismatch of the number of parameters");
    }
    int param_size = column_names.size() - 1;
    for (int i = 0; i < column_names.size(); ++i) {
        if ((database.at(key).at(table_name).table[column_names[param_size - i]].type == TEXT && actual_param[i] == "LONG") ||
            (database.at(key).at(table_name).table[column_names[param_size - i]].type == LONG && actual_param[i] == "TEXT")) {
            if (database.at(key).at(table_name).table[column_names[i]].type == TEXT) {
                throw std::runtime_error("type mismatch, TEXT type field expected");
            } else if (database.at(key).at(table_name).table[column_names[param_size - i]].type == LONG) {
                throw std::runtime_error("type mismatch, LONG type field expected");
            } else {
                throw std::runtime_error("type mismatch");
            }

        }
    }
}

std::string Table::to_string()
{
    std::string str = "\nSELECTED FROM: " + table_name + "\n";
    for (auto &col: table) {
        str += "--- COLUMN NAME: " + col.first + "\n";
        int i = 0;
        for (auto &item: col.second.data) {
            str += std::to_string(i++) + ": " + item + "\n";
        }
        str += "\n";
    }
    return std::string(str);
}
