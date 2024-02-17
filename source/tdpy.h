// tdpy.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <pybind11/pybind11.h>

#include "cpythonfuncs.h"
#include "renderer.h"
#include "tdtox.h"

#include <iostream>
#include <memory>
#include <string>

namespace py = pybind11;

void load_tox(std::string filePath)
{
	std::unique_ptr<TdTox> tox = std::make_unique<TdTox>(filePath);
	tox->load();

}

void create_vk_instance()
{
    std::unique_ptr<Renderer> context = std::make_unique<Renderer>();
    context->createInstance();

}

void init_vk()
{
    std::unique_ptr<Renderer> context = std::make_unique<Renderer>();
    context->createInstance();
    context->init();
}


void hello()
{
    py::print("Hello, World!");
}

// Wrapper function that pybind11 will expose to Python
py::object high_performance_function_wrapper(py::args args) 
{
    // Convert pybind11 args to a PyObject* tuple expected by the C API
    PyObject* args_tuple = args.ptr();

    // Now call the C API function directly with the converted arguments
    PyObject* result = high_performance_function(Py_None, args_tuple);

    // Convert the result back to a pybind11 object to return
    py::object py_result = py::reinterpret_borrow<py::object>(result);

    // Handle reference counting for the result
    Py_XDECREF(result);

    return py_result;
}

py::object hello_fast()
{

    // call the C API function directly with the converted arguments
    PyObject* result = hello_fast_imp(Py_None);

    // Convert the result back to a pybind11 object to return
    py::object py_result = py::reinterpret_borrow<py::object>(result);

    // Handle reference counting for the result
    Py_XDECREF(result);

    return py_result;

}