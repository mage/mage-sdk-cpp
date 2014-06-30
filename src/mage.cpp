#include "mage.h"

using namespace jsonrpc;
using namespace std;

namespace mage
{
	static std::string defaultDomain = "localhost:8080";
	static std::string defaultProtocol = "http";

	RPC::RPC(std::string mageApplication) :
		protocol(defaultProtocol), domain(defaultDomain), application(mageApplication)
	{
		init();
	}

	RPC::RPC(std::string mageApplication, std::string mageDomain) :
		protocol(defaultProtocol), domain(mageDomain), application(mageApplication)
	{
		init();
	}

	RPC::RPC(std::string mageApplication, std::string mageDomain, std::string mageProtocol) :
		protocol(mageProtocol), domain(mageDomain), application(mageApplication)
	{
		init();
	}

	void RPC::init()
	{
		buildConnector();
		httpClient = new HttpClient(url);
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
		return res["response"];
	}

	void RPC::RegisterCallback(const std::string &eventName, std::function<void(Json::Value)> callback) {
		cout << "Registering callback for event:" << eventName << endl;
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
