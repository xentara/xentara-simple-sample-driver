// Copyright (c) embedded ocean GmbH
#include "Device.hpp"

#include "Attributes.hpp"
#include "Input.hpp"
#include "Output.hpp"

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/plugin/SharedFactory.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/windows/Errors.hpp>

#include <iostream>
#include <filesystem>
#include <string_view>
#include <cstdlib>
#include <memory>
#include <cstdlib>

#ifdef _WIN32
#	include <Shlobj.h>
#endif

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

Device::Class Device::Class::_instance;

namespace
{

// Helper function to get the user's home directory
auto homeDirectory() -> std::filesystem::path
{
	// Get the home directory of the user
	#ifdef _WIN32

	// Get the known folder path
	PWSTR pathPointer {nullptr};
	auto result = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DONT_VERIFY, nullptr, &pathPointer);
	// Automatically delete it
	std::unique_ptr<WCHAR[], decltype(&CoTaskMemFree)> path { pathPointer, &CoTaskMemFree };

	// Handle errors
	if (result != S_OK)
	{
		throw std::system_error(int(result), utils::windows::hresultErrorCategory(), "could not get path of current user's document folder");
	}

	return path.get();

	#else // _WIN32

	// Get the HOME variable
	const auto homeVariable = std::getenv("HOME");
	if (!homeVariable)
	{
		throw std::runtime_error("could not get path of current user's document folder: HOME environment variable not set");
	}

	return homeVariable;

	#endif // _WIN32
}

}

auto Device::loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void
{
	// We can use the config handle we stored in the class object to get a pointer to our custom configuration
    auto &&config = initializer[Class::instance().configHandle()];

	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		// Handle the "fileName" member
		if (name == u8"directory"sv)
		{
			// Read the file name as a string
			const std::filesystem::path subPath = value.asString<std::u8string>();

			// Check that it is not empty
			if (subPath.empty())
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("empty directory for simple sample driver device"));
			}
			// Check that it is a relative path
			if (!subPath.is_relative())
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("directory path of simple sample driver device is not a relative path"));
			}

			// Make the absolute path
			auto directoryPath = (homeDirectory() / subPath).make_preferred();
			
			// Initialize the configuration attributes
			config._directoryPath = directoryPath.u8string();
			// Move the path into the member variable
			_directoryPath = std::move(directoryPath);
		}
		else
		{
			// Pass any unknown parameters on to the fallback handler, which will load the built-in parameters ("id", "uuid", and "children"),
			// and throw an exception if the key is unknown
            fallbackHandler(name, value);
		}
    }

	// Check that the user supplied a directory path
	if (_directoryPath.empty())
	{
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("missing directory for simple sample driver device"));
	}
}

auto Device::createIo(const io::IoClass &ioClass, plugin::SharedFactory<io::Io> &factory) -> std::shared_ptr<io::Io>
{
	if (&ioClass == &Input::Class::instance())
	{
		return factory.makeShared<Input>(*this);
	}
	else if (&ioClass == &Output::Class::instance())
	{
		return factory.makeShared<Output>(*this);
	}

	return nullptr;
}

auto Device::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// We only have the directory attribute
	return model::Attribute::resolve(name, model::Attribute::kDeviceState, attributes::kDirectory);
}

auto Device::directoryAttributeReadHandle() const noexcept -> data::ReadHandle
{
	return configBlock().member(Class::instance().configHandle(), &Config::_directoryPath);
}

auto Device::readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle
{
	// Try our attributes
	if (attribute == model::Attribute::kDeviceState)
	{
		return _stateDataBlock.handle();
	}
	else if (attribute == attributes::kDirectory)
	{
		return directoryAttributeReadHandle();
	}

	// Nothing found
	return data::ReadHandle::Error::Unknown;
}

auto Device::prepare() -> void
{
	// Create the data block
	auto initializer = _stateDataBlock.create(memory::memoryResources::data());

	// Immediately set the state to true. We don't really keep track of a device state,
	// but the "deviceState" attribute is required for I/O components to preserve a consistent interface.
	*initializer = true;
}

} // namespace xentara::samples::simpleDriver