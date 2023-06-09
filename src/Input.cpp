// Copyright (c) embedded ocean GmbH
#include "Input.hpp"

#include "Attributes.hpp"
#include "Device.hpp"
#include "Tasks.hpp"

#include <xentara/config/FallbackHandler.hpp>
#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/WriteSentinel.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/model/ForEachAttributeFunction.hpp>
#include <xentara/model/ForEachEventFunction.hpp>
#include <xentara/model/ForEachTaskFunction.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/io/FileInputStream.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>

#include <filesystem>
#include <charconv>

namespace xentara::samples::simpleDriver
{
	
using namespace std::literals;

Input::Class Input::Class::_instance;

using namespace std::literals;

const model::Attribute Input::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadOnly, data::DataType::kFloatingPoint };

auto Input::loadConfig(const ConfigIntializer &initializer,
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
				utils::json::decoder::throwWithLocation(value, std::runtime_error("empty file name for simple sample driver input"));
			}
			// Check that it is just a file name
			if (fileName.has_parent_path())
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("file name of simple sample driver input includes directory components"));
			}
			// Check that it is not a . or a ..
			if (fileName == "." || fileName == "..")
			{
				utils::json::decoder::throwWithLocation(value, std::runtime_error("invalid file name for simple sample driver input"));
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

auto Input::readFile() -> double
{
	// We use this as a generic error message
	static constexpr char const kErrorMessage[] { "failed to parse file data" };

	// Open the file. We use the file class from xentara-utils instead of std::ifstream, because std::ifstream
	// has no proper error reporting, but reports all errors as the single error code std::io_errc::stream.
	utils::io::File file(_filePath, utils::io::File::Access::Read);

	// Read a single word
	auto fileData = file.readAll();
	// Close the file before proceeding, just in case
	file.close();

	// Interpret the file contents as ASCII characters.
	const char *dataBegin = reinterpret_cast<const char *>(fileData.data());
	const char *dataEnd = dataBegin + fileData.size();

	// Decode the value
	double value;
	auto [parsedDataEnd, error] = std::from_chars(dataBegin, dataEnd, value);

	// Handle out-of-range errors
	if (error == std::errc::result_out_of_range)
	{
		throw std::system_error(CustomError::ValueOutOfRange, kErrorMessage);
	}
	// All other errors are considered bad file data. We also check that all the data has been parsed.
	if (error != std::errc() || parsedDataEnd != dataEnd)
	{
		throw std::system_error(CustomError::CorruptFileContent, kErrorMessage);
	}

	return value;
}

auto Input::perfromReadTask(const process::ExecutionContext &context) -> void
{
	try
	{
		// Read the value
		auto value = readFile();
		// Report the result
		reportReadResult(context, value);
	}
	catch (const std::exception &)
	{
		// xentara-utils has a utility function that extracts the std::error_code from the current exception, if it is an std::system_error
		reportReadResult(context, 0, utils::eh::currentErrorCode());
	}
}

auto Input::reportReadResult(const process::ExecutionContext &context, double value, std::error_code error) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _stateDataBlock };

	// Update the state
	auto &state = *sentinel;
	state._updateTime = context.scheduledTime();
	state._value = value;
	state._quality = error ? data::Quality::Bad : data::Quality::Good;
	state._error = errorAttributeValue(error);

	// Detect changes
	const auto &oldState = sentinel.oldValue();
	const auto valueChanged = state._value != oldState._value;
	const auto qualityChanged = state._quality != oldState._quality;
	const auto errorChanged = state._error != oldState._error;
	const bool changed = valueChanged | qualityChanged | errorChanged;

	// Update the change time, if necessary. We always need to write the change time, even if it is the same as before,
	// because the memory resource might use swap-in.
	state._changeTime = changed ? context.scheduledTime() : oldState._changeTime;

	// Commit the data before sending the events
	sentinel.commit();

	// Fire the correct events
	if (valueChanged)
	{
		_valueChangedEvent.fire();
	}
	if (qualityChanged)
	{
		_qualityChangedEvent.fire();
	}
	if (changed)
	{
		_changedEvent.fire();
	}
}

auto Input::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

auto Input::directions() const -> io::Directions
{
	return io::Direction::Input;
}

auto Input::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// Handle all the attributes we support
	return
		function(model::Attribute::kUpdateTime) ||
		function(kValueAttribute) ||
		function(model::Attribute::kChangeTime) ||
		function(model::Attribute::kQuality) ||
		function(attributes::kError) ||
		function(attributes::kFileName) ||
		function(attributes::kDirectory);
}

auto Input::forEachEvent(const model::ForEachEventFunction &function) -> bool
{
	// Handle all the events we support
	return
		function(model::Attribute::kValue, sharedFromThis(&_valueChangedEvent)) ||
		function(model::Attribute::kQuality, sharedFromThis(&_qualityChangedEvent)) ||
		function(process::Event::kChanged, sharedFromThis(&_changedEvent));
}

auto Input::forEachTask(const model::ForEachTaskFunction &function) -> bool
{
	// Handle all the tasks we support
	return
		function(tasks::kRead, sharedFromThis(&_readTask));
}

auto Input::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try each readable attribute
	if (attribute == model::Attribute::kUpdateTime)
	{
		return _stateDataBlock.member(&State::_updateTime);
	}
	else if (attribute == kValueAttribute)
	{
		return _stateDataBlock.member(&State::_value);
	}
	else if (attribute == model::Attribute::kChangeTime)
	{
		return _stateDataBlock.member(&State::_changeTime);
	}
	else if (attribute == model::Attribute::kQuality)
	{
		return _stateDataBlock.member(&State::_quality);
	}
	else if (attribute == attributes::kError)
	{
		return _stateDataBlock.member(&State::_error);
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

auto Input::realize() -> void
{
	// Create the data block
	_stateDataBlock.create(memory::memoryResources::data());
}

auto Input::ReadTask::preparePreOperational(const process::ExecutionContext &context) -> Status
{
	// Read the file once to initialize the value
	_input.get().perfromReadTask(context);

	// We are done now. Even if we couldn't read the value, we proceedd to the next stage,
	// because attempting again is unlikely to succeed any better.
	return Status::Ready;
}

auto Input::ReadTask::preOperational(const process::ExecutionContext &context) -> Status
{
	// We just do the same thing as in the operational stage
	operational(context);

	return Status::Ready;
}

auto Input::ReadTask::operational(const process::ExecutionContext &context) -> void
{
	// read the value
	_input.get().perfromReadTask(context);
}

} // namespace xentara::samples::simpleDriver