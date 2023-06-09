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
	// The class object containing meta-information about this skill
	class Class final : public skill::Skill::Class
	{
	public:
		auto name() const -> std::string_view final
		{
			// This is the name of the skill, as it appears in the model.json file
			return "SimpleSample"sv;
		}

		auto uuid() const -> utils::core::Uuid final
		{
			// This is an arbitrary unique UUID for the skill. This can be anything, but should never change.
			return "8640cf56-24f1-471d-a06a-2ceb7c1453b8"_uuid;
		}

		auto registerElements(Registry &registry) -> void final
		{
			// Register all the object classes
			registry <<
				Device::Class::instance() <<
				Output::Class::instance() <<
				Input::Class::instance();
		}

		auto createSkill() -> std::unique_ptr<skill::Skill> final
		{
			return std::make_unique<Skill>();
		}
	};

	auto createElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
		-> std::shared_ptr<skill::Element> final;
};

} // namespace xentara::samples::simpleDriver