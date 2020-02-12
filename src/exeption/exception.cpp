#include <string>      // std::string: push_back(), append(), find(), replace(), c_str(), length(), clear(), npos
#include "exception.h" // прототипы всех функций, описанных в этом файле

namespace Color
{
    const char *RESET = "\033[0m";    // сброс стилевых настроек печати
    const char *RED   = "\033[1;31m"; // код красного цвета с повышенной яркостью
}

bool fatal = false; // флаг = {true, если последняя ошибка фатальна; false, иначе}

bool isfatal()
{
    return ::fatal;
}


/* -------------------- class FatalError -------------------- */

FatalError::FatalError(const char *user, const char *function, const char *description)
{
    ::fatal = true; // устанавливаем флаг
    error_message.append(Color::RED)  \
                 .append("!!![")      \
                 .append(user)        \
                 .append("] ")        \
                 .append(function)    \
                 .append("(): ")      \
                 .append(description) \
                 .append(Color::RESET)\
                 ;
}


FatalError::~FatalError() noexcept
{
    error_message.clear(); // отчищаем буфер сообщения об ошибке
}


inline const char * FatalError::what() const noexcept
{
    return error_message.c_str(); 
}



/* ------------------- class FlixibleError ------------------- */

FlexibleError::FlexibleError(const char *user, const char *function, const char *description)
{
    ::fatal = false; // устанавливаем флаг
    error_message.append("!!![")     \
                 .append(user)       \
                 .append("] ")       \
                 .append(function)   \
                 .append("(): ")     \
                 .append(description)\
                 ;
}


FlexibleError::~FlexibleError() noexcept
{
    error_message.clear(); // отчищаем буфер сообщения об ошибке
}


inline const char * FlexibleError::what() const noexcept
{
    return error_message.c_str();
}



/* ------------------- class AnalyzeError ------------------- */

AnalyzeError::AnalyzeError(const std::string &error_description,
                           const std::string &error_line,
                           const std::string &error_lexeme)
{
    // ищем позицию <error_lexeme> в <error_line>
    int shift = error_line.find(error_lexeme), count = 0;
    
    // записываем описание ошибки
    error_message = "!!!" + error_description + "\n";

    if (shift != std::string::npos) {
        // добавляем ошибочную конструкцию 
        error_message.append(error_line);
        // выделяем ошибочную лексему красным цветом
        error_message.replace(error_description.length() + 4 + shift, error_lexeme.length(),
                              std::string(Color::RED + error_lexeme + Color::RESET));
        error_message.push_back('\n');
        for (int i = 0; i < shift; ++i) {
            error_message.push_back(' ');
        }
        // подчеркиваем ошибочную лексему
        error_message.append(Color::RED);
        error_message.push_back('^');
        for (int i = 1; i < error_lexeme.length(); ++i) {
            error_message.push_back('~');
        }
        error_message.append(Color::RESET);
    }
}


AnalyzeError::~AnalyzeError() noexcept
{
    error_message.clear(); // отчищаем буфер сообщения об ошибке
}


inline const char * AnalyzeError::what() const noexcept
{
    return error_message.c_str();
}
