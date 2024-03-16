#pragma once

#include "links.h"

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory> 

#include <TouchEngine/TouchEngine.h>
#include <iostream>

// TODO - import Color class from Vision

struct Int2
{
	int32_t x, y;
	Int2(int32_t x = 0, int32_t y = 0) : x(x), y(y) { }
};
static_assert(sizeof(Int2) == sizeof(int32_t) * 2, "Int2 size does not match expected size.");
static_assert(std::is_standard_layout_v<Int2>, "Int2 must be standard layout.");

struct Int3
{
	int32_t x, y, z;
	Int3(int32_t x = 0, int32_t y = 0, int32_t z = 0) : x(x), y(y), z(z) { }
};
static_assert(sizeof(Int3) == sizeof(int32_t) * 3, "Int3 size does not match expected size.");
static_assert(std::is_standard_layout_v<Int3>, "Int3 must be standard layout.");

struct Int4
{
	int32_t x, y, z, w;
	Int4(int32_t x = 0, int32_t y = 0, int32_t z = 0, int32_t w = 0) : x(x), y(y), z(z), w(w) { }
};
static_assert(sizeof(Int4) == sizeof(int32_t) * 4, "Int4 size does not match expected size.");
static_assert(std::is_standard_layout_v<Int4>, "Int4 must be standard layout.");

struct Double2
{
	double x, y;
	Double2(double x = 0.0, double y = 0.0) : x(x), y(y) { }
};
static_assert(sizeof(Double2) == sizeof(double) * 2, "Double2 size does not match expected size.");
static_assert(std::is_standard_layout_v<Double2>, "Double2 must be standard layout.");

struct Double3
{
	double x, y, z;
	Double3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) { }
};
static_assert(sizeof(Double3) == sizeof(double) * 3, "Double3 size does not match expected size.");
static_assert(std::is_standard_layout_v<Double3>, "Double3 must be standard layout.");

struct Double4
{
	double x, y, z, w;
	Double4(double x = 0.0, double y = 0.0, double z = 0.0, double w = 0.0) : x(x), y(y), z(z), w(w) { }
};
static_assert(sizeof(Double4) == sizeof(double) * 4, "Double4 size does not match expected size.");
static_assert(std::is_standard_layout_v<Double4>, "Double4 must be standard layout.");

struct ColorRGBA
{
	double r, g, b, a;
	ColorRGBA(double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0) : r(r), g(g), b(b), a(a) { }
};
static_assert(sizeof(ColorRGBA) == sizeof(double) * 4, "ColorRGBA size does not match expected size.");
static_assert(std::is_standard_layout_v<ColorRGBA>, "ColorRGBA must be standard layout.");

using ParLinkValue = std::variant<
	bool,
	std::string,
	int32_t,
	Int2,
	Int3,
	Int4,
	double,
	Double2,
	Double3,
	Double4,
	ColorRGBA
>;

class ParLink : public Link<ParLink>
{
public:
	ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) :  Link<ParLink>(instance, linkInfo) { }
	virtual ~ParLink() = default;

	virtual void set(ParLinkValue value) { }
	virtual ParLinkValue get() { return ParLinkValue(); }


protected:
	bool isPending { false };

	void noGetLinkError() const
	{
		std::string message = name_;
		message = "Failed to get value from link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to get value from link");
	}

	void noSetLinkError() const
	{
		std::string message = name_;
		message = "Failed to set value to link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to set value to link");
	}

	void noGetChoicesError() const
	{
		std::string message = name_;
		message = "Failed to get choices from link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to get choices from link");
	}

};

class BoolParLink : public ParLink
{
public:
	BoolParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetBooleanValue(instance_, identifier_.c_str(), std::get<bool>(value)) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		bool value;
		if (TEInstanceLinkGetBooleanValue(instance_, identifier_.c_str(), TELinkValueCurrent, &value) != TEResultSuccess)
				noGetLinkError();
		return value;
	}
};

class PulseParLink : public ParLink
{
	public:
	PulseParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetBooleanValue(instance_, identifier_.c_str(), true) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override { return false; }

	void pulse() 
	{
		if (TEInstanceLinkSetBooleanValue(instance_, identifier_.c_str(), true) != TEResultSuccess)
			noSetLinkError();
	}
};

class MomentaryParLink : public ParLink
{
public:
	MomentaryParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override 
	{ 
		if (TEInstanceLinkSetBooleanValue(instance_, identifier_.c_str(), std::get<bool>(value)) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		bool value;
		if (TEInstanceLinkGetBooleanValue(instance_, identifier_.c_str(), TELinkValueCurrent, &value) != TEResultSuccess)
			noGetLinkError();

		return value;
	}
};


class StringParLink : public ParLink
{
public:
	StringParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) {}
	
	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetStringValue(instance_, identifier_.c_str(), std::get<std::string>(value).c_str()) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		TEString* val = nullptr;
		if (TEInstanceLinkGetStringValue(instance_, identifier_.c_str(), TELinkValueCurrent, &val) != TEResultSuccess)
			noGetLinkError();

		return std::string(val->string);
	}
};

class IntParLink : public ParLink
{
public:
	IntParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetIntValue(instance_, identifier_.c_str(), &std::get<int32_t>(value), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		int32_t value;
		if (TEInstanceLinkGetIntValue(instance_, identifier_.c_str(), TELinkValueCurrent, &value, count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}
};


class Int2ParLink : public ParLink
{
public:
	Int2ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }
	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetIntValue(instance_, identifier_.c_str(), reinterpret_cast<int32_t*>(&std::get<Int2>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Int2 value;
		if (TEInstanceLinkGetIntValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<int32_t*>(&value), count_) != TEResultSuccess)
				noGetLinkError();

		return value;
	}

	void    setX(int32_t x) { Int2 vec = std::get<Int2>(get()); vec.x = x; set(vec); }
	void    setY(int32_t y) { Int2 vec = std::get<Int2>(get()); vec.y = y; set(vec); }
	int32_t getX()          { return std::get<Int2>(get()).x; }
	int32_t getY()          { return std::get<Int2>(get()).y; }
};

class Int3ParLink : public ParLink
{
public:
	Int3ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetIntValue(instance_, identifier_.c_str(), reinterpret_cast<int32_t*>(&std::get<Int3>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Int3 value;
		if (TEInstanceLinkGetIntValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<int32_t*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void    setX(int32_t x) { Int3 vec = std::get<Int3>(get()); vec.x = x; set(vec); }
	void    setY(int32_t y) { Int3 vec = std::get<Int3>(get()); vec.y = y; set(vec); }
	void    setZ(int32_t z) { Int3 vec = std::get<Int3>(get()); vec.z = z; set(vec); }
	int32_t getX()          { return std::get<Int3>(get()).x; }
	int32_t getY()          { return std::get<Int3>(get()).y; }
	int32_t getZ()          { return std::get<Int3>(get()).z; }
};

class Int4ParLink : public ParLink
{
public:
	Int4ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetIntValue(instance_, identifier_.c_str(), reinterpret_cast<int32_t*>(&std::get<Int4>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Int4 value;
		if (TEInstanceLinkGetIntValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<int32_t*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void    setX(int32_t x) { Int4 vec = std::get<Int4>(get()); vec.x = x; set(vec); }
	void    setY(int32_t y) { Int4 vec = std::get<Int4>(get()); vec.y = y; set(vec); }
	void    setZ(int32_t z) { Int4 vec = std::get<Int4>(get()); vec.z = z; set(vec); }
	void    setW(int32_t w) { Int4 vec = std::get<Int4>(get()); vec.w = w; set(vec); }
	int32_t getX()          { return std::get<Int4>(get()).x; }
	int32_t getY()          { return std::get<Int4>(get()).y; }
	int32_t getZ()          { return std::get<Int4>(get()).z; }
	int32_t getW()          { return std::get<Int4>(get()).w; }
};

class DoubleParLink : public ParLink
{
public:
	DoubleParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }


	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetDoubleValue(instance_, identifier_.c_str(), &std::get<double>(value), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		double value;
		if (TEInstanceLinkGetDoubleValue(instance_, identifier_.c_str(), TELinkValueCurrent, &value, count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}
};

class Double2ParLink : public ParLink
{
public:
	Double2ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetDoubleValue(instance_, identifier_.c_str(), reinterpret_cast<double*>(&std::get<Double2>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Double2 value;
		if (TEInstanceLinkGetDoubleValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<double*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void   setX(double x) { Double2 vec = std::get<Double2>(get()); vec.x = x; set(vec); }
	void   setY(double y) { Double2 vec = std::get<Double2>(get()); vec.y = y; set(vec); }
	double getX()         { return std::get<Double2>(get()).x; }
	double getY()         { return std::get<Double2>(get()).y; }
};


class Double3ParLink : public ParLink
{
public:
	Double3ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetDoubleValue(instance_, identifier_.c_str(), reinterpret_cast<double*>(&std::get<Double3>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Double3 value;
		if (TEInstanceLinkGetDoubleValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<double*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void   setX(double x) { Double3 vec = std::get<Double3>(get()); vec.x = x; set(vec); }
	void   setY(double y) { Double3 vec = std::get<Double3>(get()); vec.y = y; set(vec); }
	void   setZ(double z) { Double3 vec = std::get<Double3>(get()); vec.z = z; set(vec); }
	double getX()         { return std::get<Double3>(get()).x; }
	double getY()         { return std::get<Double3>(get()).y; }
	double getZ()         { return std::get<Double3>(get()).z; }
};

class Double4ParLink : public ParLink
{
public:
	Double4ParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetDoubleValue(instance_, identifier_.c_str(), reinterpret_cast<double*>(&std::get<Double4>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		Double4 value;
		if (TEInstanceLinkGetDoubleValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<double*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void   setX(double x) { Double4 vec = std::get<Double4>(get()); vec.x = x; set(vec); }
	void   setY(double y) { Double4 vec = std::get<Double4>(get()); vec.y = y; set(vec); }
	void   setZ(double z) { Double4 vec = std::get<Double4>(get()); vec.z = z; set(vec); }
	void   setW(double w) { Double4 vec = std::get<Double4>(get()); vec.w = w; set(vec); }
	double getX()         { return std::get<Double4>(get()).x; }
	double getY()         { return std::get<Double4>(get()).y; }
	double getZ()         { return std::get<Double4>(get()).z; }
	double getW()         { return std::get<Double4>(get()).w; }


};



class MenuParLink : public ParLink
{
public:
	MenuParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }
	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetIntValue(instance_, identifier_.c_str(), &std::get<int32_t>(value), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		int32_t value;
		if (TEInstanceLinkGetIntValue(instance_, identifier_.c_str(), TELinkValueCurrent, &value, count_) != TEResultSuccess)
				noGetLinkError();

		return value;
	}

	std::vector<std::string> getNames()
	{
		std::vector<std::string> names;
		TouchObject<TEStringArray> stringArray;

		if (TEInstanceLinkGetChoiceValues(instance_, identifier_.c_str(), stringArray.take()) != TEResultSuccess)
			noGetChoicesError();

		for (int i = 0; i < stringArray->count; i++)
		{
			names.push_back(stringArray->strings[i]);
		}

		return names;
	}

};


class ColorParLink : public ParLink 
{
public:
	ColorParLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ParLink(instance, linkInfo) { }

	void set(ParLinkValue value) override
	{
		if (TEInstanceLinkSetDoubleValue(instance_, identifier_.c_str(), reinterpret_cast<double*>(&std::get<ColorRGBA>(value)), count_) != TEResultSuccess)
			noSetLinkError();
	}

	ParLinkValue get() override
	{
		ColorRGBA value;
		if (TEInstanceLinkGetDoubleValue(instance_, identifier_.c_str(), TELinkValueCurrent, reinterpret_cast<double*>(&value), count_) != TEResultSuccess)
			noGetLinkError();

		return value;
	}

	void setR(double r) { ColorRGBA rgba = std::get<ColorRGBA>(get()); rgba.r = r; set(rgba); }
	void setG(double g) { ColorRGBA rgba = std::get<ColorRGBA>(get()); rgba.g = g; set(rgba); }
	void setB(double b) { ColorRGBA rgba = std::get<ColorRGBA>(get()); rgba.b = b; set(rgba); }
	void setA(double a) { ColorRGBA rgba = std::get<ColorRGBA>(get()); rgba.a = a; set(rgba); }
	double getR() { return std::get<ColorRGBA>(get()).r; }
	double getG() { return std::get<ColorRGBA>(get()).g; }
	double getB() { return std::get<ColorRGBA>(get()).b; }
	double getA() { return std::get<ColorRGBA>(get()).a; }
};

class ParLinkCollection
{
public:
	ParLinkCollection() = default;
	ParLinkCollection(TouchObject<TEInstance> instance) : instance_(instance) { }

	void addLink(TouchObject<TELinkInfo> linkInfo)
	{
		TELinkType type = linkInfo->type;

		switch (type)
		{
		case (TELinkTypeDouble):
		{
			addDoubleParLink(linkInfo);
			break;
		}
		case (TELinkTypeInt):
		{
			addIntParLink(linkInfo);
			break;
		}
		case (TELinkTypeString):
		{
			addStringParLink(linkInfo);
			break;
		}
		case (TELinkTypeBoolean):
		{
			addBoolParLink(linkInfo);
			break;
		}
		default:
			break;
		}
	}
	void reset() { pars.clear(); }
	std::unordered_map<std::string, std::shared_ptr<ParLink>>& getLinks() { return pars; }


	const std::shared_ptr<ParLink> getParLinkByName(const std::string& name) const
	{
		return pars.at(name);
	}

	ParLink& operator[](const std::string& name) {
		return *pars[name];
	}

	std::vector<std::string_view> getParNames() const
	{
		std::vector<std::string_view> names;
		for (const auto& [name, link] : pars)
		{
			names.push_back(name);
		}
		return names;
	}

	//std::vector<std::string> getParNames() const
	//{
	//	std::vector<std::string> names;
	//	for (const auto& [name, link] : pars)
	//	{
	//		names.push_back(name);
	//	}
	//	return names;
	//}


	size_t size() const { return pars.size(); }


private:
	TouchObject<TEInstance> instance_;
	std::unordered_map<std::string, std::shared_ptr<ParLink>> pars;

	void addDoubleParLink(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		TELinkIntent intent = linkInfo->intent;
		int32_t count = linkInfo->count;

		switch (intent)
		{
		case TELinkIntentNotSpecified:
			switch (count)
			{
			case 1:
				pars[name] = std::make_unique<DoubleParLink>(instance_, linkInfo);
				break;
			case 2:
				pars[name] = std::make_unique<Double2ParLink>(instance_, linkInfo);
				break;
			case 3:
				pars[name] = std::make_unique<Double3ParLink>(instance_, linkInfo);
				break;
			case 4:
				pars[name] = std::make_unique<Double4ParLink>(instance_, linkInfo);
				break;
			default:
				break;
			}
			break;

		case TELinkIntentColorRGBA:
			pars[name] = std::make_unique<ColorParLink>(instance_, linkInfo);
			break;

		case TELinkIntentSizeWH:
			pars[name] = std::make_unique<Double2ParLink>(instance_, linkInfo);
			break;

		case TELinkIntentUVW:
			switch (count)
			{
			case 2:
				pars[name] = std::make_unique<Double2ParLink>(instance_, linkInfo);
				break;
			case 3:
				pars[name] = std::make_unique<Double3ParLink>(instance_, linkInfo);
				break;
			}
			break;

		case TELinkIntentPositionXYZW:
			switch (count)
			{
			case 3:
				pars[name] = std::make_unique<Double3ParLink>(instance_, linkInfo);
				break;
			case 4:
				pars[name] = std::make_unique<Double4ParLink>(instance_, linkInfo);
				break;
			}
			break;

		default:
			break;
		}
	}

	void addIntParLink(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		int32_t count = linkInfo->count;

		if (!TEInstanceLinkHasChoices(instance_, linkInfo->identifier))
		{
			switch (count)
			{
			case 1:
				pars[name] = std::make_unique<IntParLink>(instance_, linkInfo);
				break;

			case 2:
				pars[name] = std::make_unique<Int2ParLink>(instance_, linkInfo);
				break;

			case 3:
				pars[name] = std::make_unique<Int3ParLink>(instance_, linkInfo);
				break;

			case 4:
				pars[name] = std::make_unique<Int4ParLink>(instance_, linkInfo);
				break;

			default:
				break;
			}
		}
		else
		{
			pars[name] = std::make_unique<MenuParLink>(instance_, linkInfo);
		}
	}

	void addStringParLink(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		pars[name] = std::make_unique<StringParLink>(instance_, linkInfo);


	};

	void addBoolParLink(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		TELinkIntent intent = linkInfo->intent;

		switch (intent)
		{
		case (TELinkIntentNotSpecified):
			pars[name] = std::make_unique<BoolParLink>(instance_, linkInfo);
			break;

		case (TELinkIntentPulse):
			pars[name] = std::make_unique<PulseParLink>(instance_, linkInfo);
			break;

		case (TELinkIntentMomentary):
			pars[name] = std::make_unique<MomentaryParLink>(instance_, linkInfo);
			break;

		default:
			break;
		}
	}
};

// visitor that attempts to assign only if the type matches
template<typename T>
auto visitor = [](auto&& arg) -> std::optional<T> {
	if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, T>) {
		return arg; // Return the T if arg is an T
	}
	else {
		return std::nullopt; // Return empty optional for other types
	}
	};