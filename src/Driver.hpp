// Copyright (c) embedded ocean GmbH
#pragma once

#include "Device.hpp"
#include "Output.hpp"
#include "Input.hpp"

#include <xentara/io/Driver.hpp>

#include <string_view>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

// This is the driver class. It registers all the elements the driver provides,
// and creates the driver runtime environment.
class Driver final : public io::Driver
{
public:
	auto name() const -> std::string_view final
	{
		// This is the name of the driver, as it appears in the model.json file
		return "SimpleSample"sv;
	}

	auto uuid() const -> utils::core::Uuid final
	{
		// This is an arbitrary unique UUID for the driver. This can be anything, but should never change.
		return "8640cf56-24f1-471d-a06a-2ceb7c1453b8"_uuid;
	}

	auto registerObjects(Registry &registry) -> void final
	{
		// Register all the object classes
		registry <<
			Device::Class::instance() <<
			Output::Class::instance() <<
			Input::Class::instance();
	}

	auto createEnvironment() -> std::unique_ptr<io::Driver::Environment> final;

private:
	// The driver runtime environment
	class Environment;
};

} // namespace xentara::samples::simpleDriver