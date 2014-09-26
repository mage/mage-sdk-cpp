#ifndef MAGEEXCEPTIONS_H
#define MAGEEXCEPTIONS_H

#include <string>
#include <stdexcept>

namespace mage {

	enum mage_error_t {
		MAGE_ERROR = 0,
		MAGE_SUCCESS,
		MAGE_CLIENT_ERROR,
		MAGE_RPC_ERROR,
		MAGE_ERROR_MESSAGE
	};

	class MageError: public std::runtime_error {
		public:
			MageError(const std::string& message);
#ifndef UNITY
			virtual ~MageError() {}
#else
			virtual ~MageError() throw() {}
#endif
			virtual std::string code() const;
			int type() const { return m_iType; }
		protected:
			MageError(mage_error_t _type, const std::string& message);
			const mage_error_t m_iType;
	};

	class MageSuccess: public MageError {
		public:
			MageSuccess(const std::string& message = "");
#ifndef UNITY
			virtual ~MageSuccess() {}
#else
			virtual ~MageSuccess() throw() {}
#endif
			virtual std::string code() const;
	};

	class MageClientError: public MageError {
		public:
			MageClientError(const std::string& message = "");
#ifndef UNITY
			virtual ~MageClientError() {}
#else
			virtual ~MageClientError() throw() {}
#endif
			virtual std::string code() const;
	};

	class MageRPCError: public MageError {
		public:
			MageRPCError(int code, const std::string& message);
#ifndef UNITY
			virtual ~MageRPCError() {}
#else
			virtual ~MageRPCError() throw() {}
#endif
			virtual std::string code() const;

		private:
			const int m_iErrorCode;
	};

	class MageErrorMessage: public MageError {
		public:
			MageErrorMessage(const std::string& code,
			                 const std::string& message = "");
#ifndef UNITY
			virtual ~MageErrorMessage() {}
#else
			virtual ~MageErrorMessage() throw() {}
#endif
			virtual std::string code() const;

		private:
			const std::string m_sErrorCode;
	};
}  // namespace mage
#endif
