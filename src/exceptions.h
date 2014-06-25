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
	};

	class MageRPCError: public MageError
	{
		public:
			MageRPCError(const int code, const std::string message);

			virtual ~MageRPCError() throw();

			int code();

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


