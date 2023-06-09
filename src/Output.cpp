// Copyright (c) embedded ocean GmbH
#include "Output.hpp"

#include "Attributes.hpp"
#include "CustomError.hpp"
#include "Device.hpp"
#include "Events.hpp"
#include "Tasks.hpp"

#include <xentara/config/FallbackHandler.hpp>
#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/WriteSentinel.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/model/ForEachAttributeFunction.hpp>
#include <xentara/model/ForEachEventFunction.hpp>
#include <xentara/model/ForEachTaskFunction.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>
#include <xentara/utils/io/File.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/numeric/toString.hpp>

#include <filesystem>

namespace xentara::samples::simpleDriver
{
	
using namespace std::literals;

Output::Class Output::Class::_instance;

using namespace std::literals;

const model::Attribute Output::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::WriteOnly, data::DataType::kFloatingPoint };

auto Output::loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const config::FallbackHandler &fallbackHandler) -> void
{
	// We can use the config handle we stored in the class object to get a pointer to our custom configuration
    auto &&config = initializer[Class::instance().configHandle()];

	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		// Handle the "fileName" member
		if (name == "fileName"sv)
		{
			// Read the file name as a string
			std::filesystem::path fileName = value.asString<std::string>();

			// Check that it is not empty
			if (fileName.empty())
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("empty file name for simple sample driver output"));
			}
			// Check that it is just a file name
			if (fileName.has_parent_path())
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("file name of simple sample driver output includes directory components"));
			}
			// Check that it is not a . or a ..
			if (fileName == "." || fileName == "..")
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("invalid file name for simple sample driver output"));
			}

			// Initialize the configuration attributes
			config._fileName = fileName.make_preferred().string();

			// Make the path by combining the directory path and the file name
			_filePath = _device.get().directoryPath() / fileName;
		}
		else
		{
			// Pass any unknown parameters on to the fallback handler, which will load the built-in parameters ("id" and "uuid"),
			// and throw an exception if the key is unknown
            fallbackHandler(name, value);
		}
    }

	// Check that the user supplied a file name
	if (_filePath.empty())
	{
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("missing file name for simple sample driver input"));
	}
}

auto Output::writeFile(double value) -> void
{
	// Open the file. We use the file class from xentara-utils instead of std::ofstream, because std::ofstream
	// has no proper error reporting, but reports all errors as the single error code std::io_errc::stream.
	utils::io::File file(_filePath, utils::io::File::Access::Truncate);

	// Format the number
	auto numericString = utils::numeric::toString(value);
	// Write it
	file.write(std::as_bytes(std::span { numericString.data(), numericString.size() }));
}

auto Output::perfromWriteTask(const process::ExecutionContext &context) -> void
{
	// Get the value
	auto pendingValue = _pendingValue.exchange(std::nullopt, std::memory_order_acq_rel);
	// If there was no pending value, just bail
	if (!pendingValue)
	{
		return;
	}

	try
	{
		// Write the value
		writeFile(*pendingValue);
		// Report the result
		reportWriteResult(context);
	}
	catch (const std::exception &)
	{
		// xentara-utils has a utility function that extracts the std::error_code from the current exception, if it is an std::system_error
		reportWriteResult(context, utils::eh::currentErrorCode());
	}
}

auto Output::reportWriteResult(const process::ExecutionContext &context, std::error_code error) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _stateDataBlock };

	// Update the state
	auto &state = *sentinel;
	state._writeTime = context.scheduledTime();
	state._writeError = errorAttributeValue(error);

	// Commit the data before sending the even
	sentinel.commit();

	// Fire the correct event
	if (!error)
	{
		_writtenEvent.fire();
	}
	else
	{
		_writeErrorEvent.fire();
	}
}

auto Output::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

auto Output::directions() const -> io::Directions
{
	return io::Direction::Output;
}

auto Output::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// Handle all the attributes we support
	return
		function(kValueAttribute) ||
		function(model::Attribute::kWriteTime) ||
		function(attributes::kWriteError) ||
		function(attributes::kFileName) ||
		function(attributes::kDirectory);
}

auto Output::forEachEvent(const model::ForEachEventFunction &function) -> bool
{
	// Handle all the events we support
	return
		function(events::kWritten, sharedFromThis(&_writtenEvent)) ||
		function(events::kWriteError, sharedFromThis(&_writeErrorEvent));
}

auto Output::forEachTask(const model::ForEachTaskFunction &function) -> bool
{
	// We only have a "write" task
	return function(tasks::kWrite, sharedFromThis(&_writeTask));
}

auto Output::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try each readable attribute
	if (attribute == model::Attribute::kWriteTime)
	{
		return _stateDataBlock.member(&State::_writeTime);
	}
	else if (attribute == attributes::kWriteError)
	{
		return _stateDataBlock.member(&State::_writeError);
	}
	else if (attribute == attributes::kFileName)
	{
		// Make a pointer to the correct member of the value stored in the config block
		return configBlock().member(Class::instance().configHandle(), &Config::_fileName);
	}
	else if (attribute == attributes::kDirectory)
	{
		// We get this from the device. This results in the same handle for all I/Os.
		return _device.get().makeDirectoryAttributeReadHandle();
	}

	return std::nullopt;
}

auto Output::makeWriteHandle(const model::Attribute &attribute) noexcept -> std::optional<data::WriteHandle>
{
	// There is only one writable attribute
	if (attribute == kValueAttribute)
	{
		// This magic code creates a write handle of type double that calls scheduleWrite() on this.
		return data::WriteHandle { std::in_place_type<double>, &Output::scheduleWrite, sharedFromThis() };
	}

	return std::nullopt;
}

auto Output::realize() -> void
{
	// Create the data block
	_stateDataBlock.create(memory::memoryResources::data());
}

auto Output::WriteTask::preparePreOperational(const process::ExecutionContext &context) -> Status
{
	// We don't actually need to do anything, so just inform the scheduler that it may proceed to the next stage
	// as far as we're concened.
	return Status::Ready;
}

auto Output::WriteTask::preOperational(const process::ExecutionContext &context) -> Status
{
	// We just do the same thing as in the operational stage
	operational(context);

	return Status::Ready;
}

auto Output::WriteTask::operational(const process::ExecutionContext &context) -> void
{
	// read the value
	_output.get().perfromWriteTask(context);
}

} // namespace xentara::samples::simpleDriver