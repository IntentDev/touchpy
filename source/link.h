#pragma once

#include <TouchEngine/TouchEngine.h>
#include <string>

enum class LinkScope { Input, Output };

template <typename Derived>
class Link
{
public:
	Link(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, LinkScope linkScope) 
		:	instance(instance), 
			name(linkInfo->name),
			identifier(linkInfo->identifier),
			linkScope(linkScope) { }

	Link(const Link&) = delete;
	Link& operator=(const Link&) = delete;
	Link(Link&&) = delete;
	Link& operator=(Link&&) = delete;
	~Link() = default;

	std::string getName() const { return std::string(name); }
	std::string getIdentifier() const { return std::string(identifier); }

protected:
	const TouchObject<TEInstance> instance;
	const std::string name;
	const std::string identifier;

	const LinkScope linkScope;
};