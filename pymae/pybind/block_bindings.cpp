#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <MaeBlock.hpp>
#include <string>
#include <ostream>

#include "utils.hpp"

namespace pymae {
namespace py = pybind11;
using namespace schrodinger::mae;

// Docstrings
constexpr auto block_write_doc = R"doc(pymae.Block.write
Write block to buffer
    
:param file: file-like object, must have `write` method,
e. g. 
```
with open("structure.mae", "w") as fd:
    block.write(fd, 1)
```
        
:param current_indentation: int, level of current indentation
)doc";
constexpr auto iblock_write_doc = R"doc(pymae.IndexedBlock.write
Write indexed block to buffer
    
:param file: file-like object, must have `write` method,
e. g. 
```
with open("structure.mae", "w") as fd:
    block.write(fd, 1)
```
        
:param current_indentation: int, level of current indentation
)doc";

// Block
void bind_block(pybind11::module& m) {
py::class_<Block, std::shared_ptr<Block>>(m, "Block",
    R"doc(A mae Block instance, main data container)doc"
)
    //---| General data
    .def("getName", &Block::getName)
    .def("__str__", &Block::toString)
    .def("toString", &Block::toString)
    .def("__repr__", [](const Block& self) {
        return "MaeBlock <<" + self.getName() + ">>";
    })
    .def("write",
        [](const Block& self, py::object file_obj, unsigned int indent=0) {
            utils::PythonStreambuf buffer(file_obj);
            std::ostream stream(&buffer);
            self.write(stream, indent);
        },
        block_write_doc
    )
    
    //---| Indexed Blocks
    .def("hasIndexedBlockData", &Block::hasIndexedBlockData)
    .def("hasIndexedBlock", &Block::hasIndexedBlock)
    .def("getIndexedBlockNames",
        &Block::getIndexedBlockNames,
        "Get the names of all indexed sub-blocks"
    )
    //setIndexedBlockMap
    .def("getIndexedBlock", &Block::getIndexedBlock)
    
    //---| Blocks
    .def("addBlock", &Block::addBlock)
    .def("hasBlock", &Block::hasBlock,
        "Check whether this block has a sub-block of the provided name"
    )
    .def("getBlockNames", &Block::getBlockNames,
        "Get the names of all non-indexed sub-blocks"
    )
    .def("getBlock", &Block::getBlock)
    
    //---| Properties
    .def("hasRealProperty", &Block::hasRealProperty)
    .def("getRealProperty", &Block::getRealProperty)
    .def("setRealProperty", &Block::setRealProperty)
    
    .def("hasIntProperty", &Block::hasIntProperty)
    .def("getIntProperty", &Block::getIntProperty)
    .def("setIntProperty", &Block::setIntProperty)
    
    .def("hasBoolProperty", &Block::hasBoolProperty)
    .def("getBoolProperty", &Block::getBoolProperty)
    .def("setBoolProperty", &Block::setBoolProperty)
    
    .def("hasStringProperty", &Block::hasStringProperty)
    .def("getStringProperty", &Block::getStringProperty)
    .def("setStringProperty", &Block::setStringProperty)
    
    .def("getPropsAsDict",
        [](const Block& self){
            const auto& s_props = self.getProperties<std::string>();
            const auto& r_props = self.getProperties<double>();
            const auto& i_props = self.getProperties<int>();
            const auto& b_props = self.getProperties<BoolProperty>();
            
            py::dict result;
            for (const auto& [key, value] : s_props) {
                result[key.c_str()] = value.c_str();
            }
            for (const auto& [key, value] : r_props) {
                result[key.c_str()] = value;
            }
            for (const auto& [key, value] : i_props) {
                result[key.c_str()] = value;
            }
            for (const auto& [key, value] : b_props) {
                result[key.c_str()] = static_cast<bool>(value);
            }
            return result;
        },
        "Return a dict with properties of all types"
    )

    //---| Python API
    .def("__eq__",
        [](const Block& self, const Block& rhs) {return self == rhs;}
    )
    /* TODO
    .def("__getitem__", &MyDict::get)
    .def("__setitem__", &MyDict::set)
    .def("__delitem__", &MyDict::remove)
    .def("__len__", &MyDict::size)
    .def("__contains__", &MyDict::hasKey)
    .def("__iter__", [](MyDict &self) {
        return py::make_iterator(self.keys().begin(), self.keys().end());
    }, py::keep_alive<0, 1>())
    .def("keys", &MyDict::keys)
    .def("values", &MyDict::values)
    .def("items", &MyDict::items)
    */
;}

template <typename T>
py::list nullable_values_(
        const std::shared_ptr<IndexedProperty<T>>& prop,
        std::function<py::object(const T&)> py_converter
    ) {
    //auto& data = prop.data();
    //auto& null_bs = prop.nullIndices();
    py::list result;
    for (size_t i = 0; i < prop->size(); ++i) {
        if (prop->isDefined(i)) {
            result.append(py_converter(prop->at(i)));
        } else {
            result.append(py::none());
        }
    }
    return result;
}

void bind_indexed_block(py::module& m) {
py::class_<IndexedBlock, std::shared_ptr<IndexedBlock>>(m, "IndexedBlock",
    R"doc(A mae IndexedBlock instance to store serial data.)doc"
    )
    // Basics
    .def("size", &IndexedBlock::size)
    .def("getName", &IndexedBlock::getName)
    .def("__str__", &IndexedBlock::toString)
    .def("toString", &IndexedBlock::toString)
    .def("write",
        [](const IndexedBlock& self, py::object file_obj, unsigned int indent=0) {
            utils::PythonStreambuf buffer(file_obj);
            std::ostream stream(&buffer);
            self.write(stream, indent);
        },
        iblock_write_doc
    )
    .def("__eq__",
        [](const IndexedBlock& self, const IndexedBlock& rhs) {return self == rhs;}
    )
    
    // Properties
    .def("hasBoolProperty", &IndexedBlock::hasBoolProperty)
    .def("getBoolProperty", &IndexedBlock::getBoolProperty)
    .def("setBoolProperty", &IndexedBlock::setBoolProperty)

    .def("hasIntProperty", &IndexedBlock::hasIntProperty)
    .def("getIntProperty", &IndexedBlock::getIntProperty)
    .def("setIntProperty", &IndexedBlock::setIntProperty)

    .def("hasRealProperty", &IndexedBlock::hasRealProperty)
    .def("getRealProperty", &IndexedBlock::getRealProperty)
    .def("setRealProperty", &IndexedBlock::setRealProperty)

    .def("hasStringProperty", &IndexedBlock::hasStringProperty)
    .def("getStringProperty", &IndexedBlock::getStringProperty)
    .def("setStringProperty", &IndexedBlock::setStringProperty)
    
    .def("getPropsAsDict",
        [](const IndexedBlock& self){
            const auto& s_props = self.getProperties<std::string>();
            const auto& r_props = self.getProperties<double>();
            const auto& i_props = self.getProperties<int>();
            const auto& b_props = self.getProperties<BoolProperty>();
            
            py::dict result;
            for (const auto& [key, values] : s_props) {
                result[key.c_str()] = nullable_values_<std::string>(
                    values, [](const std::string& val) {return py::cast(val.c_str());}
                );
            }
            for (const auto& [key, values] : r_props) {
                result[key.c_str()] = nullable_values_<double>(
                    values, [](const double val){return py::cast(val);}
                );
            }
            for (const auto& [key, values] : i_props) {
                result[key.c_str()] = nullable_values_<int>(
                    values, [](const int val){return py::cast(val);}
                );
            }
            for (const auto& [key, values] : b_props) {
                result[key.c_str()] = nullable_values_<BoolProperty>(
                    values,
                    [](const BoolProperty& val){return py::cast(static_cast<bool>(val));}
                );
            }
            return result;
        },
        "Return a dict with properties of all types"
    )
;}
} // pymae
/*


    template <typename T>
    void setProperty(const std::string& name,
                     std::shared_ptr<IndexedProperty<T>> value);


    template <typename T>
    const std::map<std::string, std::shared_ptr<IndexedProperty<T>>>&
    getProperties() const;
};
 */