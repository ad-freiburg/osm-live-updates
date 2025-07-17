//
// Created by Nicolas von Trott on 17.07.25.
//

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>
#include <string>

namespace olu::util {

    /**
     * Exception that is thrown when the database is up to date.
     */
    class DatabaseUpToDateException final : public std::exception {
        std::string message;
    public:
        explicit DatabaseUpToDateException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //EXCEPTIONS_H
