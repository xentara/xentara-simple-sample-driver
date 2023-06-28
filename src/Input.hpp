// Copyright (c) embedded ocean GmbH
#pragma once

#include "CustomError.hpp"

#include <xentara/data/Quality.hpp>
#include <xentara/memory/Array.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/process/Task.hpp>
#include <xentara/skill/DataPoint.hpp>
#include <xentara/skill/EnableSharedFromThis.hpp>

#include <functional>
#include <string_view>
#include <filesystem>
#include <system_error>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

class Device;

// This class represents a single input. This sample class reads a floating point value from a text file.
// The value of the input is read using a Xentara task "read" that must be attached to a Xentara timer or event.
class Input final : public skill::DataPoint, public skill::EnableSharedFromThis<Input>
{
private:
	// This structure is used to publish the configuration parameters as Xentara attributes
	struct Config final
	{
		// The file name
		std::string _fileName;
	};
	
public:
	// This is the class object for the input. The class contains meta-information about this
	// type of element.
	class Class final : public skill::Element::Class
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

		auto name() const -> std::string_view final
		{
			// This is the name of the element class, as it appears in the model.json file
			return "Input"sv;
		}
	
		auto uuid() const -> utils::core::Uuid final
		{
			// This is an arbitrary unique UUID for the element class. This can be anything, but should never change.
			return "2911026c-52c6-4c76-8da7-69e185b6f9f1"_uuid;
		}

	private:
        // A handle that allows us to find our custom configuration parameters in the element configuration
		// data block. The initializer registers the handle with the classes configuration block.
		memory::Array::ObjectHandle<Config> _configHandle { config().appendObject<Config>() };

		// The global object that represents the class
		static Class _instance;
	};

	// This constructor attaches the input to its device
	Input(std::reference_wrapper<Device> device) :
		_device(device)
	{
	}
	
	auto dataType() const -> const data::DataType &;

	auto directions() const -> io::Directions;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;

	auto forEachEvent(const model::ForEachEventFunction &function) -> bool final;
	
	auto forEachTask(const model::ForEachTaskFunction &function) -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto realize() -> void final;

	// A Xentara attribute containing the current value. This is a membor of this class rather than
	// of the attributes namespace, because the access flags differ from class to class
	static const model::Attribute kValueAttribute;

protected:
	auto loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const config::FallbackHandler &fallbackHandler) -> void final;

private:
	// This structure represents the current state of the input
	struct State
	{
		// The update time stamp
		std::chrono::system_clock::time_point _updateTime { std::chrono::system_clock::time_point::min() };
		// The current value
		double _value {};
		// The change time stamp
		std::chrono::system_clock::time_point _changeTime { std::chrono::system_clock::time_point::min() };
		// The quality of the value
		data::Quality _quality { data::Quality::Bad };
		// The error code when reading the value, or a default constructed std::error_code object for none.
		ErrorAttributeValue _error { errorAttributeValue(CustomError::NoData) };
	};

	// This class providing callbacks for the Xentara scheduler for the "read" task
	class ReadTask : public process::Task
	{
	public:
		// This constuctor attached the task to its parent input
		ReadTask(std::reference_wrapper<Input> input) : _input(input)
		{
		}

		auto stages() const -> Stages final
		{
			// Read the value in these stages
			return Stage::PreOperational | Stage::Operational;
		}

		auto preparePreOperational(const process::ExecutionContext &context) -> Status final;

		auto preOperational(const process::ExecutionContext &context) -> Status final;

		auto operational(const process::ExecutionContext &context) -> void final;

	private:
		// A reference to the parent input
		std::reference_wrapper<Input> _input;
	};

	// This function reads the value from the file
	auto readFile() -> double;

	// This function is called by the "read" task. It reads the file, and reports the result.
	auto perfromReadTask(const process::ExecutionContext &context) -> void;
	// Updates the state and sends events depending on the read result
	auto reportReadResult(const process::ExecutionContext &context, double value, std::error_code error = std::error_code()) -> void;

	// The device this input belongs to
	std::reference_wrapper<Device> _device;

	// A summary event that is raised when anything changes
	process::Event _changedEvent { io::Direction::Input };

	// The "read" task
	ReadTask _readTask { *this };

	// The path of the file we are reading
	std::filesystem::path _filePath;

	// The data block that contains the state
	memory::ObjectBlock<State> _stateDataBlock;
};

} // namespace xentara::samples::simpleDriver