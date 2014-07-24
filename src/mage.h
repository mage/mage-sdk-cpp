#ifndef MAGERPC_H
#define MAGERPC_H

#include <iostream>
#include <list>
#include <functional>
#include <future>

#include <jsonrpc/rpc.h>

#include "exceptions.h"
#include "eventObserver.h"

namespace mage {

	enum Transport {
		SHORTPOLLING = 0,
		LONGPOLLING
	};

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

			virtual void ReceiveEvent(const std::string& name,
			                          const Json::Value& data = Json::Value::null) const;
			void AddObserver(EventObserver* observer);

			void PullEvents(Transport transport = SHORTPOLLING);
			void StartPolling(Transport transport = LONGPOLLING);

			void SetProtocol(const std::string& mageProtocol);
			void SetDomain(const std::string& mageDomain);
			void SetApplication(const std::string& mageApplication);
			void SetSession(const std::string& sessionKey);
			void ClearSession() const;

			std::string GetUrl() const;
			std::string GetMsgStreamUrl(Transport transport = SHORTPOLLING) const;

		private:
			std::string m_sProtocol;
			std::string m_sDomain;
			std::string m_sApplication;
			std::string m_sSessionKey;

			std::list<EventObserver*> m_oObjserverList;
			std::list<std::string>    m_oMsgToConfirm;

			jsonrpc::HttpClient *m_pHttpClient;
			jsonrpc::Client     *m_pJsonRpcClient;
	};

}  // namespace mage
#endif /* MAGERPC_H */
