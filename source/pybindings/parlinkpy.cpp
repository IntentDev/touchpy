#include "parlinkpy.h"
#include "parlink.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/variant.h>


namespace nb = nanobind;
using namespace nb::literals;
using namespace std;

void initParLinkBindings(nb::module_& m)
{
	nb::class_<Int2> int2(m, "Int2");
	int2.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int2::x).def_rw("y", &Int2::y)
		.def("__repr__", [](Int2& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ")"; });

	nb::class_<Int3> int3(m, "Int3");
	int3.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int3::x).def_rw("y", &Int3::y).def_rw("z", &Int3::z)
		.def("__repr__", [](Int3& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ", " + to_string(self.z) + ")"; });

	nb::class_<Int4> int4(m, "Int4");
	int4.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int4::x).def_rw("y", &Int4::y).def_rw("z", &Int4::z).def_rw("w", &Int4::w)
		.def("__repr__", [](Int4& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ", " + to_string(self.z) + ", " + to_string(self.w) + ")"; });

	nb::class_<Double2> double2(m, "Float2");
	double2.def(nb::init<double, double>()).def_rw("x", &Double2::x).def_rw("y", &Double2::y)
		.def("__repr__", [](Double2& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ")"; });

	nb::class_<Double3> double3(m, "Float3");
	double3.def(nb::init<double, double, double>()).def_rw("x", &Double3::x).def_rw("y", &Double3::y).def_rw("z", &Double3::z)
		.def("__repr__", [](Double3& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ", " + to_string(self.z) + ")"; });

	nb::class_<Double4> double4(m, "Float4");
	double4.def(nb::init<double, double, double, double>())
		.def_rw("x", &Double4::x).def_rw("y", &Double4::y).def_rw("z", &Double4::z).def_rw("w", &Double4::w)
		.def("__repr__", [](Double4& self) { return "(" + to_string(self.x) + ", " + to_string(self.y) + ", " + to_string(self.z) + ", " + to_string(self.w) + ")"; });

	nb::class_<Color> color(m, "Color");
	color.def(nb::init<double, double, double, double>(), "r"_a = 1.0, "g"_a = 1.0, "b"_a = 1.0, "a"_a = 1.0)
		.def_rw("r", &Color::r).def_rw("g", &Color::g).def_rw("b", &Color::b).def_rw("a", &Color::a)
		.def("__repr__", [](Color& self) { return "(" + to_string(self.r) + ", " + to_string(self.g) + ", " + to_string(self.b) + ", " + to_string(self.a) + ")"; });

	nb::class_ <ParLink> par(m, "Par");
	par.doc() = "A parameter in a TouchDesigner component";
	par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &ParLink::set)
		.def("get", &ParLink::get)
		.def_prop_rw("val", &ParLink::get, &ParLink::set)
		;

	nb::class_ <Int2ParLink> int2Par(m, "Int2Par");
	int2Par.doc() = "An int2 parameter in a TouchDesigner component";
	int2Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int2ParLink::set)
		.def("get", &Int2ParLink::get)
		.def_prop_rw("val", &Int2ParLink::get, &Int2ParLink::set)
		.def_prop_rw("x", &Int2ParLink::getX, &Int2ParLink::setX)
		.def_prop_rw("y", &Int2ParLink::getY, &Int2ParLink::setY)
		.def("set", [](Int2ParLink& self, int32_t x = 0, int32_t y = 0) { self.set(Int2(x, y)); }, "x"_a = 0, "y"_a = 0)
		.def("set", [](Int2ParLink& self, const nb::list& list)
			{
				if (list.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Int2(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1])
				);
				self.set(val);
			})
		.def("set", [](Int2ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Int2(
					nb::cast<int32_t>(tuple[0]),
					nb::cast<int32_t>(tuple[1])
				);
				self.set(val);
			})

		;

	nb::class_ <Int3ParLink> int3Par(m, "Int3Par");
	int3Par.doc() = "An int3 parameter in a TouchDesigner component";
	int3Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int3ParLink::set)
		.def("get", &Int3ParLink::get)
		.def_prop_rw("val", &Int3ParLink::get, &Int3ParLink::set)
		.def_prop_rw("x", &Int3ParLink::getX, &Int3ParLink::setX)
		.def_prop_rw("y", &Int3ParLink::getY, &Int3ParLink::setY)
		.def_prop_rw("z", &Int3ParLink::getZ, &Int3ParLink::setZ)
		.def("set", [](Int3ParLink& self, int32_t x = 0, int32_t y = 0, int32_t z = 0)
			{
				self.set(Int3(x, y, z));
			}, "x"_a = 0, "y"_a = 0, "z"_a = 0)

		.def("set", [](Int3ParLink& self, const nb::list& list)
			{
				if (list.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Int3(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1]),
					nb::cast<int32_t>(list[2])
				);
				self.set(val);
			})
		.def("set", [](Int3ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Int3(
					nb::cast<int32_t>(tuple[0]),
					nb::cast<int32_t>(tuple[1]),
					nb::cast<int32_t>(tuple[2])
				);
				self.set(val);
			})
		;

	nb::class_ <Int4ParLink> int4Par(m, "Int4Par");
	int4Par.doc() = "An int4 parameter in a TouchDesigner component";
	int4Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int4ParLink::set)
		.def("get", &Int4ParLink::get)
		.def_prop_rw("val", &Int4ParLink::get, &Int4ParLink::set)
		.def_prop_rw("x", &Int4ParLink::getX, &Int4ParLink::setX)
		.def_prop_rw("y", &Int4ParLink::getY, &Int4ParLink::setY)
		.def_prop_rw("z", &Int4ParLink::getZ, &Int4ParLink::setZ)
		.def_prop_rw("w", &Int4ParLink::getW, &Int4ParLink::setW)
		.def("set", [](Int4ParLink& self, int32_t x = 0, int32_t y = 0, int32_t z = 0, int32_t w = 0)
			{
				self.set(Int4(x, y, z, w));
			}, "x"_a = 0, "y"_a = 0, "z"_a = 0, "w"_a = 0)

		.def("set", [](Int4ParLink& self, const nb::list& list)
			{
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Int4(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1]),
					nb::cast<int32_t>(list[2]),
					nb::cast<int32_t>(list[3])
				);
				self.set(val);
			})
		.def("set", [](Int4ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Int4(
					nb::cast<int32_t>(tuple[0]),
					nb::cast<int32_t>(tuple[1]),
					nb::cast<int32_t>(tuple[2]),
					nb::cast<int32_t>(tuple[3])
				);
				self.set(val);
			})
		;

	nb::class_ <Double2ParLink> double2Par(m, "Double2Par");
	double2Par.doc() = "A float2 parameter in a TouchDesigner component";
	double2Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double2ParLink::set)
		.def("get", &Double2ParLink::get)
		.def_prop_rw("val", &Double2ParLink::get, &Double2ParLink::set)
		.def_prop_rw("x", &Double2ParLink::getX, &Double2ParLink::setX)
		.def_prop_rw("y", &Double2ParLink::getY, &Double2ParLink::setY)
		.def("set", [](Double2ParLink& self, double x = 0., double y = 0.)
			{
				self.set(Double2(x, y));
			}, "x"_a = 0., "y"_a = 0.)

		.def("set", [](Double2ParLink& self, const nb::list& list)
			{
				if (list.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Double2(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1])
				);
				self.set(val);
			})

		.def("set", [](Double2ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Double2(
					nb::cast<double>(tuple[0]),
					nb::cast<double>(tuple[1])
				);
				self.set(val);
			})
		;

	nb::class_ <Double3ParLink> double3Par(m, "Double3Par");
	double3Par.doc() = "A float3 parameter in a TouchDesigner component";
	double3Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double3ParLink::set)
		.def("get", &Double3ParLink::get)
		.def_prop_rw("val", &Double3ParLink::get, &Double3ParLink::set)
		.def_prop_rw("x", &Double3ParLink::getX, &Double3ParLink::setX)
		.def_prop_rw("y", &Double3ParLink::getY, &Double3ParLink::setY)
		.def_prop_rw("z", &Double3ParLink::getZ, &Double3ParLink::setZ)
		.def("set", [](Double3ParLink& self, double x = 0., double y = 0., double z = 0.)
			{
				self.set(Double3(x, y, z));
			}, "x"_a = 0., "y"_a = 0., "z"_a = 0.)
		.def("set", [](Double3ParLink& self, const nb::list& list)
			{
				if (list.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Double3(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1]),
					nb::cast<double>(list[2])
				);
				self.set(val);
			}, "values"_a)
		.def("set", [](Double3ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Double3(
					nb::cast<double>(tuple[0]),
					nb::cast<double>(tuple[1]),
					nb::cast<double>(tuple[2])
				);
				self.set(val);
			}, "values"_a)

				;

	nb::class_ <Double4ParLink> double4Par(m, "Double4Par");
	double4Par.doc() = "A float4 parameter in a TouchDesigner component";
	double4Par.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double4ParLink::set)
		.def("get", &Double4ParLink::get)
		.def_prop_rw("val", &Double4ParLink::get, &Double4ParLink::set)
		.def_prop_rw("x", &Double4ParLink::getX, &Double4ParLink::setX)
		.def_prop_rw("y", &Double4ParLink::getY, &Double4ParLink::setY)
		.def_prop_rw("z", &Double4ParLink::getZ, &Double4ParLink::setZ)
		.def_prop_rw("w", &Double4ParLink::getW, &Double4ParLink::setW)
		.def("set", [](Double4ParLink& self, double x = 0., double y = 0., double z = 0., double w = 0.)
			{
				self.set(Double4(x, y, z, w));
			}, "x"_a = 0., "y"_a = 0., "z"_a = 0., "w"_a = 0.)

		.def("set", [](Double4ParLink& self, const nb::list& list)
			{
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Double4(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1]),
					nb::cast<double>(list[2]),
					nb::cast<double>(list[3])
				);
				self.set(val);
			})
		.def("set", [](Double4ParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Double4(
					nb::cast<double>(tuple[0]),
					nb::cast<double>(tuple[1]),
					nb::cast<double>(tuple[2]),
					nb::cast<double>(tuple[3])
				);
				self.set(val);
			})
		;

	nb::class_ <ColorParLink> colorPar(m, "ColorPar");
	colorPar.doc() = "A color parameter in a TouchDesigner component";
	colorPar.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &ColorParLink::set)
		.def("get", &ColorParLink::get)
		.def_prop_rw("val", &ColorParLink::get, &ColorParLink::set)
		.def_prop_rw("r", &ColorParLink::getR, &ColorParLink::setR)
		.def_prop_rw("g", &ColorParLink::getG, &ColorParLink::setG)
		.def_prop_rw("b", &ColorParLink::getB, &ColorParLink::setB)
		.def_prop_rw("a", &ColorParLink::getA, &ColorParLink::setA)
		.def("set", [](ColorParLink& self, double r = 1., double g = 1., double b = 1., double a = 1.)
			{
				self.set(Color(r, g, b, a));
			}, "r"_a = 1., "g"_a = 1., "b"_a = 1., "a"_a = 1.)

		.def("set", [](ColorParLink& self, const nb::list& list)
			{
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Color(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1]),
					nb::cast<double>(list[2]),
					nb::cast<double>(list[3])
				);
				self.set(val);
			})
		.def("set", [](ColorParLink& self, const nb::tuple& tuple)
			{
				if (tuple.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Color(
					nb::cast<double>(tuple[0]),
					nb::cast<double>(tuple[1]),
					nb::cast<double>(tuple[2]),
					nb::cast<double>(tuple[3])
				);
				self.set(val);
			})
		;

	nb::class_ <PulseParLink> pulsePar(m, "PulsePar");
	pulsePar.doc() = "A pulse parameter in a TouchDesigner component";
	pulsePar.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &PulseParLink::set)
		.def("pulse", &PulseParLink::pulse)
		;

	nb::class_<ParLinkCollection> parCollection(m, "ParCollection");
	parCollection.doc() = "A collection of par Par objects";
	parCollection.def(nb::init<>())
		.def_prop_ro("count", [](ParLinkCollection& self) { return self.size(); })
		.def_prop_ro("names", [](ParLinkCollection& self) { return self.getParNames(); })
		.def("__getitem__", [](ParLinkCollection& self, const std::string& name) -> std::shared_ptr<ParLink>
			{ return self.getParLinkByName(name); }, nb::rv_policy::reference_internal)
		;
}