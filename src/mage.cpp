#include "mage.h"

using namespace jsonrpc;

namespace mage {

	RPC::RPC(const std::string& mageApplication,
	         const std::string& mageDomain,
	         const std::string& mageProtocol)
	: m_sProtocol(mageProtocol)
	, m_sDomain(mageDomain)
	, m_sApplication(mageApplication) {
		m_pHttpClient    = new HttpClient(GetUrl());
		m_pJsonRpcClient = new Client(m_pHttpClient);
	}

	RPC::~RPC() {
		delete m_pJsonRpcClient;
		delete m_pHttpClient;
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
			Json::Value myEvents = res["myEvents"];
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
						          << "amount of data." << std::endl;
				}
			}
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

	void RPC::ReceiveEvent(const std::string& name, const Json::Value& data) const {
		std::list<EventObserver*>::const_iterator citr;
		for(citr = m_oObjserverList.cbegin();
		    citr != m_oObjserverList.cend(); ++citr) {
			(*citr)->ReceiveEvent(name, data);
		}
	}

	void RPC::AddObserver(EventObserver* observer) {
		m_oObjserverList.push_back(observer);
	}

	void RPC::SetDomain(const std::string& mageDomain) {
		m_sDomain = mageDomain;
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetApplication(const std::string& mageApplication) {
		m_sApplication = mageApplication;
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetProtocol(const std::string& mageProtocol) {
		m_sProtocol = mageProtocol;
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetSession(const std::string& sessionKey) const {
		m_pHttpClient->AddHeader("X-MAGE-SESSION", sessionKey);
	}

	void RPC::ClearSession() const {
		m_pHttpClient->RemoveHeader("X-MAGE-SESSION");
	}

	std::string RPC::GetUrl() const {
		return m_sProtocol + "://" + m_sDomain + "/" + m_sApplication + "/jsonrpc";
	}
}  // namespace mage
