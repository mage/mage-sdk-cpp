#include "rpc.h"

#include <iostream>

using namespace jsonrpc;

namespace mage {

	RPC::RPC(const std::string& mageApplication,
	         const std::string& mageDomain,
	         const std::string& mageProtocol)
	: m_sProtocol(mageProtocol)
	, m_sDomain(mageDomain)
	, m_sApplication(mageApplication)
	, m_bShouldRunPollingThread(false)
	, m_pPollingThread(nullptr) {
		m_pHttpClient    = new HttpClient(GetUrl());
		m_pJsonRpcClient = new Client(m_pHttpClient);
	}

	RPC::~RPC() {
		if (m_pPollingThread != nullptr) {
			if (m_pPollingThread->joinable() == true) {
				m_pPollingThread->join();
			}
			delete m_pPollingThread;
		}

		delete m_pJsonRpcClient;
		delete m_pHttpClient;
	}

	void RPC::ExtractEventsFromCommandResponse(const Json::Value& myEvents) const {
		for (unsigned int i = 0; i < myEvents.size(); ++i) {
			// We can only handle string
			if (!myEvents[i].isString()) {
				continue;
			}

			Json::Reader reader;
			Json::Value event;
			if (!reader.parse(myEvents[i].asString(), event)) {
				std::cerr << "Unable to read the following event: "
						  << myEvents[i] << std::endl;
				continue;
			}

			switch (event.size()) {
				case 1:
					ReceiveEvent(event[0u].asString());
					break;
				case 2:
					ReceiveEvent(event[0u].asString(), event[1u]);
					break;
				default:
					std::cerr << "The event doesn't have the correct "
					          << "amount of data."
					          << std::endl
					          << event.toStyledString()
					          << std::endl;
			}
		}
	}

	Json::Value RPC::Call(const std::string& name,
	                      const Json::Value& params) const {
		Json::Value res;

		try {
			m_pJsonRpcClient->CallMethod(name, params, res);
		} catch (JsonRpcException ex) {
			throw MageRPCError(ex.GetCode(), ex.GetMessage());
		}

		if (res.isMember("errorCode")) {
			throw MageErrorMessage(res["errorCode"].asString());
		}

		// If the myEvents array is present
		if (res.isMember("myEvents") && res["myEvents"].isArray()) {
			ExtractEventsFromCommandResponse(res["myEvents"]);
		}

		return res;
	}

	std::future<Json::Value> RPC::Call(const std::string& name,
	                                   const Json::Value& params,
	                                   bool doAsync) const {
		std::launch policy = doAsync ? std::launch::async : std::launch::deferred;

		return std::async(policy, [this, name, params]{
			return Call(name, params);
		});
	}

	std::future<void> RPC::Call(const std::string& name,
	               const Json::Value& params,
	               const std::function<void(mage::MageError, Json::Value)>& callback,
	               bool doAsync) const {
		std::launch policy = doAsync ? std::launch::async : std::launch::deferred;

		return std::async(policy, [this, name, params, callback]{
			Json::Value res;
			mage::MageSuccess ok;

			try {
				res = Call(name, params);
				callback(ok, res);
			} catch (mage::MageError e) {
				callback(e, res);
			}
		});
	}

	void RPC::SetProtocol(const std::string& mageProtocol) {
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
		m_sProtocol = mageProtocol;
		msgStreamUrl_mutex.unlock();
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetDomain(const std::string& mageDomain) {
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
		m_sDomain = mageDomain;
		msgStreamUrl_mutex.unlock();
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetApplication(const std::string& mageApplication) {
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
		m_sApplication = mageApplication;
		msgStreamUrl_mutex.unlock();
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetSession(const std::string& sessionKey) {
		std::lock_guard<std::mutex> lock(sessionKey_mutex);

		m_pHttpClient->AddHeader("X-MAGE-SESSION", sessionKey);
		msgStreamUrl_mutex.lock();
		m_sSessionKey = sessionKey;
		msgStreamUrl_mutex.unlock();
	}

	void RPC::ClearSession() const {
		std::lock_guard<std::mutex> lock(sessionKey_mutex);

		m_pHttpClient->RemoveHeader("X-MAGE-SESSION");
	}

	std::string RPC::GetUrl() const {
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		return m_sProtocol + "://" + m_sDomain + "/" + m_sApplication + "/jsonrpc";
	}

}  // namespace mage
