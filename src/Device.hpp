// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/memory/Array.hpp>
#include <xentara/skill/Element.hpp>
#include <xentara/skill/EnableSharedFromThis.hpp>
#include <xentara/utils/core/Uuid.hpp>

#include <string_view>
#include <filesystem>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

// This class represents an I/O device. This sample class just stores a directory path, from which files are read and written.
class Device final : public skill::Element, public skill::EnableSharedFromThis<Device>
{
public:
	// This is the class object for the input. The class contains meta-information about this
	// type of element.
	using Class = ConcreteClass<
		// This is the name of the element class, as it appears in the model.json file
		"Device",
		// This is an arbitrary unique UUID for the element class. This can be anything, but should never change.
		"763544ff-9b86-491c-90a5-b2b343bab115"_uuid,
		// This is a human readable name for the element class.
		// NOTE: The display name must be understandable event without knowing the skill it belongs to.
		"simple sample device">;

	// Returns the absolute path of the directory
	auto directoryPath() const -> const auto &
	{
		return _directoryPath;
	}

	// Creates a read handle for the "directory" attribute
	auto makeDirectoryAttributeReadHandle() const noexcept -> std::optional<data::ReadHandle>;

	auto createChildElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
		-> std::shared_ptr<skill::Element> final;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

protected:
	auto load(utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const config::FallbackHandler &fallbackHandler) -> void final;

private:
	// The absolute path to the directory
	std::filesystem::path _directoryPath;

	// The path as a string, for publishing it as an attribute
	std::string _directoryPathAsString;
};

} // namespace xentara::samples::simpleDriver