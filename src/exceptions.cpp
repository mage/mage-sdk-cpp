#include "exceptions.h"

namespace mage {
	MageError::MageError(const std::string message) :
		std::runtime_error(message)
	{
		type = "MageError";
	};

	std::string MageError::code() {
		return "unknown";
	};

	MageSuccess::MageSuccess(const std::string message) :
		MageError::MageError(message)
	{
		type = "MageSuccess";
	};

	std::string MageSuccess::code() {
		return "success";
	};

	MageRPCError::MageRPCError(const int code, const std::string message) :
		MageError::MageError("MAGE RPC error: " + message), errorCode(code)
	{
		type = "MageRPCError";
	};

	MageRPCError::~MageRPCError() throw()
	{
	};

	std::string MageRPCError::code() {
		return std::to_string(errorCode);
	};

	MageErrorMessage::MageErrorMessage(const std::string code) :
		MageError::MageError("MAGE error message received"), errorCode(code)
	{
		type = "MageErrorMessage";
	};

	MageErrorMessage::MageErrorMessage(const std::string code, const std::string message) :
		MageError::MageError("MAGE error message received: " + message), errorCode(code)
	{
		type = "MageErrorMessage";
	};

	MageErrorMessage::~MageErrorMessage() throw()
	{
	};

	std::string MageErrorMessage::code() {
		return errorCode;
	};
};
