#ifndef MAGEEXCEPTIONS_H_
#define MAGEEXCEPTIONS_H_

#include <iostream>
#include <stdexcept>

namespace mage
{
	class MageError: public std::runtime_error
	{
		public:
			MageError(const std::string message);
			std::string code();
			std::string type;
	};

	class MageSuccess: public MageError
	{
		public:
			MageSuccess(const std::string message = "");
			std::string code();
	};

	class MageRPCError: public MageError
	{
		public:
			MageRPCError(const int code, const std::string message);

			virtual ~MageRPCError() throw();

			std::string code();

		private:
			const int errorCode;
	};

	class MageErrorMessage:  public MageError
	{
		public:
			MageErrorMessage(const std::string code);
			MageErrorMessage(const std::string code, const std::string message);

			virtual ~MageErrorMessage() throw();

			std::string code();

		private:
			const std::string errorCode;
	};
};
#endif
