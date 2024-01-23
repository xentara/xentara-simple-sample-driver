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
public:
	// This is the class object for the input. The class contains meta-information about this
	// type of element.
	using Class = ConcreteClass<
		// This is the name of the element class, as it appears in the model.json file
		"Input",
		// This is an arbitrary unique UUID for the element class. This can be anything, but should never change.
		"2911026c-52c6-4c76-8da7-69e185b6f9f1"_uuid,
		// This is a human readable name for the element class.
		// NOTE: The display name must be understandable event without knowing the skill it belongs to.
		"simple sample input">;

	// This constructor attaches the input to its device
	Input(std::reference_wrapper<Device> device) :
		_device(device)
	{
	}
	
	///////////////////////////////////////////////////////
	// Virtual overrides for skill::DataPoint

	auto dataType() const -> const data::DataType &;

	auto directions() const -> io::Directions;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;

	auto forEachEvent(const model::ForEachEventFunction &function) -> bool final;
	
	auto forEachTask(const model::ForEachTaskFunction &function) -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	// A Xentara attribute containing the current value. This is a membor of this class rather than
	// of the attributes namespace, because the access flags differ from class to class
	static const model::Attribute kValueAttribute;

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
		std::error_code _error { CustomError::NoData };
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

	///////////////////////////////////////////////////////
	// Virtual overrides for skill::DataPoint

	auto load(utils::json::decoder::Object &jsonObject, config::Context &context) -> void final;

	auto realize() -> void final;

	// The device this input belongs to
	std::reference_wrapper<Device> _device;

	// The file name
	std::string _fileName;

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