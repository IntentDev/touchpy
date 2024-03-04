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

// check for the existence of a name() method for the link type
template<typename T, typename = void>
struct has_name_method : std::false_type {};

template<typename T>
struct has_name_method<T, std::void_t<decltype(std::declval<T>().name())>>
	:	std::is_same<decltype(std::declval<T>().name()), std::string> {};

// check for the existence of an identifier() method for the link type
template<typename T, typename = void>
struct has_identifier_method : std::false_type {};

template<typename T>
struct has_identifier_method<T, std::void_t<decltype(std::declval<T>().identifier())>>
	:	std::is_same<decltype(std::declval<T>().identifier()), std::string> {};

//// check for the existence of an updateOuput() method for the link type
//template<typename T, typename = void>
//struct has_onOuputValueChange_method : std::false_type {};
//
//template<typename T>
//struct has_onOuputValueChange_method<T, std::void_t<decltype(std::declval<T>().onOuputValueChange())>> 
//	:	std::is_same<decltype(std::declval<T>().onOuputValueChange()), void> {};

template <typename Derived, typename T>
class Links 
{
	static_assert(has_name_method<T>::value, "Type T must have a member function std::string getName()");
	static_assert(has_identifier_method<T>::value, "Type T must have a member function getIdentifier()");
	//static_assert(has_onOuputValueChange_method<T>::value, "Type T must have a member function onOuputValueChange()");

public:

	Links() = default;
	Links(const Links&) = delete;
	Links& operator=(const Links&) = delete;
	Links(Links&&) = delete;
	Links& operator=(Links&&) = delete;
	~Links() = default;


	Links(TouchObject<TEInstance> instance) : instance_(instance) { }

	virtual void addLink(TouchObject<TELinkInfo> linkInfo)
	{
		links_.push_back(std::make_unique<T>(instance_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

	void removeLinkByName(const std::string& name)
	{
		auto it = nameMap_.find(name);
		if (it != nameMap_.end())
		{
			auto link = it->second;
			nameMap_.erase(it);
			identifierMap_.erase(link->identifier());
			links_.erase(std::remove_if(links_.begin(), links_.end(), [link](const std::unique_ptr<T>& p) { return p.get() == link; }), links_.end());
		}
	}

	void removeLinkByIdentifier(const std::string& identifier)
	{
		auto it = identifierMap_.find(identifier);
		if (it != identifierMap_.end())
		{
			auto link = it->second;
			identifierMap_.erase(it);
			nameMap_.erase(link->name());
			links_.erase(std::remove_if(links_.begin(), links_.end(), [link](const std::unique_ptr<T>& p) { return p.get() == link; }), links_.end());
		}
	}

	T* getLinkByName(const std::string& name)
	{
		auto it = nameMap_.find(name);
		if (it != nameMap_.end()) return it->second;
		return nullptr;
	}

	T* getLinkByIdentifier(const std::string& identifier)
	{
		auto it = identifierMap_.find(identifier);
		if (it != identifierMap_.end()) return it->second;
		return nullptr;
	}

	T* getLinkByIndex(size_t index)
	{
		if (index < links_.size()) return links_[index].get();
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

	size_t size() const { return links_.size(); }

	T& operator[](const std::string& name) 
	{ 
		auto link = getLinkByName(name);
		if (!link) throw std::runtime_error("Link not found");
		return *link; 
	}

	void reset()
	{
		links_.clear();
	}

	T& operator[](size_t index)
	{
		auto link = getLinkByIndex(index);
		if (!link) throw std::runtime_error("Link not found");
		return *link; 
	}

	// iterator support
	auto& begin()  { return links_.begin(); }
	auto& end()    { return links_.end(); }
	auto& cbegin() { return links_.cbegin(); }
	auto& cend()   { return links_.cend(); }

	// get direct access to the links member
	const std::vector<std::unique_ptr<T>>& getLinks() const { return links_; }

protected:
	const TouchObject<TEInstance>		instance_		{ nullptr };
	std::vector<std::unique_ptr<T>>		links_;
	std::unordered_map<std::string, T*> nameMap_;
	std::unordered_map<std::string, T*> identifierMap_;
};