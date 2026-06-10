
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "reader_bindings.hpp"
#include "block_bindings.hpp"
#include "iblock_bindings.hpp"
#include "writer_bindings.hpp"


namespace pymae {
PYBIND11_MODULE(pymae, m) {
    m.doc();
    bind_reader(m);
    bind_writer(m);
    bind_block(m);
    bind_indexed_block(m);
}
} // namespace pymae
