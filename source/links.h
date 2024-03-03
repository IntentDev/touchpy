#pragma once

#include <TouchEngine/TouchEngine.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <type_traits>

#include "link.h"


// using type_traits to check for the existence of required methods for the link types until C++20/concepts update, 
// we can use std::void_t to check for the existence of a member function
// https://en.cppreference.com/w/cpp/types/void_t
// need upgrade CUDA to 12.x to use C++20, but to need make sure TD will work with it

// check for the existence of a getName() method for the link type
template<typename T, typename = void>
struct has_getName_method : std::false_type {};

template<typename T>
struct has_getName_method<T, std::void_t<decltype(std::declval<T>().getName())>>
	:	std::is_same<decltype(std::declval<T>().getName()), std::string> {};

// check for the existence of an identifier() method for the link type
template<typename T, typename = void>
struct has_getIdentifier_method : std::false_type {};

template<typename T>
struct has_getIdentifier_method<T, std::void_t<decltype(std::declval<T>().getIdentifier())>>
	:	std::is_same<decltype(std::declval<T>().getIdentifier()), std::string> {};

//// check for the existence of an updateOuput() method for the link type
//template<typename T, typename = void>
//struct has_updateOutput_method : std::false_type {};
//
//template<typename T>
//struct has_updateOutput_method<T, std::void_t<decltype(std::declval<T>().updateOutput())>> 
//	:	std::is_same<decltype(std::declval<T>().updateOutput()), void> {};

template <typename Derived, typename T>
class Links 
{
	static_assert(has_getName_method<T>::value, "Type T must have a member function std::string getName()");
	static_assert(has_getIdentifier_method<T>::value, "Type T must have a member function getIdentifier()");
	//static_assert(has_updateOutput_method<T>::value, "Type T must have a member function updateOutput()");

public:

	Links() = default;
	Links(const Links&) = delete;
	Links& operator=(const Links&) = delete;
	Links(Links&&) = delete;
	Links& operator=(Links&&) = delete;
	~Links() = default;


	Links(TouchObject<TEInstance> instance) : instance(instance) { }

	virtual void addLink(TouchObject<TELinkInfo> linkInfo)
	{
		links.push_back(std::make_unique<T>(instance, linkInfo));
		nameMap[linkInfo->name] = links.back().get();
		identifierMap[linkInfo->identifier] = links.back().get();
	}

	void removeLinkByName(const std::string& name)
	{
		auto it = nameMap.find(name);
		if (it != nameMap.end())
		{
			auto link = it->second;
			nameMap.erase(it);
			identifierMap.erase(link->identifier());
			links.erase(std::remove_if(links.begin(), links.end(), [link](const std::unique_ptr<T>& p) { return p.get() == link; }), links.end());
		}
	}

	void removeLinkByIdentifier(const std::string& identifier)
	{
		auto it = identifierMap.find(identifier);
		if (it != identifierMap.end())
		{
			auto link = it->second;
			identifierMap.erase(it);
			nameMap.erase(link->name());
			links.erase(std::remove_if(links.begin(), links.end(), [link](const std::unique_ptr<T>& p) { return p.get() == link; }), links.end());
		}
	}

	T* getLinkByName(const std::string& name)
	{
		auto it = nameMap.find(name);
		if (it != nameMap.end()) return it->second;
		return nullptr;
	}

	T* getLinkByIdentifier(const std::string& identifier)
	{
		auto it = identifierMap.find(identifier);
		if (it != identifierMap.end()) return it->second;
		return nullptr;
	}

	T* getLinkByIndex(size_t index)
	{
		if (index < links.size()) return links[index].get();
		return nullptr;
	}

	bool hasLinkByName(const std::string& name)
	{
		return getLinkByName(name) != nullptr;
	}

	bool hasLinkByIdentifier(const std::string& identifier)
	{
		return getLinkByIdentifier(identifier) != nullptr;
	}

	size_t size() const { return links.size(); }

	T& operator[](const std::string& name) 
	{ 
		auto link = getLinkByName(name);
		if (!link) throw std::runtime_error("Link not found");
		return *link; 
	}

	void reset()
	{
		links.clear();
	}

	T& operator[](size_t index)
	{
		auto link = getLinkByIndex(index);
		if (!link) throw std::runtime_error("Link not found");
		return *link; 
	}

	// iterator support
	auto& begin()  { return links.begin(); }
	auto& end()    { return links.end(); }
	auto& cbegin() { return links.cbegin(); }
	auto& cend()   { return links.cend(); }

	// get direct access to the links member
	const std::vector<std::unique_ptr<T>>& getLinks() const { return links; }

protected:
	const TouchObject<TEInstance>		instance		{ nullptr };
	std::vector<std::unique_ptr<T>>		links;
	std::unordered_map<std::string, T*> nameMap;
	std::unordered_map<std::string, T*> identifierMap;
};