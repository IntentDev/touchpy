#pragma once

#include "common/helpers.h"

#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TEVulkan.h>
#include <string>
#include <iostream>

#ifdef NDEBUG
#define TE_CHECK(x) x
#else
#define TE_CHECK(x) \
do \
{ \
	TEResult result = x; \
	if (result != TEResultSuccess) \
	{ \
		std::cerr << "TE Error: "  <<  __FILE__ << ": " << __LINE__ << ": " \
			<< TEResultGetDescription(result) << std::endl; \
		abort(); \
	} \
} while (0)
#endif




NAMESPACE_BEGIN(teutils)

std::string eventToString(TEEvent event)
{
	switch (event) {
	case TEEventGeneral:
		return "TEEventGeneral";
	case TEEventInstanceReady:
		return "TEEventInstanceReady";
	case TEEventInstanceDidLoad:
		return "TEEventInstanceDidLoad";
	case TEEventInstanceDidUnload:
		return "TEEventInstanceDidUnload";
	case TEEventFrameDidFinish:
		return "TEEventFrameDidFinish";
	default:
		return "Unknown TEEvent";
	}
}

std::string linkEventToString(TELinkEvent event)
{
	switch (event)
	{
	case TELinkEventAdded:
		return "TELinkEventAdded";
	case TELinkEventRemoved:
		return "TELinkEventRemoved";
	case TELinkEventModified:
		return "TELinkEventModified";
	case TELinkEventMoved:
		return "TELinkEventMoved";
	case TELinkEventStateChange:
		return "TELinkEventStateChange";
	case TELinkEventChildChange:
		return "TELinkEventChildChange";
	case TELinkEventValueChange:
		return "TELinkEventValueChange";
	default:
		return "Unknown";
	}
}

std::string scopeToString(TEScope scope)
{
	switch (scope)
	{
	case TEScopeInput:
		return "TEScopeInput";
	case TEScopeOutput:
		return "TEScopeOutput";
	default:
		return "Unknown";
	}
}

std::string linkTypeToString(TELinkType linkType)
{
	switch (linkType) {
	case TELinkTypeGroup:
		return "TELinkTypeGroup";
	case TELinkTypeComplex:
		return "TELinkTypeComplex";
	case TELinkTypeSequence:
		return "TELinkTypeSequence";
	case TELinkTypeBoolean:
		return "TELinkTypeBoolean";
	case TELinkTypeDouble:
		return "TELinkTypeDouble";
	case TELinkTypeInt:
		return "TELinkTypeInt";
	case TELinkTypeString:
		return "TELinkTypeString";
	case TELinkTypeTexture:
		return "TELinkTypeTexture";
	case TELinkTypeFloatBuffer:
		return "TELinkTypeFloatBuffer";
	case TELinkTypeStringData:
		return "TELinkTypeStringData";
	case TELinkTypeSeparator:
		return "TELinkTypeSeparator";
	default:
		return "Unknown TELinkType";
	}
}

std::string linkIntentToString(TELinkIntent linkIntent)
{
	switch (linkIntent) {
	case TELinkIntentNotSpecified:
		return "TELinkIntentNotSpecified";
	case TELinkIntentColorRGBA:
		return "TELinkIntentColorRGBA";
	case TELinkIntentPositionXYZW:
		return "TELinkIntentPositionXYZW";
	case TELinkIntentSizeWH:
		return "TELinkIntentSizeWH";
	case TELinkIntentUVW:
		return "TELinkIntentUVW";
	case TELinkIntentFilePath:
		return "TELinkIntentFilePath";
	case TELinkIntentDirectoryPath:
		return "TELinkIntentDirectoryPath";
	case TELinkIntentMomentary:
		return "TELinkIntentMomentary";
	case TELinkIntentPulse:
		return "TELinkIntentPulse";
	default:
		return "Unknown TELinkIntent";
	}
}

std::string linkValueToString(TELinkValue linkValue)
{
	switch (linkValue) {
	case TELinkValueMinimum:
		return "TELinkValueMinimum";
	case TELinkValueMaximum:
		return "TELinkValueMaximum";
	case TELinkValueUIMinimum:
		return "TELinkValueUIMinimum";
	case TELinkValueUIMaximum:
		return "TELinkValueUIMaximum";
	case TELinkValueDefault:
		return "TELinkValueDefault";
	case TELinkValueCurrent:
		return "TELinkValueCurrent";
	default:
		return "Unknown TELinkValue";
	}
}

std::string linkDomainToString(TELinkDomain domain)
{
	switch (domain)
	{
	case TELinkDomainNone:
		return "TELinkDomainNone";
	case TELinkDomainParameter:
		return "TELinkDomainParameter";
	case TELinkDomainParameterPage:
		return "TELinkDomainParameterPage";
	case TELinkDomainOperator:
		return "TELinkDomainOperator";
	default:
		return "Unknown";
	}
}

std::string linkInfoToString(TELinkInfo* info)
{
	std::string result = "Link Info: ";
	result += " scope: " + scopeToString(info->scope);
	result += " intent: " + linkIntentToString(info->intent);
	result += " type: " + linkTypeToString(info->type);
	result += " domain: " + linkDomainToString(info->domain);
	result += " count: " + std::to_string(info->count);
	result += " label: " + std::string(info->label);
	result += " name: " + std::string(info->name);
	result += " identifier: " + std::string(info->identifier);
	return result;
	
}

void printLinkInfo(TELinkInfo* info)
{
	std::cout << "Link Info: " 
		<< " scope: " << scopeToString(info->scope)
		<< " intent: " << linkIntentToString(info->intent)
		<< " type: " << linkTypeToString(info->type)
		<< " domain: " << linkDomainToString(info->domain)
		<< " count: " << info->count
		<< " label: " << info->label
		<< " name: " << info->name
		<< " identifier: " << info->identifier << std::endl;
}

std::string
getLinkInfoAsString(TouchObject<TELinkInfo> info)
{
	std::stringstream ss;
	ss << std::left
		<< std::setw(6) << "Link:" << std::setw(16) << info->identifier
		<< std::setw(6) << "name:" << std::setw(16) << info->name
		<< std::setw(7) << "label:" << std::setw(16) << info->label
		<< std::setw(7) << "scope:" << std::setw(16) << teutils::scopeToString(info->scope)
		<< std::setw(8) << "intent:" << std::setw(28) << teutils::linkIntentToString(info->intent)
		<< std::setw(8) << "domain:" << std::setw(24) << teutils::linkDomainToString(info->domain)
		<< std::setw(7) << "count:" << std::setw(5) << info->count
		<< std::setw(6) << "type:" << std::setw(16) << teutils::linkTypeToString(info->type)
		;

	return ss.str();
}

NAMESPACE_END(teutils)