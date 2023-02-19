#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <stdexcept>

namespace db {
    class error_file : public std::runtime_error {
    public:
        explicit error_file(const std::string &error) : std::runtime_error(error) {};

    };

    class error_command : public std::runtime_error { ;
    public:
        explicit error_command(const std::string &error) : std::runtime_error(error) {}
    };

    class error_type : public std::runtime_error { ;
    public:
        explicit error_type(const std::string &error) : std::runtime_error(error) {}
    };

    class error_table : public std::runtime_error { ;
    public:
        explicit error_table(const std::string &error) : std::runtime_error(error) {}
    };

    class error_database : public std::runtime_error { ;
    public:
        explicit error_database(const std::string &error) : std::runtime_error(error) {}
    };
}

#endif 
