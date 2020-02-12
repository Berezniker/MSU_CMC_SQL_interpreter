#ifndef SQL_INTERPRETER_EXCEPTION_H
#define SQL_INTERPRETER_EXCEPTION_H

#include <string>    // std::string
#include <exception> // derived class std::exception

/* --------------------------------------------------- */
/* -------------------- EXCEPTION -------------------- */
/* --------------------------------------------------- */

/**
 * [isfatal: returns true if the last error is fatal; false - otherwise]
 */
bool isfatal();


class FatalError: public std::exception
{
private:
    std::string error_message; // сообщение о фатальной ошибке

public:
    /**
     * [constructor: generates a RED error_message in the format:]
     * [             !!![<user>] <function>(): <description>     ]
     */
    FatalError(const char *user, const char *function, const char *description);

    /**
     * [destructor: clears the error buffer]
     */
    ~FatalError() noexcept;

    /**
     * [what: returns an error message]
     */
    inline const char * what() const noexcept;
}; // class FatalError


class FlexibleError: public std::exception
{
private:
    std::string error_message; // сообщение об исправимой ошибке

public:
    /**
     * [constructor: generates an error message in the format:]
     * [             !!![<user>] <function>(): <description>  ]
     */
    FlexibleError(const char *user, const char *function, const char *description);

    /**
     * [destructor: clears the error buffer]
     */
    ~FlexibleError() noexcept;

    /**
     * [what: returns an error message]
     */
    inline const char * what() const noexcept;
}; // class FlexibleError


class AnalyzeError: public std::exception
{
private:
	std::string error_message; // сообщение об ошибке при анализе

public:
    /**
     * [constructor: generates an error_message in the format:]
     * [             !!!<error_description>                   ]
     * [             **** **** **** <error_lexeme> **** ****  ]
     * [                            ^~~~~~~~~~~~~             ]
     */
    AnalyzeError(const std::string &error_description,
                 const std::string &error_line,
                 const std::string &error_lexeme);

    /**
     * [destructor: clears the error buffer]
     */
    ~AnalyzeError() noexcept;

    /**
     * [what: returns an error message]
     */
    inline const char * what() const noexcept;
}; // class AnalyzeError

#endif // SQL_INTERPRETER_EXCEPTION_H
