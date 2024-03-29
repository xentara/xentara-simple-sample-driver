// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/data/Quality.hpp>
#include <xentara/io/Io.hpp>
#include <xentara/io/IoClass.hpp>
#include <xentara/plugin/EnableSharedFromThis.hpp>
#include <xentara/memory/Array.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/process/StageAgnosticTask.hpp>
#include <xentara/utils/atomic/Optional.hpp>

#include <functional>
#include <string_view>
#include <filesystem>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

class Device;

// This class represents a single output. This sample class writes a floating point value to a text file.
// The value of the outout is written lazily using a Xentara task "write" that must be attached to a Xentara timer or event.
// The value is written lazily, so that reads and writes can be coordinated using the Xentara scheduler.
class Output final : public io::Io, public plugin::EnableSharedFromThis<Output>
{
private:
	// This structure is used to publish the configuration parameters as Xentara attributes
	struct Config final
	{
		// The file name
		std::string _fileName;
	};
	
public:
	// This is the class object for the output. The class contains meta-information about this
	// type of element.
	class Class final : public io::IoClass
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
			return "Output"sv;
		}
	
		auto uuid() const -> utils::core::Uuid final
		{
			// This is an arbitrary unique UUID for the driver. This can be anything, but should never change.
			return "213a66f2-bada-46ce-b9ff-bee77da56163"_uuid;
		}

	private:
        // A handle that allows us to find our custom configuration parameters in the element configuration
		// data block. The initializer registers the handle with the classes configuration block.
		memory::Array::ObjectHandle<Config> _configHandle { config().appendObject<Config>() };

		// The global object that represents the class
		static Class _instance;
	};

	// This constructor attaches the output to its device
	Output(std::reference_wrapper<Device> device) :
		_device(device)
	{
	}

	auto dataType() const -> const data::DataType &;

	auto directions() const -> io::Directions;

	auto resolveAttribute(std::string_view name) -> const model::Attribute * final;
	
	auto resolveTask(std::string_view name) -> std::shared_ptr<process::Task> final;

	auto resolveEvent(std::string_view name) -> std::shared_ptr<process::Event> final;

	auto readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle final;

	auto writeHandle(const model::Attribute &attribute) noexcept -> data::WriteHandle final;

	auto realize() -> void final;

	// A Xentara attribute containing the current value. This is a membor of this class rather than
	// of the attributes namespace, because the access flags differ from class to class
	static const model::Attribute kValueAttribute;

protected:
	auto loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void final;

private:
	// This structure represents the current state of the output
	struct State
	{
		// The update time stamp
		std::chrono::system_clock::time_point _writeTime { std::chrono::system_clock::time_point::min() };
		// The error code when writing the value, or a default constructed std::error_code object for none.
		// The error is default initialized, because it is not an error if the value was never written.
		int _writeError { 0 };
	};

	// This class providing callbacks for the Xentara scheduler for the "write" task
	class WriteTask : public process::Task
	{
	public:
		// This constuctor attached the task to its parent output
		WriteTask(std::reference_wrapper<Output> output) : _output(output)
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
		// A reference to the parent output
		std::reference_wrapper<Output> _output;
	};

	// This function writes the value to the file
	auto writeFile(double value) -> void;

	// This function is called by the "write" task. It writes the value in _pendingValue to the file, and reports the result.
	// If there is no value pending, this function does nothing.
	auto perfromWriteTask(const process::ExecutionContext &context) -> void;
	// Updates the state and sends events depending on the write result
	auto reportWriteResult(const process::ExecutionContext &context, std::error_code error = std::error_code()) -> void;

	// This function is called by the value write handle. It does not actually write the value,
	// but places it into the _pendingValue member, to be written by the "write" task when it is called.
	// This is doene:
	// 1. To allow reads and writes to be properly scheduled
	// 2. To keep write handles real-time safe, which writing a file is not
	auto scheduleWrite(double value) -> void
	{
		_pendingValue.store(value, std::memory_order_release);
	}

	// The device this output belongs to
	std::reference_wrapper<Device> _device;
	
	// A Xentara event that is fired when the value was successfully written
	process::Event _writtenEvent { io::Direction::Output };
	// A Xentara event that is fired when a write error occurred
	process::Event _writeErrorEvent { io::Direction::Output };

	// The "write" task
	WriteTask _writeTask { *this };

	// The path of the file we are reading
	std::filesystem::path _filePath;

	// The last pending write value
	utils::atomic::Optional<double> _pendingValue;

	// The data block that contains the state
	memory::ObjectBlock<State> _stateDataBlock;
};

} // namespace xentara::samples::simpleDriver