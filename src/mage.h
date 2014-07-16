#ifndef MAGERPC_H
#define MAGERPC_H

#include "exceptions.h"
#include <iostream>
#include <functional>
#include <jsonrpc/rpc.h>

namespace mage
{
	class RPC
	{
		public:
			RPC(std::string mageApplication,
			    std::string mageDomain = "localhost:8080",
			    std::string mageProtocol = "http");
			~RPC();

			virtual Json::Value Call(const std::string &name, const Json::Value &params);
			virtual void RegisterCallback(const std::string &eventName, std::function<void(Json::Value)> callback);

			void SetProtocol(const std::string mageProtocol);
			void SetDomain(const std::string mageDomain);
			void SetApplication(const std::string mageApplication);
			void SetSession(const std::string sessionKey);
			void ClearSession();

			std::string GetUrl();

		private:
			std::string protocol;
			std::string domain;
			std::string application;

			jsonrpc::HttpClient *httpClient;
			jsonrpc::Client     *jsonRpcClient;
	};

} /* namespace mage */
#endif /* MAGERPC_H */
