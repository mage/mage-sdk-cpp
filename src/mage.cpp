#include "mage.h"

using namespace jsonrpc;
using namespace std;

namespace mage
{

	RPC::RPC(std::string mageApplication, std::string mageDomain, std::string mageProtocol) :
		protocol(mageProtocol), domain(mageDomain), application(mageApplication)
	{
		init();
	}

	RPC::~RPC()
	{
		delete jsonRpcClient;
		delete httpClient;
	}

	void RPC::init()
	{
		httpClient = new HttpClient(GetUrl());
		jsonRpcClient = new Client(httpClient);
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
		cout << "Registering callback for event:" << eventName << endl;
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
