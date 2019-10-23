cmake_minimum_required(VERSION 3.0.0)

project(PyEmbMessenger)

find_package(pybind11 REQUIRED)

pybind11_add_module(pybuffer pybuffer.cpp)
pybind11_add_module(pycommand pycommand.cpp)
pybind11_add_module(pyembmsger pyembmsger.cpp)
