#ifndef MAGERPC_H
#define MAGERPC_H

#include "exceptions.h"
#include <iostream>
#include <functional>
#include <future>
#include <jsonrpc/rpc.h>

namespace mage {

	class RPC {
		public:
			RPC(const std::string& mageApplication,
			    const std::string& mageDomain = "localhost:8080",
			    const std::string& mageProtocol = "http");
			~RPC();

			virtual Json::Value Call(const std::string& name,
			                         const Json::Value& params) const;
			virtual std::future<Json::Value> Call(const std::string& name,
			                                      const Json::Value& params,
			                                      bool doAsync) const;
			virtual std::future<void> Call(const std::string& name,
			                               const Json::Value& params,
			                               const std::function<void(mage::MageError, Json::Value)>& callback,
			                               bool doAsync) const;

			virtual std::thread::id Call(const std::string& name,
			                              const Json::Value& params,
			                              const std::function<void(mage::MageError, Json::Value)>& callback);

			void SetProtocol(const std::string& mageProtocol);
			void SetDomain(const std::string& mageDomain);
			void SetApplication(const std::string& mageApplication);
			void SetSession(const std::string& sessionKey);
			void ClearSession();

			std::string GetUrl() const;

			void Join(std::thread::id threadId);
			void Cancel(std::thread::id threadId);

		private:
			std::string m_sProtocol;
			std::string m_sDomain;
			std::string m_sApplication;
			std::string m_sessionKey;

			std::map<std::thread::id, std::thread> m_taskList;

			static bool IsCancelThread(std::thread::id threadId);
			void CancelAll();
	};

}  // namespace mage
#endif /* MAGERPC_H */
