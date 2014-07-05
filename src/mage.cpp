#include "mage.h"

using namespace jsonrpc;

namespace mage
{

	RPC::RPC(std::string mageApplication, std::string mageDomain, std::string mageProtocol) :
		protocol(mageProtocol), domain(mageDomain), application(mageApplication)
	{
		buildConnector();
		httpClient = new HttpClient(url);
		jsonRpcClient = new Client(httpClient);
	}

	RPC::~RPC()
	{
		if (jsonRpcClient) {
			delete jsonRpcClient;
		}
		if (httpClient) {
			delete httpClient;
		}
	}

	Json::Value RPC::Call(const std::string &name, const Json::Value &params)
	{
		Json::Value res;

		try {
			jsonRpcClient->CallMethod(name, params, res);
		} catch (JsonRpcException ex) {
			throw MageRPCError(ex.GetCode(), ex.GetMessage());
		}

		if (res.isMember("errorCode")) {
			throw MageErrorMessage(res["errorCode"].asString());
		}

		/**
		 * Todo?:
		 *   foreach Event
		 *     call event callback
		 */
		return res;
	}

	void RPC::RegisterCallback(const std::string &eventName, std::function<void(Json::Value)> callback) {
		std::cout << "Registering callback for event:" << eventName << std::endl;
	}

	void RPC::SetDomain(const std::string mageDomain) {
		domain = mageDomain;
		buildConnector();
		httpClient->SetUrl(url);
	}

	void RPC::SetApplication(const std::string mageApplication) {
		application = mageApplication;
		buildConnector();
		httpClient->SetUrl(url);
	}

	void RPC::SetProtocol(const std::string mageProtocol) {
		protocol = mageProtocol;
		buildConnector();
		httpClient->SetUrl(url);
	}

	void RPC::SetSession(const std::string sessionKey) {
		httpClient->AddHeader("X-MAGE-SESSION", sessionKey);
	}

	void RPC::ClearSession() {
		httpClient->RemoveHeader("X-MAGE-SESSION");
	}

	void RPC::buildConnector() {
		url = protocol + "://" + domain + "/" + application + "/jsonrpc";
	}
}
