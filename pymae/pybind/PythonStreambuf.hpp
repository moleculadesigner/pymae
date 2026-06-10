#pragma once

#include <stdexcept>
#include <vector>
#include <streambuf>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>


namespace pymae {
namespace utils {
namespace py = pybind11;

using bytes_buffer = std::vector<char>;

class PythonStreambuf : public std::streambuf {
private:
    py::object file_obj_;
    py::object py_read_;
    py::object py_write_;
    
    bytes_buffer in_buffer_;
    static constexpr size_t size_ = 4096;

    bool has_exception_ = false;
    std::string exception_msg_ = "";

    bool refill_buffer();

public:
    PythonStreambuf(py::object py_file_obj);
    void check_python_exceptions();

protected:
    // Read
    int_type underflow() override;
    int_type uflow() override;

    // Write
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int_type overflow(int c) override;
};

}} // pymae::utils