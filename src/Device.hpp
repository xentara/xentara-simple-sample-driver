// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/data/String.hpp>
#include <xentara/io/Component.hpp>
#include <xentara/io/ComponentClass.hpp>
#include <xentara/memory/Array.hpp>

#include <string_view>
#include <filesystem>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

// This class represents an I/O device. This sample class just stores a directory path, from which files are read and written.
class Device final : public io::Component
{
private:
	// This structure is used to publish the configuration parameters as Xentara attributes
	struct Config final
	{
		// The absolute path to the directory
		data::U8String _directoryPath;
	};
	
public:
	// This is the class object for the input. The class contains meta-information about this
	// type of element.
	class Class : public io::ComponentClass
	{
	public:
		// Gets the global object
		static auto instance() -> Class&
		{
			return _instance;
		}

	    // Returns a handle to the class specific configuration
	    auto configHandle() const -> const auto &
        {
            return _configHandle;
        }

		auto name() const -> std::u16string_view final
		{
			// This is the name of the element class, as it appears in the model.json file
			return u"Device"sv;
		}
	
		auto uuid() const -> utils::core::Uuid final
		{
			// This is an arbitrary unique UUID for the driver. This can be anything, but should never change.
			return "763544ff-9b86-491c-90a5-b2b343bab115"_uuid;
		}

	private:
        // A handle that allows us to find our custom configuration parameters in the element configuration
		// data block. The initializer registers the handle with the classes configuration block.
		memory::Array::ObjectHandle<Config> _configHandle { config().appendObject<Config>() };

		// The global object that represents the class
		static Class _instance;
	};

	// Returns the absolute path of the directory
	auto directoryPath() const -> const auto &
	{
		return _directoryPath;
	}

	// Creates a read handle for the "directory" attribute
	auto directoryAttributeReadHandle() const noexcept -> data::ReadHandle;

	auto createIo(const io::IoClass &ioClass, plugin::SharedFactory<io::Io> &factory) -> std::shared_ptr<io::Io> final;

	auto resolveAttribute(std::u16string_view name) -> const model::Attribute * final;

	auto readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle final;

protected:
	auto loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void final;

private:
	// The absolute path to the directory
	std::filesystem::path _directoryPath;
};

} // namespace xentara::samples::simpleDriver