#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Важно! Конвертирует векторы в списки Python

#include <streambuf>

namespace pymae {
namespace utils {
namespace py = pybind11;

class PythonStreambuf : public std::streambuf {
public:
    PythonStreambuf(py::object file_obj) : file_obj_(std::move(file_obj)) {}
    
protected:
    int_type overflow(int_type c) override {
        if (c != traits_type::eof()) {
            file_obj_.attr("write")(std::string(1, static_cast<char>(c)));
        }
        return c;
    }
    
    std::streamsize xsputn(const char *s, std::streamsize n) override {
        file_obj_.attr("write")(std::string(s, n));
        return n;
    }
    
private:
    py::object file_obj_;
};

}} // pymae::utils