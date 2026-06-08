#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <MaeBlock.hpp>
#include <string>
#include <ostream>
#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "utils.hpp"

namespace pymae {
namespace py = pybind11;
using namespace schrodinger::mae;


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


template <typename T>
py::list extract_nullable_values_(
        const std::shared_ptr<IndexedProperty<T>>& prop,
        std::function<py::object(const T&)> py_converter
    ) {
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

template <typename T>
std::shared_ptr<IndexedProperty<T>> prop_from_list_(const py::list& list) {
    std::vector<T> data = std::vector<T>();
    boost::dynamic_bitset<> *is_null = nullptr;
    for (auto& val : list) {
        try {
            T converted = val.cast<T>();
            data.push_back(converted);
        } catch (const py::cast_error& e) {
            if (!is_null) {
                is_null = new boost::dynamic_bitset<>(list.size());
            }
            is_null->set(data.size() - 1);
            data.push_back(T{});  // Fill w/ defaults
        }
    }
    
    return std::make_shared<IndexedProperty<T>>(data, is_null);
}

void bind_indexed_block(py::module& m) {
py::class_<IndexedBlock, std::shared_ptr<IndexedBlock>>(m, "IndexedBlock",
    R"doc(A mae IndexedBlock instance to store serial data.)doc"
    )
    // Basics
    .def(py::init<std::string>())
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
    .def("setBoolProperty",
        [](IndexedBlock& self, const std::string& prop_name, const py::list& values) {
            auto prop = prop_from_list_<BoolProperty>(values);
            self.setBoolProperty(prop_name, prop);
        }
    )
    .def("hasIntProperty", &IndexedBlock::hasIntProperty)
    .def("getIntProperty", &IndexedBlock::getIntProperty)
    .def("setIntProperty",
        [](IndexedBlock& self, const std::string& prop_name, const py::list& values) {
            auto prop = prop_from_list_<int>(values);
            self.setIntProperty(prop_name, prop);
        }
    )

    .def("hasRealProperty", &IndexedBlock::hasRealProperty)
    .def("getRealProperty", &IndexedBlock::getRealProperty)
    .def("setRealProperty", 
        [](IndexedBlock& self, const std::string& prop_name, const py::list& values) {
            auto prop = prop_from_list_<double>(values);
            self.setRealProperty(prop_name, prop);
        }
    )

    .def("hasStringProperty", &IndexedBlock::hasStringProperty)
    .def("getStringProperty", &IndexedBlock::getStringProperty)
    .def("setStringProperty", 
        [](IndexedBlock& self, const std::string& prop_name, const py::list& values) {
            auto prop = prop_from_list_<std::string>(values);
            self.setStringProperty(prop_name, prop);
        }
    )
    
    .def("getPropsAsDict",
        [](const IndexedBlock& self){
            const auto& s_props = self.getProperties<std::string>();
            const auto& r_props = self.getProperties<double>();
            const auto& i_props = self.getProperties<int>();
            const auto& b_props = self.getProperties<BoolProperty>();
            
            py::dict result;
            for (const auto& [key, values] : s_props) {
                result[key.c_str()] = extract_nullable_values_<std::string>(
                    values, [](const std::string& val) {return py::cast(val.c_str());}
                );
            }
            for (const auto& [key, values] : r_props) {
                result[key.c_str()] = extract_nullable_values_<double>(
                    values, [](const double val){return py::cast(val);}
                );
            }
            for (const auto& [key, values] : i_props) {
                result[key.c_str()] = extract_nullable_values_<int>(
                    values, [](const int val){return py::cast(val);}
                );
            }
            for (const auto& [key, values] : b_props) {
                result[key.c_str()] = extract_nullable_values_<BoolProperty>(
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