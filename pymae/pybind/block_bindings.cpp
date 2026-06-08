#include <cmath>
#include <map>
#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <MaeBlock.hpp>
#include <pybind11/typing.h>
#include <string>
#include <ostream>
#include <utility>
#include <vector>
#include <boost/dynamic_bitset.hpp>

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

enum class PropertyType {
    R, I, B, S
};

bool is_valid_python_identifier_(const std::string& name) {
    if (name.empty()) return false;
    
    if (!std::isalpha(name[0]) && name[0] != '_') return false;
    
    for (size_t i = 1; i < name.size(); ++i) {
        if (!std::isalnum(name[i]) && name[i] != '_') return false;
    }
    return true;
}

std::vector<std::pair<std::string, PropertyType>> get_prop_names_(const Block& self) {
    const auto& s_props = self.getProperties<std::string>();
    const auto& r_props = self.getProperties<double>();
    const auto& i_props = self.getProperties<int>();
    const auto& b_props = self.getProperties<BoolProperty>();
    
    auto prop_names = std::vector<std::pair<std::string, PropertyType>>{};
    
    for (const auto& [key, value] : s_props) {
        prop_names.push_back(std::pair(key, PropertyType::S));
    }
    for (const auto& [key, value] : r_props) {
        prop_names.push_back(std::pair(key, PropertyType::R));
    }
    for (const auto& [key, value] : i_props) {
        prop_names.push_back(std::pair(key, PropertyType::I));
    }
    for (const auto& [key, value] : b_props) {
        prop_names.push_back(std::pair(key, PropertyType::B));
    }
    return prop_names;
}

py::object get_some_property_(const Block& self, const std::string& prop_name) {
    if (self.hasBoolProperty(prop_name))   return py::cast(self.getBoolProperty(prop_name));
    if (self.hasIntProperty(prop_name))    return py::cast(self.getIntProperty(prop_name));
    if (self.hasRealProperty(prop_name))   return py::cast(self.getRealProperty(prop_name));
    if (self.hasStringProperty(prop_name)) return py::cast(self.getStringProperty(prop_name));
    
    throw py::attribute_error("Block object has no property '" + prop_name + "'");
}

void set_some_property_(Block& self, const std::string& prop_name, const py::object& value) {
    // bool -> int -> float -> str
    if (py::isinstance<py::bool_>(value)) {
        self.setBoolProperty(prop_name, value.cast<bool>());
    } 
    else if (py::isinstance<py::int_>(value)) {
        self.setIntProperty(prop_name, value.cast<int>());
    } 
    else if (py::isinstance<py::float_>(value)) {
        self.setRealProperty(prop_name, value.cast<double>());
    } 
    else if (py::isinstance<py::str>(value)) {
        self.setStringProperty(prop_name, value.cast<std::string>());
    } 
    else {
        throw py::type_error("Unsupported property type for attribute '" + prop_name + "'");
    }
}

// Block
void bind_block(pybind11::module& m) {
py::class_<Block, std::shared_ptr<Block>>(m, "Block",
    R"doc(A mae Block instance, main data container)doc"
)
    //---| General data
    .def(py::init<std::string>())
    .def("getName", &Block::getName)
    .def("__str__", &Block::toString)
    .def("toString", &Block::toString)
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
    
    .def_property_readonly("properties",
        [](const Block& self){
            auto prop_names = get_prop_names_(self);
            py::dict result;
            
            for (const auto& [prop_name, prop_type] : prop_names) {
                switch (prop_type) {
                    case PropertyType::S:
                        result[py::str(prop_name)] = py::str(self.getStringProperty(prop_name));
                        break;
                    case PropertyType::R:
                        result[py::str(prop_name)] = py::float_(self.getRealProperty(prop_name));
                        break;
                    case PropertyType::I:
                        result[py::str(prop_name)] = py::int_(self.getIntProperty(prop_name));
                        break;
                    case PropertyType::B:
                        result[py::str(prop_name)] = py::bool_(static_cast<bool>(self.getBoolProperty(prop_name)));
                        break;
                }
            }
            return result;
        },
        "Return a dict with properties of all types"
    )

    //---| Python API
    .def("__eq__",
        [](const Block& self, const Block& rhs) {return self == rhs;}
    )
    .def("__getattr__",
        [](const Block& self, const std::string& name) {
            if (!is_valid_python_identifier_(name)) {
                throw py::attribute_error("Block object has no attribute '" + name + "'");
            }
            return get_some_property_(self, name);
        }
    )
    .def("__setattr__",
        [](Block& self, const std::string& name, const py::object& value) {
            if (!is_valid_python_identifier_(name)) {
                throw py::attribute_error("Cannot set invalid Python identifier '" + name + "' as attribute");
            }
            set_some_property_(self, name, value);
        }
    )
    .def("__dir__", [](py::object& self) {
        py::list result;
        
        // defaults
        py::object type_obj = self.attr("__class__");
            for (auto handle : type_obj.attr("__dict__")) {
                result.append(handle.cast<std::string>()); 
            }
        
        // related
        Block& core_block = self.cast<Block&>();
        for (const auto& [p, t] : get_prop_names_(core_block)) {
            if (is_valid_python_identifier_(p)) {
                result.append(p);
            }
        }
    
        return result;
    })

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


} // pymae
