#include <EmbMessenger/IBuffer.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

class PyBuffer : public emb::shared::IBuffer
{
    public:

        using emb::shared::IBuffer::IBuffer;

        void writeByte(const uint8_t byte) override 
        {
            PYBIND11_OVERLOAD_PURE(
                void,
                emb::shared::IBuffer,
                writeByte,
                byte
            );
        }

        uint8_t peek() const override
        {
            PYBIND11_OVERLOAD_PURE(
                uint8_t,
                emb::shared::IBuffer,
                peek,
            );
        }

        uint8_t readByte() override
        {
            PYBIND11_OVERLOAD_PURE(
                uint8_t,
                emb::shared::IBuffer,
                readByte,
            );
        }

        bool empty() const override 
        {
            PYBIND11_OVERLOAD_PURE(
                bool,
                emb::shared::IBuffer,
                empty,
            );
        }

        size_t size() const override
        {
            PYBIND11_OVERLOAD_PURE(
                size_t,
                emb::shared::IBuffer,
                size,
            );
        }

        uint8_t messages() const override
        {
            PYBIND11_OVERLOAD_PURE(
                uint8_t,
                emb::shared::IBuffer,
                messages,
            );
        }

        void update() override
        {
            PYBIND11_OVERLOAD_PURE(
                void,
                emb::shared::IBuffer,
                update,
            );
        }

        void zero() override
        {
            PYBIND11_OVERLOAD_PURE(
                void,
                emb::shared::IBuffer,
                zero,
            );
        }

        void print() const override
        {
            PYBIND11_OVERLOAD_NAME(
                void,
                emb::shared::IBuffer,
                print,
                "debugPrint",
            );
        }
};

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
