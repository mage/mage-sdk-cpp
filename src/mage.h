#ifndef MAGERPC_H
#define MAGERPC_H

#include "exceptions.h"
#include <iostream>
#if __cplusplus >= 201103L
	#include <functional>
	#include <future>
#endif
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
#if __cplusplus >= 201103L
			virtual std::future<Json::Value> Call(const std::string& name,
			                                      const Json::Value& params,
			                                      bool doAsync) const;
			virtual std::future<void> Call(const std::string& name,
			                  const Json::Value& params,
			                  const std::function<void(mage::MageError, Json::Value)>& callback,
			                  bool doAsync) const;
#endif

			void SetProtocol(const std::string& mageProtocol);
			void SetDomain(const std::string& mageDomain);
			void SetApplication(const std::string& mageApplication);
			void SetSession(const std::string& sessionKey) const;
			void ClearSession() const;

			std::string GetUrl() const;

		private:
			std::string m_sProtocol;
			std::string m_sDomain;
			std::string m_sApplication;

			jsonrpc::HttpClient *m_pHttpClient;
			jsonrpc::Client     *m_pJsonRpcClient;
	};

}  // namespace mage
#endif /* MAGERPC_H */
