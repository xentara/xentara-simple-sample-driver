// Copyright (c) embedded ocean GmbH
#include "Driver.hpp"
#include "Device.hpp"

#include <xentara/plugin/SharedFactory.hpp>
#include <xentara/io/Component.hpp>

namespace xentara::samples::simpleDriver
{

class Driver::Environment : public io::Driver::Environment
{
public:
	auto createComponent(const io::ComponentClass &componentClass, plugin::SharedFactory<io::Component> &factory)
		-> std::shared_ptr<io::Component> final;
};

auto Driver::Environment::createComponent(const io::ComponentClass &componentClass,
	plugin::SharedFactory<io::Component> &factory)
	-> std::shared_ptr<io::Component>
{
	if (&componentClass == &Device::Class::instance())
	{
		return factory.makeShared<Device>();
	}

	return nullptr;
}

auto Driver::createEnvironment() -> std::unique_ptr<io::Driver::Environment>
{
	return std::make_unique<Environment>();
}

} // namespace xentara::samples::simpleDriver