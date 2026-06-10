#include "PythonStreambuf.hpp"

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <streambuf>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>


namespace pymae {
namespace utils {
namespace py = pybind11;


bool PythonStreambuf::refill_buffer() {
    if (has_exception_) return false;

    try {
        py::gil_scoped_acquire acquire;
        
        py::bytes py_data = py_read_(size_);
        std::string_view view(py_data);
        if (view.empty()) {
            return false; // Reach EOF
        }
        std::copy(view.begin(), view.end(), in_buffer_.begin());
            
        setg(in_buffer_.data(), in_buffer_.data(), in_buffer_.data() + view.size());
        return true;
    } 
    catch (py::error_already_set& e) {
        has_exception_ = true;
        exception_msg_ = e.what();
        return false;
    }
}


PythonStreambuf::PythonStreambuf(py::object py_file_obj) : 
    in_buffer_(size_),
    file_obj_(std::move(py_file_obj))
{

    if (py::hasattr(file_obj_, "read")) {
        py_read_ = file_obj_.attr("read");
    }
    if (py::hasattr(file_obj_, "write")) {
        py_write_ = file_obj_.attr("write");
    }

    // Reading init
    setg(in_buffer_.data(), in_buffer_.data(), in_buffer_.data());
}

void PythonStreambuf::check_python_exceptions() {
    if (has_exception_) {
        py::gil_scoped_acquire acquire;
        has_exception_ = false;
        throw std::runtime_error(
            "Python I/O Error:\n" + exception_msg_
        );
    }
}

// Read
PythonStreambuf::int_type PythonStreambuf::underflow() {
    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }
    if (refill_buffer()) {
        return traits_type::to_int_type(*gptr());
    }
            
    return traits_type::eof();
}

PythonStreambuf::int_type PythonStreambuf::uflow() {
    if (gptr() < egptr()) {
        int_type ch = traits_type::to_int_type(*gptr());
        gbump(1);
        return ch;
    }
            
    if (refill_buffer()) {
        int_type ch = traits_type::to_int_type(*gptr());
        gbump(1);
        return ch;
    }
            
    return traits_type::eof();
}

    // Write
std::streamsize PythonStreambuf::xsputn(const char* s, std::streamsize n) {
    if (!py_write_) return 0;
    if (has_exception_) return 0;
    
    try {
        py::gil_scoped_acquire acquire;
        py_write_(py::bytes(s, n));
        return n;
    } 
    catch (py::error_already_set& e) {
        has_exception_ = true;
        exception_msg_ = e.what();
        return 0;
    }
}

int PythonStreambuf::overflow(int c) {
    if (c != traits_type::eof()) {
        char ch = static_cast<char>(c);
        if (xsputn(&ch, 1) == 0) return traits_type::eof();
    }
    return c;
}


}} // pymae::utils