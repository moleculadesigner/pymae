// writer_bindings.cpp
// 
#include <string>
#include <filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <MaeBlock.hpp>
#include <Writer.hpp>
#include "PythonStreambuf.hpp"

namespace pymae {
namespace py = pybind11;
using namespace schrodinger::mae;

constexpr auto writer_class_doc = R"doc(Maestro file writer
    
Writes blocks to a Maestro file (.mae or .mae.gz).
Automatically writes the m2io version header.
    
Example::

    with pymae.Writer("output.mae") as writer:
        for block in reader:
            writer.write(block)

    # Also supports pathlib.Path
    with pymae.Writer(Path("output.mae")) as writer:
        for block in reader:
            writer.write(block)

    # Automatically compress if file has .mae.gz extension
    with pymae.Writer("output.mae.gz") as writer:
        for block in reader:
            writer.write(block)
)doc";

class PyMaeWriter {
// Passing Writer itself causes improper destruction in case of gzip pipeline
// so make a wrapper with ownership of all needed resources to keep a raw pointer
// valid in this part of logic in Writer.cpp
// #ifdef MAEPARSER_HAVE_BOOST_IOSTREAMS
//     auto* gzip_stream = new filtering_ostream();
//     gzip_stream->push(boost::iostreams::gzip_compressor());
//     gzip_stream->push(file_sink(fname, ios_mode));
//     m_out.reset(static_cast<ostream*>(gzip_stream));

private:
    std::unique_ptr<utils::PythonStreambuf> m_buf = nullptr;
    std::unique_ptr<schrodinger::mae::Writer> m_writer = nullptr;

public:
    PyMaeWriter(const std::filesystem::path& filename) {
        m_writer = std::make_unique<Writer>(filename.string());
    }
    
    PyMaeWriter(py::object py_file_obj) {
        m_buf = std::make_unique<utils::PythonStreambuf>(py_file_obj);
            
        // creating ostream on the heap to ensure its liftime
        // is not less than Writer's
        auto shared_stream = std::make_shared<std::ostream>(m_buf.get());
        m_writer = std::make_unique<Writer>(shared_stream);
    }
    ~PyMaeWriter() {
        close();
    }

    void close() {
        if (!m_writer) return;
        
        // destroy Writer to flush buffer
        m_writer.reset(); 
        
        if (m_buf) {
            m_buf->check_python_exceptions();
            m_buf.reset();
        }
    }

    void write(const std::shared_ptr<schrodinger::mae::Block>& block) {
        if (!m_writer) {
            throw py::value_error("I/O operation on closed file.");
        }
        m_writer->write(block);
    
        if (m_buf) {
            m_buf->check_python_exceptions();
        }
    }

};

void bind_writer(py::module& m) {
py::class_<PyMaeWriter>(m, "Writer", writer_class_doc)
    .def(
        py::init<const std::filesystem::path&>(),
        py::arg("filename"),
        "Create writer from filename (str or pathlib.Path). Supports *.mae, *.mae.gz and *.maegz"
    )
    .def(
        py::init<py::object>(),
        py::arg("stream"),
        "Create writer from file-like object (e.g. io.BytesIO)."
    )
    .def(
        "__enter__",
        [](py::object &self) { return self; },
        "Enter context manager"
    )
    .def(
        "__exit__",
        [](PyMaeWriter& self, py::object t, py::object b, py::object tb) {
            self.close();
        },
        "Exit context manager"
    )
    .def(
        "close",
        &PyMaeWriter::close,
        "Flush data and close Writer buffer"
    )
    .def(
        "write",
        &PyMaeWriter::write,
        py::arg("block"),
        "Write a block to the file or stream"
    );
} // bind_writer
} // namespace pymae