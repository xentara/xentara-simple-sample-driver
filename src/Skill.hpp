// Copyright (c) embedded ocean GmbH
#pragma once

#include "Device.hpp"
#include "Output.hpp"
#include "Input.hpp"

#include <xentara/skill/Skill.hpp>
#include <xentara/utils/core/Uuid.hpp>

#include <string_view>

namespace xentara::samples::simpleDriver
{

using namespace std::literals;

// The skill
class Skill final : public skill::Skill
{
public:
	///////////////////////////////////////////////////////
	// Virtual overrides for skill::Skill

	auto createElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
		-> std::shared_ptr<skill::Element> final;

private:
	// The skill class
	using Class = ConcreteClass<Skill,
		"SimpleSampleDriver",
		"8640cf56-24f1-471d-a06a-2ceb7c1453b8"_uuid,
		Device::Class,
		Output::Class,
		Input::Class>;

	// The skill class object
	static Class _class;
};

} // namespace xentara::samples::simpleDriver