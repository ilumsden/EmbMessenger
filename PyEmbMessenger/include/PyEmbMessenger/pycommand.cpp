#ifndef PYCOMMAND_HPP
#define PYCOMMAND_HPP

#include <EmbMessenger/Command.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

class PyCommand : public emb::host::Command
{
    public:

        using emb::host::Command::Command;

        void send
};

#endif
