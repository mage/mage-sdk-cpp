#include "exceptions.h"

namespace mage {
	MageError::MageError(const std::string& message)
	: MageError::MageError(MAGE_ERROR, message)
	{
	};

	MageError::MageError(mage_error_t _type, const std::string& message)
	: std::runtime_error(message)
	, m_iType(_type)
	{
	};

	std::string MageError::code() const {
		return "unknown";
	};

	MageSuccess::MageSuccess(const std::string& message)
	: MageError::MageError(MAGE_SUCCESS, message)
	{
	};

	std::string MageSuccess::code() const {
		return "success";
	};

	MageRPCError::MageRPCError(int code, const std::string& message)
	: MageError::MageError(MAGE_RPC_ERROR, "MAGE RPC error: " + message)
	, errorCode(code)
	{
	};

	std::string MageRPCError::code() const {
		return std::to_string(errorCode);
	};

	MageErrorMessage::MageErrorMessage(const std::string& code,
	                                   const std::string& message)
	: MageError::MageError(MAGE_ERROR_MESSAGE,
	                       "MAGE error message received" + 
	                       ((message != "") ?  ": " + message : ""))
	, errorCode(code)
	{
	};

	std::string MageErrorMessage::code() const {
		return errorCode;
	};
};
