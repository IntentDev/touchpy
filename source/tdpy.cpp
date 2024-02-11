// tdpy.cpp : Defines the entry point for the application.
//

#include "tdpy.h"
#include "vkcontext.h"






PYBIND11_MODULE(tdpy, m) 
{
    m.doc() = "pybind11 example plugin"; // Optional module docstring

    m.def("create_vk_instance", &create_vk_instance, "Create VkInstance");
    m.def("init_vk", &init_vk, "Create Vulkan Device");

    m.def("hello", &hello, "A function that prints 'Hello, World!'");
    m.def("hello_fast", &hello_fast, "A function defined in C");
    m.def("add_fast", &high_performance_function_wrapper, "A function defined in C");


}

