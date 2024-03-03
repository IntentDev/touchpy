#pragma once

#include <TouchEngine/TouchEngine.h>
#include <string>



template <typename Derived>
class Link
{
public:
	enum class Scope { Input, Output };

	Link(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
		:	instance(instance), 
			name(linkInfo->name),
			identifier(linkInfo->identifier),
			count(linkInfo->count),
			scope(linkInfo->scope == TEScope::TEScopeInput ? Scope::Input : Scope::Output) { }

	Link(const Link&) = delete;
	Link& operator=(const Link&) = delete;
	Link(Link&&) = delete;
	Link& operator=(Link&&) = delete;
	~Link() = default;

	std::string getName() const { return name; }
	std::string getIdentifier() const { return identifier; }

protected:
	const TouchObject<TEInstance> instance;
	const std::string name;
	const std::string identifier;
	int32_t count { 0 };
	const Scope scope { Scope::Input };
};