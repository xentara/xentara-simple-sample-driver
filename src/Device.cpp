// Copyright (c) embedded ocean GmbH
#include "Device.hpp"

#include "Attributes.hpp"
#include "Input.hpp"
#include "Output.hpp"

#include <xentara/config/Errors.hpp>
#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/model/ForEachAttributeFunction.hpp>
#include <xentara/skill/ElementFactory.hpp>
#include <xentara/utils/filesystem/toString.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
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

auto Device::load(utils::json::decoder::Object &jsonObject, config::Context &context) -> void
{
	// The directory path is required, so we need to keep track of whether it was loaded or not
	bool directoryPathLoaded = false;

	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		// Handle the "directory" member
		if (name == "directory"sv)
		{
			// Set the flag denoting that the directory entry was present
			directoryPathLoaded = true;

			// Read the file name as a string
			const std::filesystem::path subPath = value.asString<std::string>();

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
			
			// Initialize the configuration attribute. This must be done before the assignment below, because that uses std::move().
			_directoryPathAsString = utils::filesystem::toString(directoryPath);
			// Move the path into the member variable
			_directoryPath = std::move(directoryPath);
		}
		else
		{
            config::throwUnknownParameterError(name);
		}
    }

	// Check that the user supplied a directory path
	if (!directoryPathLoaded)
	{
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("missing directory for simple sample driver device"));
	}
}

auto Device::createChildElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
	-> std::shared_ptr<skill::Element>
{
	if (&elementClass == &Input::Class::instance())
	{
		return factory.makeShared<Input>(*this);
	}
	else if (&elementClass == &Output::Class::instance())
	{
		return factory.makeShared<Output>(*this);
	}

	return nullptr;
}

auto Device::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// We only have the directory attribute
	return function(attributes::kDirectory);
}

auto Device::makeDirectoryAttributeReadHandle() const noexcept -> std::optional<data::ReadHandle>
{
	return sharedFromThis(&_directoryPathAsString);
}

auto Device::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try our attributes
	if (attribute == attributes::kDirectory)
	{
		return makeDirectoryAttributeReadHandle();
	}

	// Nothing found
	return std::nullopt;
}

} // namespace xentara::samples::simpleDriver