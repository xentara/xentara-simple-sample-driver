// Copyright (c) embedded ocean GmbH
#pragma once

#include "Driver.hpp"

#include <xentara/plugin/Plugin.hpp>

namespace xentara::samples::simpleDriver
{

// This is the plugin class. This class registers all the drivers and service providers.
class Plugin final : plugin::Plugin
{
public:
	auto registerObjects(Registry & registry) -> void final
	{
		// Register the driver object.
		registry << _driver;
	}

private:
	// The driver object
	Driver _driver;

	// The global plugin object
	static Plugin _instance;
};

} // namespace xentara::samples::simpleDriver
