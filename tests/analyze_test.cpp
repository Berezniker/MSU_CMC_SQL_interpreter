#include "gtest/gtest.h"

#include <analyze/analyze.h>
#include <exeption/exception.h>


/**
 CREATE TABLE t (id LONG, name TEXT);
 CREATE TABLE table( id LONG, age LONG);
 DELETE FROM table WHERE age < 18;
 DELETE FROM t WHERE name NOT LIKE 'a_%_%';
 DELETE FROM t WHERE id > 10 - id*2;
 INSERT INTO t (1, 'Sasha Grey');
 SELECT id FROM t WHERE id = 2;
 SELECT id FROM t WHERE name IN ('Johnny Sins', 'Evan Stone');
 SELECT name FROM t WHERE id IN (SELECT id FROM table  WHERE age > 18);
 SELECT id, name FROM t WHERE name > 'xxx' AND id < 20;
 UPDATE t SET name = 'XXX' WHERE id = 69;
 DROP TABLE t;


 CREATE TABLE t (id LONG, name TEXT);
0  (t, LEX_ID);
1  (id, LEX_ID);
2  (LONG, LEX_LONG);
3  (name, LEX_ID);
4  (TEXT, LEX_TEXT);
5  (CREATE, LEX_CREATE);

 CREATE TABLE table( id LONG, age LONG);
0  (table, LEX_ID);
1  (id, LEX_ID);
2  (LONG, LEX_LONG);
3  (age, LEX_ID);
4  (LONG, LEX_LONG);
5  (CREATE, LEX_CREATE);

 DELETE FROM table WHERE age < 18;
0  (table, LEX_ID);
1  (DELETE, LEX_DELETE);
2  (age, LEX_ID);
3  (18, LEX_NUM);
4  (<, LEX_LESS);
5  (WHERE, LEX_WHERE);

 INSERT INTO t (1, 'Sasha Grey');
0  (t, LEX_ID);
1  (1, LEX_NUM);
2  (Sasha Grey, LEX_STRING);
3  (INSERT, LEX_INSERT);

 SELECT id FROM t WHERE id = 2;
0  (id, LEX_ID);
1  (t, LEX_ID);
2  (FROM, LEX_FROM);
3  (SELECT, LEX_SELECT);
4  (id, LEX_ID);
5  (2, LEX_NUM);
6  (=, LEX_EQUAL);
6  (=, LEX_EQUAL);
7  (WHERE, LEX_WHERE);

 SELECT id FROM t WHERE name IN ('Johnny Sins', 'Evan Stone');
0  (id, LEX_ID);
1  (t, LEX_ID);
2  (FROM, LEX_FROM);
3  (SELECT, LEX_SELECT);
4  (name, LEX_ID);
5  (Johnny Sins, LEX_STRING);
6  (Evan Stone, LEX_STRING);
7  (IN, LEX_IN);
8  (WHERE, LEX_WHERE);

 SELECT name FROM t WHERE id IN (SELECT id FROM table  WHERE age > 18);
0  (name, LEX_ID);
1  (t, LEX_ID);
2  (FROM, LEX_FROM);
3  (SELECT, LEX_SELECT);
4  (id, LEX_ID);
5  (id, LEX_ID);
6  (table, LEX_ID);
7  (FROM, LEX_FROM);
8  (SELECT, LEX_SELECT);
9  (age, LEX_ID);
10 (18, LEX_NUM);
11 (>, LEX_GREATER);
12 (WHERE, LEX_WHERE);
13 (IN, LEX_IN);
14 (WHERE, LEX_WHERE);

 SELECT id, name FROM t WHERE name > 'xxx' AND id < 20;
0  (id, LEX_ID);
1  (name, LEX_ID);
2  (t, LEX_ID);
3  (FROM, LEX_FROM);
4  (SELECT, LEX_SELECT);
5  (name, LEX_ID);
6  (xxx, LEX_STRING);
7  (>, LEX_GREATER);
8  (id, LEX_ID);
9  (20, LEX_NUM);
10 (<, LEX_LESS);
11 (AND, LEX_AND);
12 (WHERE, LEX_WHERE);

 DELETE FROM t WHERE name NOT LIKE 'a_%_%;
0  (t, LEX_ID);
1  (DELETE, LEX_DELETE);
2  (name, LEX_ID);
3  (NOT, LEX_NOT);
4  (a_%_%, LEX_STRING);
5  (LIKE, LEX_LIKE);
6  (WHERE, LEX_WHERE);

 DELETE FROM t WHERE id > 10 - id*2;
0  (t, LEX_ID);
1  (DELETE, LEX_DELETE);
2  (id, LEX_ID);
3  (10, LEX_NUM);
4  (id, LEX_ID);
5  (2, LEX_NUM);
6  (*, LEX_STAR);
7  (-, LEX_MINUS);
8  (>, LEX_GREATER);
9  (WHERE, LEX_WHERE);

 UPDATE t SET name = 'XXX' WHERE id = 69;
0  (t, LEX_ID);
1  (name, LEX_ID);
2  (XXX, LEX_STRING);
3  (=, LEX_EQUAL);
4  (SET, LEX_SET);
5  (UPDATE, LEX_UPDATE);
6  (id, LEX_ID);
7  (69, LEX_NUM);
8  (=, LEX_EQUAL);
9  (WHERE, LEX_WHERE);

  DROP TABLE t;
0  (t, LEX_ID);
1  (DROP, LEX_DROP);
 */

TEST(analyze_test, correct_input)
{
    EXPECT_NO_THROW(
            Analyze(0, "CREATE TABLE t (id LONG, name TEXT);").start();
            Analyze(0, "CREATE TABLE table( id LONG, age LONG);").start();
    );
    EXPECT_NO_THROW(
            Analyze(0, "DELETE FROM table WHERE age < 18;").start();
            Analyze(0, "DELETE FROM t WHERE name NOT LIKE 'a_%_%';").start();
            Analyze(0, "DELETE FROM t WHERE id > 10 - id*2;").start();
    );
    EXPECT_NO_THROW(
            Analyze(0, "INSERT INTO t (69, 'Sasha Grey');").start();
    );
    EXPECT_NO_THROW(
            Analyze(0, "SELECT id FROM t WHERE id = 2;").start();
            Analyze(0, "SELECT id FROM t WHERE name IN ('Johnny Sins', 'Evan Stone');").start();
            Analyze(0, "SELECT name FROM t WHERE id IN (SELECT id FROM table  WHERE age > 18);").start();
            Analyze(0, "SELECT id, name FROM t WHERE name > 'xxx' AND id < 20;").start();
    );
    EXPECT_NO_THROW(
            Analyze(0, "UPDATE t SET name = 'XXX' WHERE id = 69;").start();
    );
    EXPECT_NO_THROW(
            Analyze(0, "DROP TABLE t;").start();
    );
}

TEST(analyze_test, correct_output)
{
    Analyze a1 = Analyze(1, "CREATE TABLE m (id LONG, name TEXT, age LONG);");
    a1.start();
    EXPECT_EQ(a1.get_table_text(),
              "");
    a1.~Analyze();
    Analyze a2 = Analyze(1, "SELECT id, name, age FROM m WHERE ALL;");
    a2.start();
    EXPECT_EQ(a2.get_table_text(),
              "\nSELECTED FROM: m\n--- COLUMN NAME: age\n\n--- COLUMN NAME: id\n\n--- COLUMN NAME: name\n\n");
    a2.~Analyze();
    Analyze a3 = Analyze(1, "INSERT INTO m (10, 'Johnny Depp', 55);");
    a3.start();
    EXPECT_EQ(a3.get_table_text(),
              "");
    a3.~Analyze();
    Analyze a4 = Analyze(1, "SELECT name FROM m WHERE ALL;");
    a4.start();
    EXPECT_EQ(a4.get_table_text(),
              "\nSELECTED FROM: m\n--- COLUMN NAME: name\n0: Johnny Depp\n\n");
    a4.~Analyze();
}