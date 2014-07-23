#include "mage.h"

#include <curl/curl.h>

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

	static int writer(char *data, size_t size, size_t nmemb,
	                  std::string *writerData) {
		if (writerData == NULL) return 0;
		writerData->append(data, size*nmemb);
		return size * nmemb;
	}

	void RPC::PullEvents() {
		if (m_sSessionKey.empty()) {
			std::cerr << "No session ke registered." << std::endl;
			return;
		}

		CURL* c;
		static std::string buffer;

		c = curl_easy_init();
		if (!c) {
			std::cerr << "Unable to pull events. "
			          << "Unable to initialize curl."
			          << std::endl;
			return;
		}

		std::string url = GetMsgStreamUrl();
		if (!m_oMsgToConfirm.empty()) {
			url.append("&confirmIds=");
			bool first = true;
			std::list<std::string>::const_iterator citr;
			for (citr = m_oMsgToConfirm.cbegin();
			     citr != m_oMsgToConfirm.cend();
			     ++citr) {
				if (!first) {
					url.append(",");
				} else {
					first = false;
				}
				url.append(*citr);
			}
			m_oMsgToConfirm.clear();
		}

		curl_easy_setopt(c, CURLOPT_URL, url.c_str());
		curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(c, CURLOPT_WRITEDATA, &buffer);

		CURLcode res = curl_easy_perform(c);
		if(res != CURLE_OK) {
			std::cerr << "Unable to pull events. Curl error: "
			          << curl_easy_strerror(res) << std::endl;
			curl_easy_cleanup(c);
			return;
		}

		curl_easy_cleanup(c);

		Json::Reader reader;
		Json::Value messages;
		if (!reader.parse(buffer.c_str(), messages)) {
			std::cerr << "Unable to parse the received content from "
			          << "the message stream." << std::endl;
			return;
		}
		std::cout << "Messages: " << messages.toStyledString() << std::endl;
		std::cout << "# events: " << messages.getMemberNames().size() << std::endl;
		std::vector<std::string> members = messages.getMemberNames();
		std::vector<std::string>::const_iterator citr;
		for (citr = members.begin();
		    citr != members.end();
		    ++citr) {
			std::cout << "Message: " << (*citr) << std::endl;
			for (int i = 0; i < messages[(*citr)].size(); ++i) {
				Json::Value event = messages[(*citr)][i];
				std::cout << "Event: " << event[0u].asString();
				if (event.size() > 1) {
					std::cout << " - data: " << (event[1u]).toStyledString();
				}
				std::cout << std::endl;
			}
			m_oMsgToConfirm.push_back(*citr);
		}
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

	void RPC::SetSession(const std::string& sessionKey) {
		m_pHttpClient->AddHeader("X-MAGE-SESSION", sessionKey);
		m_sSessionKey = sessionKey;
	}

	void RPC::ClearSession() const {
		m_pHttpClient->RemoveHeader("X-MAGE-SESSION");
	}

	std::string RPC::GetUrl() const {
		return m_sProtocol + "://" + m_sDomain + "/" + m_sApplication + "/jsonrpc";
	}

	std::string RPC::GetMsgStreamUrl() const {
		return m_sProtocol + "://" + m_sDomain + "/msgstream?transport=shortpolling&sessionKey=" + m_sSessionKey;
	}
}  // namespace mage
