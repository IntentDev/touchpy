#pragma once

#include <TouchEngine/TouchEngine.h>
#include <string>



template <typename Derived>
class Link
{
public:
	enum class Scope { Input, Output };

	Link(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
		:	instance_(instance), 
			name_(linkInfo->name),
			identifier_(linkInfo->identifier),
			count_(linkInfo->count),
			scope_(linkInfo->scope == TEScope::TEScopeInput ? Scope::Input : Scope::Output) { }

	Link(const Link&) = delete;
	Link& operator=(const Link&) = delete;
	Link(Link&&) = delete;
	Link& operator=(Link&&) = delete;
	~Link() = default;

	inline TouchObject<TEInstance> instance() const { return instance; }
	inline std::string name() const { return name_; }
	inline std::string identifier() const { return identifier_; }
	inline int32_t count() const { return count_; }
	inline Scope scope() const { return scope_; }

protected:
	const TouchObject<TEInstance> instance_;
	const std::string name_;
	const std::string identifier_;
	const int32_t count_;
	const Scope scope_;
};