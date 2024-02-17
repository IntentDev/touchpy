// tdpy.cpp : Defines the entry point for the application.
//

#include "tdpy.h"
#include "renderer.h"






PYBIND11_MODULE(tdpy, m) 
{
    m.doc() = "pybind11 example plugin"; // Optional module docstring

    py::class_<TdTox>(m, "TdTox")
		.def(py::init<std::string>())
		.def("load", &TdTox::load)
		.def("configured", &TdTox::didConfigure)
		.def("loaded", &TdTox::loaded)
		.def("update", &TdTox::update);



    m.def("create_vk_instance", &create_vk_instance, "Create VkInstance");
    m.def("init_vk", &init_vk, "Create Vulkan Device");
    m.def("load_tox", &load_tox, "Load a tox file");

    m.def("hello", &hello, "A function that prints 'Hello, World!'");
    m.def("hello_fast", &hello_fast, "A function defined in C");
    m.def("add_fast", &high_performance_function_wrapper, "A function defined in C");


}

