// tdpy.h : Include file for standard system include files,
// or project specific include files.

#pragma once
// #include <Python.h> // not needed with pybind11
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "renderer.h"
#include "comp.h"

#include <iostream>
#include <memory>
#include <string>

 //Example function using the Python C API to be wrapped by pybind11 
 //assume it takes two integers and returns their sum as an example
static PyObject* high_performance_function(PyObject* self, PyObject* args)
{
    long a, b;
    if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
        return NULL; // Error parsing arguments
    }
    long result = a + b; // Imagine this is a performance-critical operation
    return PyLong_FromLong(result);
}


static PyObject* hello_fast_imp(PyObject* self)
{
    printf("Hello, World! no pybind11!\n");
    Py_RETURN_NONE; // Return Py_None to Python
}




namespace py = pybind11;

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


//PYBIND11_MODULE(touchpy, m)
//{
//    m.doc() = "Comp: Loads a TouchDesigner .tox file and runs in process.";
//
//    py::class_<Comp> comp(m, "Comp");
//
//    comp.def(py::init<>())
//        .def(py::init<std::string>())
//        .def("load_tox", &Comp::loadTox)
//        .def("loaded", &Comp::loaded)
//        .def("update", &Comp::update)
//        .def("start", &Comp::runUpdateLoop)
//        .def("stop", &Comp::stopUpdateLoop)
//        ;
//
//    comp.def("set_on_frame_start_callback", [](
//        Comp& self,
//        py::function callback,
//        py::object userData)
//        {
//            auto userDataPtr = std::make_shared<py::object>(userData);
//            self.setOnFrameStartCallback(
//                [callback](Comp& comp, std::shared_ptr<void> userData)
//                {
//                    auto userDataPyObj = *std::static_pointer_cast<py::object>(userData);
//                    callback(comp, userDataPyObj);
//                },
//                userDataPtr);
//        });
//
//
//
//
//
//
//    m.def("create_vk_instance", &create_vk_instance, "Create VkInstance");
//    m.def("init_vk", &init_vk, "Create Vulkan Device");
//
//    m.def("hello", &hello, "A function that prints 'Hello, World!'");
//    m.def("hello_fast", &hello_fast, "A function defined in C");
//    m.def("add_fast", &high_performance_function_wrapper, "A function defined in C");
//
//}
