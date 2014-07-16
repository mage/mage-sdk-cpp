#include "mage.h"

using namespace jsonrpc;

namespace mage
{

	RPC::RPC(std::string mageApplication, std::string mageDomain, std::string mageProtocol) :
		protocol(mageProtocol), domain(mageDomain), application(mageApplication)
	{
		httpClient = new HttpClient(GetUrl());
		jsonRpcClient = new Client(httpClient);
	}

	RPC::~RPC()
	{
		delete jsonRpcClient;
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
		httpClient->SetUrl(GetUrl());
	}

	void RPC::SetApplication(const std::string mageApplication) {
		application = mageApplication;
		httpClient->SetUrl(GetUrl());
	}

	void RPC::SetProtocol(const std::string mageProtocol) {
		protocol = mageProtocol;
		httpClient->SetUrl(GetUrl());
	}

	void RPC::SetSession(const std::string sessionKey) {
		httpClient->AddHeader("X-MAGE-SESSION", sessionKey);
	}

	void RPC::ClearSession() {
		httpClient->RemoveHeader("X-MAGE-SESSION");
	}

	std::string RPC::GetUrl() {
		return protocol + "://" + domain + "/" + application + "/jsonrpc";
	}
}
