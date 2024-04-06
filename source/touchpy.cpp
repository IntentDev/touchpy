#include <nanobind/nanobind.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/async.h>
#include <mutex>
#include <iostream>
namespace nb = nanobind;

template<typename Mutex>
class PythonSink : public spdlog::sinks::base_sink<Mutex>
{
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {

        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.raw contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        nb::gil_scoped_acquire acquire;
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        //std::cout << fmt::to_string(formatted);
        nb::print(fmt::to_string(formatted).c_str());
    }

    void flush_() override
    {
        nb::gil_scoped_acquire acquire;
        std::cout << std::flush;
    }
};

void init_logging()
{
    //auto python_sink = std::make_shared<PythonSink<spdlog::details::null_mutex>>(); // single threaded
    //auto logger = std::make_shared<spdlog::logger>("python_logger", python_sink);
    auto python_sink = std::make_shared<PythonSink<std::mutex>>(); // Use mutex for thread safety
    auto logger = std::make_shared<spdlog::logger>("python_logger", std::move(python_sink));
    logger->set_pattern("[%M:%S] [%t] [%l]: %v");
    spdlog::register_logger(logger);
}

extern void initCompBindings(nb::module_& m);
extern void initTopLinkBindings(nb::module_& m);
extern void initChopLinkBindings(nb::module_& m);
extern void initDatLinkBindings(nb::module_& m);
extern void initParLinkBindings(nb::module_& m);

NB_MODULE(touchpy, m)
{
	//nb::set_leak_warnings(false);
    init_logging();
    //m.def("init_logging", &init_logging, "logCallback"_a,  "Initialize logging with a Python sink.");

	initCompBindings(m);
	initTopLinkBindings(m);
	initChopLinkBindings(m);
	initDatLinkBindings(m);
	initParLinkBindings(m);
}