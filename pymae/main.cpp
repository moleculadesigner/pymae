
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Важно! Конвертирует векторы в списки Python
#include <Reader.hpp>
#include <MaeConstants.hpp>
//#include <Block.hpp>

namespace py = pybind11;
using namespace schrodinger::mae;

PYBIND11_MODULE(pymae, m) {
    // Пробрасываем класс Reader
    py::class_<Reader, std::shared_ptr<Reader>>(m, "Reader", R"doc(
        A streaming parser for Maestro (.mae) and compressed Maestro (.mae.gz) files.
        
        This class implements the Python iterator protocol. Note that the reader
        is forward-only; to re-iterate over the file, a new Reader instance must be created.
    )doc")
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
                throw py::stop_iteration(); // Остановка цикла в Python
            }
            return next;
        });
    /*
    // Пробрасываем класс Block
    py::class_<Block, std::shared_ptr<Block>>(m, "Block")
        .def("getName", &Block::getName)
        .def("getDataBlock", &Block::getDataBlock, "Доступ к вложенным данным")
        .def("getRealProperty", &Block::getRealProperty)
        .def("getIntProperty", &Block::getIntProperty)
        .def("getStringProperty", &Block::getStringProperty);
    */
}
    
