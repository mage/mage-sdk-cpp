#include "exceptions.h"

namespace mage {
	MageError::MageError(const std::string message) :
		std::runtime_error(message)
	{
	};

	MageRPCError::MageRPCError(const int code, const std::string message) :
		MageError::MageError("MAGE RPC error: " + message), errorCode(code)
	{
	};

	MageRPCError::~MageRPCError() throw()
	{
	};

	int MageRPCError::code() {
		return errorCode;
	};

	MageErrorMessage::MageErrorMessage(const std::string code) :
		MageError::MageError("MAGE error message received"), errorCode(code)
	{
	};

	MageErrorMessage::MageErrorMessage(const std::string code, const std::string message) :
		MageError::MageError("MAGE error message received: " + message), errorCode(code)
	{
	};

	MageErrorMessage::~MageErrorMessage() throw()
	{
	};

	std::string MageErrorMessage::code() {
		return errorCode;
	};
};
