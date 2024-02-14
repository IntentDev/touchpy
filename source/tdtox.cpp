#include "tdtox.h"
#include "teutils.h"
#include <iostream>





TdTox::TdTox(std::string filePath)
	: filePath_(filePath)
{
	createVkContext();
}

TdTox::~TdTox()
{
}

void TdTox::createVkContext()
{
	vkContext_ = std::make_unique<VkContext>();
	vkContext_->createInstance();
	vkContext_->init();
}

void TdTox::load()
{
	std::cout << "Loading tox file: " << std::string(filePath_.begin(), filePath_.end()) << std::endl;

	TEResult teresult = TEInstanceCreate(eventCallback, linkEventCallback, this, instance_.take());

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance created!" << std::endl;
		teresult = TEInstanceAssociateGraphicsContext(instance_, vkContext_->teContext());
	}
	else
	{
		std::cout << "Failed to create instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance associated with Graphics Context!" << std::endl;
		teresult = TEInstanceConfigure(instance_, filePath_.c_str(), TETimeInternal);
	}
	else
	{
		std::cout << "Failed to associate instance with Graphics Context" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance configured!" << std::endl;
		teresult = TEInstanceSetFrameRate(instance_, framesPerSecond_, 1);
	}
	else
	{
		std::cout << "Failed to configure instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		teresult = TEInstanceLoad(instance_);
	}
	else
	{
		std::cout << "Failed to set frame rate" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance loaded!" << std::endl;
		teresult = TEInstanceResume(instance_);
	}
	else
	{
		std::cout << "Failed to load instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult != TEResultSuccess)
	{
		std::cout << "Failed to resume instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}



	//assert(teresult == TEResultSuccess);

	//const auto interval = static_cast<unsigned int>(std::ceil(1000. / framesPerSecond_ / 2.));


}


void TdTox::configured(TEResult result)
{
	// Configuration can be cancelled by a subsequent configuration or other action
	// - we can ignore the event in that case and await a following one
	if (result != TEResultCancelled)
	{
		std::lock_guard<std::mutex> guard(mutex_);
		configured_ = true;
		configuredResult_ = result;
	}
}

void TdTox::eventCallback(TEInstance* instance,
	TEEvent event,
	TEResult result,
	int64_t start_time_value,
	int32_t start_time_scale,
	int64_t end_time_value,
	int32_t end_time_scale,
	void* info)
{

	TdTox* tdTox = static_cast<TdTox*>(info);

	switch (event)
	{
	case TEEventInstanceReady:
		std::cout << "Instance ready" << std::endl;
		break;
	case TEEventInstanceDidLoad:
		tdTox->loaded();
		break;
	case TEEventInstanceDidUnload:
		break;
	case TEEventFrameDidFinish:
		tdTox->endFrame(start_time_value, start_time_scale, result);
		break;
	case TEEventGeneral:
		// TODO: check result here
		break;
	default:
		break;
	}
}

void TdTox::linkEventCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
	TdTox* tdTox = static_cast<TdTox*>(info);
	switch (event)
	{
	case TELinkEventAdded:
		tdTox->linkLayoutDidChange(event, identifier);
		break;
	case TELinkEventValueChange:
		tdTox->linkValueChange(identifier);
		break;
	default:
		break;
	}
}

void TdTox::endFrame(int64_t time_value, int32_t time_scale, TEResult result)
{
	setInFrame(false);
}

void TdTox::getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	configured = configured_;
	configured_ = false;
	loaded = loaded_;
	if (loaded_)
	{
		// For this example, we are only interested in links after load has completed
		linksChanged = pendingLayoutChange_;
		inFrame = inFrame_;
		pendingLayoutChange_ = false;
	}
	else
	{
		linksChanged = false;
		inFrame = false;
	}
}

void TdTox::setInFrame(bool inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	inFrame_ = inFrame;
}


void TdTox::linkLayoutDidChange(TELinkEvent event, const char* identifier)
{
	std::lock_guard<std::mutex> guard(mutex_);
	pendingLayoutChange_ = true;

	TouchObject<TELinkInfo> link;
	TEResult result = TEInstanceLinkGetInfo(instance_, identifier, link.take());
	if (result == TEResultSuccess)
	{
		std::cout << "Link layout changed: " << teutils::linkInfoToString(link) << " event: "
			<< teutils::linkEventToString(event) << std::endl;

		if (link->scope == TEScopeOutput)
		{
			//if (strcmp(link->identifier, "op/topOut1") == 0)
			//{
			//	TouchObject <TETexture> tex;
			//	TEResult result = TEInstanceLinkGetTextureValue(
			//		instance_, identifier, TELinkValueCurrent, tex.take());

			//	if (result == TEResultSuccess)
			//	{
			//		TEVulkanTexture* vkTex = static_cast<TEVulkanTexture*>(tex.get());
			//		texFromTE_ = std::make_unique<Texture>(
			//			vkContext_->vContext().device,
			//			vkTex
			//		);

			//		VkExtent2D extent = {
			//			static_cast<uint32_t> (TEVulkanTextureGetWidth(vkTex)),
			//			static_cast<uint32_t> (TEVulkanTextureGetHeight(vkTex))
			//		};

			//		VkFormat format = TEVulkanTextureGetFormat(vkTex);

			//		std::cout << "Texture extent: " << extent.width << " x " << extent.height << std::endl;

			//		texToTE_ = std::make_unique<Texture>(
			//			vkContext_->vContext().physicalDevice,
			//			vkContext_->vContext().device,
			//			vkContext_->vContext().queueFamilyIndices.usedFamilyIndices(),
			//			extent,
			//			format
			//		);
			//	}
			//	else
			//	{
			//		std::cout << TEResultGetDescription(result) << std::endl;
			//	}
			//}
		}

	}
	//std::cout << "Link layout changed: " << identifier << " : " 
	//			<< getLinkEventString(event) << std::endl;
}

void TdTox::update()
{
	TEInstanceStartFrameAtTime(instance_, TETimeInternal, 0, 0);
}


void TdTox::linkValueChange(const char* identifier)
{
	TouchObject<TELinkInfo> link;
	TEResult result = TEInstanceLinkGetInfo(instance_, identifier, link.take());
	if (result == TEResultSuccess && link->scope == TEScopeOutput)
	{
		switch (link->type)
		{
		case TELinkTypeTexture:
		{
			// Stash the state, we don't do any actual renderer work from this thread
			//std::lock_guard<std::mutex> guard(mutex_);
			//myPendingOutputTextures.push_back(identifier);

			if (strcmp(link->identifier, "op/topOut1") == 0)
			{
				TouchObject <TETexture> tex;
				TEResult result = TEInstanceLinkGetTextureValue(
					instance_, identifier, TELinkValueCurrent, tex.take());

				if (result == TEResultSuccess)
				{
					TEVulkanTexture* vkTex = static_cast<TEVulkanTexture*>(tex.get());

					if (texFromTE_.get() == nullptr)
					{
						texFromTE_ = std::make_unique<Texture>(
							vkContext_->vContext().device,
							vkTex
						);
					}

					VkExtent2D extent = {
						static_cast<uint32_t> (TEVulkanTextureGetWidth(vkTex)),
						static_cast<uint32_t> (TEVulkanTextureGetHeight(vkTex))
					};

					VkFormat format = TEVulkanTextureGetFormat(vkTex);

					//std::cout << "Texture extent: " << extent.width << " x " << extent.height 
					//		<< " format: " << string_VkFormat(format) << std::endl;


	/*				if (texToTE_.get() == nullptr)
					{
						texToTE_ = std::make_unique<Texture>(
							vkContext_->vContext().physicalDevice,
							vkContext_->vContext().device,
							vkContext_->vContext().queueFamilyIndices.usedFamilyIndices(),
							extent,
							format
						);
					}*/
				}
				else
				{
					std::cout << TEResultGetDescription(result) << std::endl;
				}
			}


			break;
		}
		case TELinkTypeFloatBuffer:
		{
			TouchObject<TEFloatBuffer> buffer;
			result = TEInstanceLinkGetFloatBufferValue(instance_, identifier, TELinkValueCurrent, buffer.take());

			if (result == TEResultSuccess)
			{
				uint32_t valueCount = TEFloatBufferGetValueCount(buffer);
				int32_t channelCount = TEFloatBufferGetChannelCount(buffer);
				if (buffer && channelCount > 0 && valueCount > 0)
				{
					const float* const* data = TEFloatBufferGetValues(buffer);

					std::cout << "Float buffer values: ";

					for (int channel = 0; channel < channelCount; channel++)
					{
						// Here we just grab the first sample in the channel
						float value = data[channel][0];
						std::cout << value << ", ";
					}

					std::cout << std::endl;
				}
			}
			break;
		}
		case TELinkTypeStringData:
		{
			TouchObject<TEObject> value;
			result = TEInstanceLinkGetObjectValue(instance_, identifier, TELinkValueCurrent, value.take());
			// String data can be a TETable or TEString, so check the type
			if (value && TEGetType(value) == TEObjectTypeTable)
			{
				TouchObject<TETable> table;
				table.set(static_cast<TETable*>(value.get()));
				// do something with the table 
			}
			else if (value && TEGetType(value) == TEObjectTypeString)
			{
				TouchObject<TEString> string;
				string.set(static_cast<TEString*>(value.get()));
				// do something with the string
			}
			break;
		}
		default:
			break;
		}
	}
}


