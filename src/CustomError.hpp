// Copyright (c) embedded ocean GmbH
#pragma once

#include <system_error>
#include <type_traits>

namespace xentara::samples::simpleDriver
{

// These are custom error codes used in addition to system errors for error attributes.
// This enum is in tegrated into the standard C++ error code mechanism (std::error_code),
// By supplying a specialization for std::is_error_code_enum, and a function make_error_code().
enum class CustomError
{
	// The base value for custom errors, so they don't overlap with system errors
	NoError = 0,

	// No data has been read yet.
	NoData,
	// The format of a value file was invalid
	CorruptFileContent,
	// The value in the file was out of range
	ValueOutOfRange,

	// An unknown error occurred
	UnknownError = 999
};

// Returns the custom error category to encapsulate ErrorCode errors in std::error_code objects
auto customErrorCategory() noexcept -> const std::error_category &;

// Support automatic conversion from CustomError to std::error_code
inline auto make_error_code(CustomError error) noexcept -> std::error_code
{
	return { int(error), customErrorCategory() };
}

// Support automatic conversion from CustomError to std::error_condition. We do this,
// because CustomError is platform independent, and hence satisfies the conditions for
// an std::error_condition.
inline auto make_error_condition(CustomError error) noexcept -> std::error_condition
{
	return { int(error), customErrorCategory() };
}

// This is the type used for Xentara attributes that contain en error code
using ErrorAttributeValue = std::uint32_t;

// This function converts a CustomError value into a value that can be used in a Xentara error attribute
auto errorAttributeValue(CustomError error) noexcept -> ErrorAttributeValue;

// This function converts an std::error_code into a value that can be used in a Xentara error attribute
auto errorAttributeValue(std::error_code error) noexcept -> ErrorAttributeValue;

} // namespace xentara::samples::simpleDriver

namespace std
{

// This specialization enables automatic conversion from CustomError to std::error_code.
template<>
struct is_error_code_enum<xentara::samples::simpleDriver::CustomError> : public std::true_type
{
};

// This specialization enables automatic conversion from CustomError to std::error_condition. We do this,
// because CustomError is platform independent, and hence satisfies the conditions for
// an std::error_condition.
template<>
struct is_error_condition_enum<xentara::samples::simpleDriver::CustomError> : public std::true_type
{
};

}