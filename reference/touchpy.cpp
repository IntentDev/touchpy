#include <iostream>
#include <streambuf>
#include <string>
#include <mutex>
#include <nanobind/nanobind.h>


namespace nb = nanobind;

// Buffer that writes to Python instead of C++
class pythonbuf : public std::streambuf {
private:
    using traits_type = std::streambuf::traits_type;

    size_t buf_size;
    std::unique_ptr<char[]> d_buffer;
    nb::object pywrite;
    nb::object pyflush;
    mutable std::mutex buffer_mutex;

    int overflow(int c) override {
        //std::lock_guard<std::mutex> lock(buffer_mutex);
        nb::gil_scoped_acquire acquire;
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(c);
            pbump(1);
        }
        return sync() == 0 ? traits_type::not_eof(c) : traits_type::eof();
    }

    // Computes how many bytes at the end of the buffer are part of an
    // incomplete sequence of UTF-8 bytes.
    // Precondition: pbase() < pptr()
    size_t utf8_remainder() const {
        const auto rbase = std::reverse_iterator<char*>(pbase());
        const auto rpptr = std::reverse_iterator<char*>(pptr());
        auto is_ascii = [](char c) { return (static_cast<unsigned char>(c) & 0x80) == 0x00; };
        auto is_leading = [](char c) { return (static_cast<unsigned char>(c) & 0xC0) == 0xC0; };
        auto is_leading_2b = [](char c) { return static_cast<unsigned char>(c) <= 0xDF; };
        auto is_leading_3b = [](char c) { return static_cast<unsigned char>(c) <= 0xEF; };
        // If the last character is ASCII, there are no incomplete code points
        if (is_ascii(*rpptr)) {
            return 0;
        }
        // Otherwise, work back from the end of the buffer and find the first
        // UTF-8 leading byte
        const auto rpend = rbase - rpptr >= 3 ? rpptr + 3 : rbase;
        const auto leading = std::find_if(rpptr, rpend, is_leading);
        if (leading == rbase) {
            return 0;
        }
        const auto dist = static_cast<size_t>(leading - rpptr);
        size_t remainder = 0;

        if (dist == 0) {
            remainder = 1; // 1-byte code point is impossible
        }
        else if (dist == 1) {
            remainder = is_leading_2b(*leading) ? 0 : dist + 1;
        }
        else if (dist == 2) {
            remainder = is_leading_3b(*leading) ? 0 : dist + 1;
        }
        // else if (dist >= 3), at least 4 bytes before encountering an UTF-8
        // leading byte, either no remainder or invalid UTF-8.
        // Invalid UTF-8 will cause an exception later when converting
        // to a Python string, so that's not handled here.
        return remainder;
    }

    // This function must be non-virtual to be called in a destructor.
    int _sync() {
        //std::lock_guard<std::mutex> lock(buffer_mutex);
        nb::gil_scoped_acquire acquire;
        if (pbase() != pptr()) { 
            
            // If buffer is not empty
            //nb::gil_scoped_acquire tmp;
            // This subtraction cannot be negative, so dropping the sign.
            auto size = static_cast<size_t>(pptr() - pbase());
            size_t remainder = utf8_remainder();

            if (size > remainder) {
                nb::str line(pbase(), size - remainder);
                pywrite(std::move(line));
                pyflush();
            }

            // Copy the remainder at the end of the buffer to the beginning:
            if (remainder > 0) {
                std::memmove(pbase(), pptr() - remainder, remainder);
            }
            setp(pbase(), epptr());
            pbump(static_cast<int>(remainder));
        }
        return 0;
    }

    int sync() override { return _sync(); }

public:
    explicit pythonbuf(const nb::object& pyostream, size_t buffer_size = 1024)
        : buf_size(buffer_size), d_buffer(new char[buf_size]), pywrite(pyostream.attr("write")),
        pyflush(pyostream.attr("flush")) {
        setp(d_buffer.get(), d_buffer.get() + buf_size - 1);
    }

    pythonbuf(pythonbuf&&) = default;

    /// Sync before destroy
    ~pythonbuf() override { _sync(); }
};

class scoped_ostream_redirect
{
protected:
    std::streambuf* old;
    std::ostream& costream;
    pythonbuf buffer;

public:
    explicit scoped_ostream_redirect(
        std::ostream& costream = std::cout,
        const nb::object& pyostream = nb::module_::import_("sys").attr("stdout"))
        noexcept
        : costream(costream), buffer(pyostream)
    {
        old = costream.rdbuf(&buffer);
    }
    ~scoped_ostream_redirect() {
        costream.rdbuf(old);
    }
    scoped_ostream_redirect(const scoped_ostream_redirect&) = delete;
    scoped_ostream_redirect(scoped_ostream_redirect&& other) = default;
    scoped_ostream_redirect& operator=(const scoped_ostream_redirect&) = delete;
    scoped_ostream_redirect& operator=(scoped_ostream_redirect&&) = delete;
};




extern void initCompBindings(nb::module_& m);
extern void initTopLinkBindings(nb::module_& m);
extern void initChopLinkBindings(nb::module_& m);
extern void initDatLinkBindings(nb::module_& m);
extern void initParLinkBindings(nb::module_& m);

NB_MODULE(touchpy, m)
{
	//nb::set_leak_warnings(false);

    nb::class_<scoped_ostream_redirect>(m, "ScopedOstreamRedirect")
        .def(nb::init<>());


	initCompBindings(m);
	initTopLinkBindings(m);
	initChopLinkBindings(m);
	initDatLinkBindings(m);
	initParLinkBindings(m);
}