#include <Python.h>

// Example function using the Python C API to be wrapped by pybind11 
// assume it takes two integers and returns their sum as an example
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

