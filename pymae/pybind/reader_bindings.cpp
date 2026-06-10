#include <filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h> 
#include <pybind11/pytypes.h>
#include <Reader.hpp>
#include <MaeConstants.hpp>
#include <MaeBlock.hpp>
#include "PythonStreambuf.hpp"

namespace pymae {
namespace py = pybind11;
using namespace schrodinger::mae;

constexpr auto reader_class_doc = R"doc(pymae.Reader

A streaming parser for Maestro (.mae) and compressed Maestro (.mae.gz) files.

This class implements the Python iterator protocol. Note that the reader
is forward-only; to re-iterate over the file, a new Reader instance must be created.
)doc";

class PyMaeReader {
private:
    std::unique_ptr<utils::PythonStreambuf> m_buf = nullptr;
    std::unique_ptr<Reader> m_reader = nullptr;

public:
    PyMaeReader(const std::filesystem::path& filename) {
        m_reader = std::make_unique<Reader>(filename.string());
    }

    PyMaeReader(py::object py_file_obj) {
        if (!py::hasattr(py_file_obj, "read")) {
            std::string type_name = py::str(
                py_file_obj.attr("__class__").attr("__name__")
            );
            throw py::type_error(
                "Argument must be a string, pathlib.Path or a file-like object, not \"" + type_name + "\"");
        }

        m_buf = std::make_unique<utils::PythonStreambuf>(py_file_obj);
        
        auto shared_stream = std::make_shared<std::istream>(m_buf.get());
        m_reader = std::make_unique<Reader>(shared_stream);
    }
    ~PyMaeReader() {
        close();
    }

    std::shared_ptr<Block> next(const std::string& block_name) {
        if (!m_reader) {
            throw py::value_error("I/O operation on closed file.");
        }
        
        auto block = m_reader->next(block_name);
        if (m_buf) {
            m_buf->check_python_exceptions();
        }
        return block;
    }

    void close() {
        m_reader.reset();
        if (m_buf) {
            m_buf->check_python_exceptions();
            m_buf.reset();
        }
    }
};

void bind_reader(py::module& m) {
py::class_<PyMaeReader>(m, "Reader", reader_class_doc)
    .def(py::init<const std::filesystem::path&>(), py::arg("filename"))
    .def(py::init<py::object>(), py::arg("file_object"))
    .def(
        "next",
        [](PyMaeReader& self) {return self.next(CT_BLOCK);},
        "Return next Structure block"
    )
    .def("close", &PyMaeReader::close)
    // make it an iterator
    .def("__iter__", [](py::object self) { return self; })
    .def("__next__", [](PyMaeReader& self) {
        auto next = self.next(CT_BLOCK);
        if (!next) {
            throw py::stop_iteration();
        }
        return next;
    })
    .def(
        "__enter__",
        [](py::object self) { return self; },
        "Enter context manager"
    )
    .def(
        "__exit__",
        [](PyMaeReader& self, py::object t, py::object b, py::object tb) {
            self.close();
        },
        "Exit context manager"
    );
}
} // pymae