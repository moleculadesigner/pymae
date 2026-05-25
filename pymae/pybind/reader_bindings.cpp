#include <pybind11/pybind11.h>
#include <Reader.hpp>
#include <MaeConstants.hpp>
#include <MaeBlock.hpp>

namespace pymae {
namespace py = pybind11;
using namespace schrodinger::mae;

constexpr auto reader_class_doc = R"doc(pymae.Reader

A streaming parser for Maestro (.mae) and compressed Maestro (.mae.gz) files.

This class implements the Python iterator protocol. Note that the reader
is forward-only; to re-iterate over the file, a new Reader instance must be created.
)doc";

void bind_reader(py::module& m) {
py::class_<Reader, std::shared_ptr<Reader>>(m, "Reader", reader_class_doc)
    .def(py::init<std::string>())
    .def(
        "nextStructure",
        [](std::shared_ptr<Reader> &r) {return r->next(CT_BLOCK);},
        "Return next Structure block"
    )
    // make it an iterator
    .def("__iter__", [](std::shared_ptr<Reader> &r) { return r; })
    .def("__next__", [](std::shared_ptr<Reader> &r) {
        auto next = r->next(CT_BLOCK);
        if (!next) {
            throw py::stop_iteration();
        }
        return next;
    });
}
} // pymae