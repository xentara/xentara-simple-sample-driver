// Copyright (c) embedded ocean GmbH
#include "Skill.hpp"
#include "Device.hpp"

#include <xentara/skill/Element.hpp>
#include <xentara/skill/ElementFactory.hpp>

namespace xentara::samples::simpleDriver
{

auto Skill::createElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
	-> std::shared_ptr<skill::Element>
{
	if (&elementClass == &Device::Class::instance())
	{
		return factory.makeShared<Device>();
	}

	return nullptr;
}

} // namespace xentara::samples::simpleDriver