#ifndef PY_EMB_MSGER_CLASS_HPP
#define PY_EMB_MSGER_CLASS_HPP

#include <vector>

#include <EmbMessenger/EmbMessenger.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

class PyEmbMessenger : public emb::host::EmbMessenger
{
    public:
#ifdef EMB_SINGLE_THREADED

#else

#endif

    // Add something to handle to registerCommand method
    
    void write(std::vector<bool> args)
    {
        
    }

};

#endif
