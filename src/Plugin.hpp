// Copyright (c) embedded ocean GmbH
#pragma once

#include "Skill.hpp"

#include <xentara/plugin/Plugin.hpp>

namespace xentara::samples::simpleDriver
{

// This is the plugin class. This class registers all the drivers and service providers.
class Plugin final : plugin::Plugin
{
public:
	auto registerSkills(Registry & registry) -> void final
	{
		// Register the skill class.
		registry << _skillClass;
	}

private:
	// The skill class
	Skill::Class _skillClass;

	// The global plugin object
	static Plugin _instance;
};

} // namespace xentara::samples::simpleDriver
