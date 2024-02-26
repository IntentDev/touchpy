#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory> 

#include <TouchEngine/TouchEngine.h>

// TODO - import Color class from Vision

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
	ColorRGBA(double r = 0.0, double g = 0.0, double b = 0.0, double a = 0.0) : r(r), g(g), b(b), a(a) { }
};
static_assert(sizeof(ColorRGBA) == sizeof(double) * 4, "ColorRGBA size does not match expected size.");
static_assert(std::is_standard_layout_v<ColorRGBA>, "ColorRGBA must be standard layout.");

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


using ParValue = std::variant<
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

class Par
{
public:
	Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : instance(instance), linkInfo(linkInfo) { }
	virtual ~Par() = default;
	virtual void set(ParValue value) = 0;
	virtual void setPending() = 0;
	virtual ParValue get() = 0;

	bool pending() const { return isPending; }

protected:
	TouchObject<TEInstance> instance;
	TouchObject<TELinkInfo> linkInfo;
	bool                    isPending { false };

	void noGetLinkError() const
	{
		std::string message = linkInfo->name;
		message = "Failed to get value from link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to get value from link");
	}

	void noSetLinkError() const
	{
		std::string message = linkInfo->name;
		message = "Failed to set value to link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to set value to link");
	}

	void noGetChoicesError() const
	{
		std::string message = linkInfo->name;
		message = "Failed to get choices from link: " + message;
		std::cout << message << std::endl;
		throw std::runtime_error("Failed to get choices from link");
	}

};

class BoolPar : public Par
{
public:
	BoolPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetBooleanValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<bool>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetBooleanValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetBooleanValue(instance, linkInfo->identifier, value_) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}


private:
	bool value_;
};

class PulsePar : public Par
{
	public:
	PulsePar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) { }

	void set(ParValue value) override { isPending = true; }

	ParValue get() override 
	{ 
		if (!isPending)
			return false; 

		return true;
	}

	void pulse() { isPending = true; }

	void setPending() override
	{
		if (TEInstanceLinkSetBooleanValue(instance, linkInfo->identifier, true) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

};

class MomentaryPar : public Par
{
public:
	MomentaryPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) { }

	void set(ParValue value) override { isPending = true; }

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetBooleanValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void pulse() { isPending = true; }

	void setPending() override
	{
		if (TEInstanceLinkSetBooleanValue(instance, linkInfo->identifier, true) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	bool value_ { false };

};


class StringPar : public Par
{
public:
	StringPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		TEString* val = nullptr;
		if (TEInstanceLinkGetStringValue(instance, linkInfo->identifier, TELinkValueCurrent, &val) != TEResultSuccess)
			noGetLinkError();

		value_ = val->string;
	}

	void set(ParValue value) override
	{
		value_ = std::get<std::string>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
		{
			TEString* val = nullptr;
			if (TEInstanceLinkGetStringValue(instance, linkInfo->identifier, TELinkValueCurrent, &val) != TEResultSuccess)
				noGetLinkError();

			value_ = val->string;
		}

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetStringValue(instance, linkInfo->identifier, value_.c_str()) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	std::string value_;

};

class IntPar : public Par
{
public:
	IntPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<int32_t>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if(!isPending)
			if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetIntValue(instance, linkInfo->identifier, &value_, linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	int32_t value_;
};


class Int2Par : public Par
{
public:
	Int2Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Int2>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetIntValue(instance, linkInfo->identifier, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Int2 value_;
};

class Int3Par : public Par
{
public:
	Int3Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Int3>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetIntValue(instance, linkInfo->identifier, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Int3 value_;
};

class Int4Par : public Par
{
public:
	Int4Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Int4>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetIntValue(instance, linkInfo->identifier, reinterpret_cast<int32_t*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Int4 value_;
};

class DoublePar : public Par
{
public:
	DoublePar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<double>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetDoubleValue(instance, linkInfo->identifier, &value_, linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	double value_;
};

class Double2Par : public Par
{
public:
	Double2Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Double2>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetDoubleValue(instance, linkInfo->identifier, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Double2 value_;
};


class Double3Par : public Par
{
public:
	Double3Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Double3>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetDoubleValue(instance, linkInfo->identifier, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Double3 value_;

};

class Double4Par : public Par
{
public:
	Double4Par(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{
		if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<Double4>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetDoubleValue(instance, linkInfo->identifier, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
	Double4 value_;

};



class MenuPar : public Par
{
public:
	MenuPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<int32_t>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetIntValue(instance, linkInfo->identifier, TELinkValueCurrent, &value_, linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetIntValue(instance, linkInfo->identifier, &value_, linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}


	std::vector<std::string> getNames()
	{
		std::vector<std::string> names;
		TouchObject<TEStringArray> stringArray;

		if (TEInstanceLinkGetChoiceValues(instance, linkInfo->identifier, stringArray.take()) != TEResultSuccess)
			noGetChoicesError();

		for (int i = 0; i < stringArray->count; i++)
		{
			names.push_back(stringArray->strings[i]);
		}

		return names;
	}

private:
	int32_t value_;
};


class ColorPar : public Par 
{
public:
	ColorPar(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Par(instance, linkInfo) 
	{ 
		if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noGetLinkError();
	}

	void set(ParValue value) override
	{
		value_ = std::get<ColorRGBA>(value);
		isPending = true;
	}

	ParValue get() override
	{
		if (!isPending)
			if (TEInstanceLinkGetDoubleValue(instance, linkInfo->identifier, TELinkValueCurrent, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
				noGetLinkError();

		return value_;
	}

	void setPending() override
	{
		if (TEInstanceLinkSetDoubleValue(instance, linkInfo->identifier, reinterpret_cast<double*>(&value_), linkInfo->count) != TEResultSuccess)
			noSetLinkError();
		isPending = false;
	}

private:
		ColorRGBA value_;
	
};

class ParCollection
{   
public:
	ParCollection () = default;
	ParCollection(TouchObject<TEInstance> instance) : instance_(instance) { }

	void setPending()
	{
		for (auto& pair : pars)
		{
			if(pair.second->pending())
				pair.second->setPending();
		}
	}

	void addPar(TouchObject<TELinkInfo> linkInfo)
	{
		TELinkType type = linkInfo->type;

		switch (type)
		{
		case (TELinkTypeDouble):
		{
			addDoublePar(linkInfo);
			break;
		}
		case (TELinkTypeInt):
		{
			addIntPar(linkInfo);
			break;
		}
		case (TELinkTypeString):
		{
			addStringPar(linkInfo);
			break;
		}
		case (TELinkTypeBoolean):
		{
			addBoolPar(linkInfo);
			break;
		}
		default:
			break;
		}
	}

	void reset() { pars.clear(); }

	Par& operator[](const std::string& name) {
		return *pars[name];
	}

	std::unordered_map<std::string, std::unique_ptr<Par>>& getPars() { return pars; }

private:
	TouchObject<TEInstance> instance_;
	std::unordered_map<std::string, std::unique_ptr<Par>> pars;

	void addDoublePar(TouchObject<TELinkInfo> linkInfo)
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
				pars[name] = std::make_unique<DoublePar>(instance_, linkInfo);
				break;
			case 2:
				pars[name] = std::make_unique<Double2Par>(instance_, linkInfo);
				break;
			case 3:
				pars[name] = std::make_unique<Double3Par>(instance_, linkInfo);
				break;
			case 4:
				pars[name] = std::make_unique<Double4Par>(instance_, linkInfo);
				break;
			default:
				break;
			}
			break;

		case TELinkIntentColorRGBA:
			pars[name] = std::make_unique<ColorPar>(instance_, linkInfo);
			break;

		case TELinkIntentSizeWH:
			pars[name] = std::make_unique<Double2Par>(instance_, linkInfo);
			break;

		case TELinkIntentUVW:
			pars[name] = std::make_unique<Double3Par>(instance_, linkInfo);
			break;

		case TELinkIntentPositionXYZW:
			pars[name] = std::make_unique<Double4Par>(instance_, linkInfo);
			break;

		default:
			break;
		}
	}

	void addIntPar(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		int32_t count = linkInfo->count;

		if (!TEInstanceLinkHasChoices(instance_, linkInfo->identifier))
		{
			switch (count)
			{
			case 1:
				pars[name] = std::make_unique<IntPar>(instance_, linkInfo);
				break;

			case 2:
				pars[name] = std::make_unique<Int2Par>(instance_, linkInfo);
				break;

			case 3:
				pars[name] = std::make_unique<Int3Par>(instance_, linkInfo);
				break;

			case 4:
				pars[name] = std::make_unique<Int4Par>(instance_, linkInfo);
				break;

			default:
				break;
			}
		}
		else
		{
			pars[name] = std::make_unique<MenuPar>(instance_, linkInfo);
		}
	}

	void addStringPar(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		pars[name] = std::make_unique<StringPar>(instance_, linkInfo);


};

	void addBoolPar(TouchObject<TELinkInfo> linkInfo)
	{
		std::string name = linkInfo->name;
		TELinkIntent intent = linkInfo->intent;

		switch (intent)
		{
		case (TELinkIntentNotSpecified):
			pars[name] = std::make_unique<BoolPar>(instance_, linkInfo);
			break;

		case (TELinkIntentPulse):
			pars[name] = std::make_unique<PulsePar>(instance_, linkInfo);
			break;

		case (TELinkIntentMomentary):
			pars[name] = std::make_unique<MomentaryPar>(instance_, linkInfo);
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