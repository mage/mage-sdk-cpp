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

			virtual void Call(const std::string& name,
			                  const Json::Value& params,
			                  const std::function<void(mage::MageError, Json::Value)>& callback);

			void SetProtocol(const std::string& mageProtocol);
			void SetDomain(const std::string& mageDomain);
			void SetApplication(const std::string& mageApplication);
			void SetSession(const std::string& sessionKey) const;
			void ClearSession() const;

			std::string GetUrl() const;

			void Cancel();

		private:
			std::string m_sProtocol;
			std::string m_sDomain;
			std::string m_sApplication;

			jsonrpc::HttpClient *m_pHttpClient;
			jsonrpc::Client     *m_pJsonRpcClient;

			std::thread          m_task;

			static bool IsCancelAndCleanThread(std::__thread_id threadId);
	};

}  // namespace mage
#endif /* MAGERPC_H */
