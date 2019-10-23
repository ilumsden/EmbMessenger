#include <PyEmbMessenger/pybuffer.hpp>

PYBIND11_MODULE(ibuffer, m) {
    py::class<emb::shared::IBuffer, PyBuffer>(m, "IBuffer")
        .def(py::init<>())
        .def("writeByte", &IBuffer::writeByte)
        .def("peek", &IBuffer::peek)
        .def("readByte", &IBuffer::readByte)
        .def("empty", &IBuffer::empty)
        .def("size", &IBuffer::size)
        .def("messages", &IBuffer::messages)
        .def("update", &IBuffer::update)
        .def("zero", &IBuffer::zero)
        .def("debugPrint", &IBuffer::print);
}
