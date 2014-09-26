#include "exceptions.h"

namespace mage {
	MageError::MageError(const std::string& message)
	: std::runtime_error(message)
	, m_iType(MAGE_ERROR) {
	}

	MageError::MageError(mage_error_t type, const std::string& message)
	: std::runtime_error(message)
	, m_iType(type) {
	}

	std::string MageError::code() const {
		return "unknown";
	}

	MageSuccess::MageSuccess(const std::string& message)
	: MageError::MageError(MAGE_SUCCESS, message) {
	}

	std::string MageSuccess::code() const {
		return "success";
	}

	MageClientError::MageClientError(const std::string& message)
	: MageError::MageError(MAGE_CLIENT_ERROR, message) {
	}

	std::string MageClientError::code() const {
		return "client error";
	}

	MageRPCError::MageRPCError(int code, const std::string& message)
	: MageError::MageError(MAGE_RPC_ERROR, "MAGE RPC error: " + message)
	, m_iErrorCode(code) {
	}

	std::string MageRPCError::code() const {
#ifndef UNITY
		return std::to_string(m_iErrorCode);
#else
		char buffer[10];
		sprintf(buffer, "%d", m_iErrorCode);
		return std::string(buffer);
#endif
	}

	MageErrorMessage::MageErrorMessage(const std::string& code,
	                                   const std::string& message)
	: MageError::MageError(MAGE_ERROR_MESSAGE,
	                       "MAGE error message received" +
	                       ((message != "") ?  ": " + message : ""))
	, m_sErrorCode(code) {
	}

	std::string MageErrorMessage::code() const {
		return m_sErrorCode;
	}
}  // namespace mage
