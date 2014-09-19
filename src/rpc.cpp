#include "rpc.h"

using namespace jsonrpc;

namespace mage {

	static std::vector<std::thread::id> s_runningThreadIds;

	RPC::RPC(const std::string& mageApplication,
	         const std::string& mageDomain,
	         const std::string& mageProtocol)
	: m_sProtocol(mageProtocol)
	, m_sDomain(mageDomain)
	, m_sApplication(mageApplication)
#ifndef UNITY
	, m_bShouldRunPollingThread(false)
	, m_pPollingThread(nullptr)
#endif
	{
		m_pHttpClient    = new HttpClient(GetUrl());
		m_pJsonRpcClient = new Client(m_pHttpClient);
	}

	RPC::~RPC() {
#ifndef UNITY
		this->CancelAll();

		if (m_pPollingThread != nullptr) {
			if (m_pPollingThread->joinable() == true) {
				m_pPollingThread->join();
			}
			delete m_pPollingThread;
		}
#endif

		delete m_pJsonRpcClient;
		delete m_pHttpClient;
	}

	void RPC::ExtractEventsFromCommandResponse(const Json::Value& myEvents) const {
		bool hasParseError = false;
		bool hasInvalidFormatError = false;

		for (unsigned int i = 0; i < myEvents.size(); ++i) {
			// We can only handle string
			if (!myEvents[i].isString()) {
				continue;
			}

			Json::Reader reader;
			Json::Value event;
			if (!reader.parse(myEvents[i].asString(), event)) {
				hasParseError = true;
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
					hasInvalidFormatError = true;
			}
		}

		if (hasParseError) {
			throw new MageClientError("One of the received events can't be read.");
		}

		if (hasInvalidFormatError) {
			throw new MageClientError("One of the received events has an invalid format.");
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

#ifndef UNITY
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
#endif

	std::thread::id RPC::Call(const std::string& name,
	                           const Json::Value& params,
	                           const std::function<void(mage::MageError, Json::Value)>& callback) {
		std::thread task = std::thread([this, name, params, callback]{
			Json::Value res;
			mage::MageSuccess ok;

			std::thread::id threadId = std::this_thread::get_id();

			try {
				if (IsCancelThread(threadId)) return;
				res = Call(name, params);
				if (!IsCancelThread(threadId)) callback(ok, res);
			} catch (mage::MageError e) {
				if (!IsCancelThread(threadId)) callback(e, res);
			}

			s_runningThreadIds.erase(std::remove(s_runningThreadIds.begin(), s_runningThreadIds.end(), threadId)
				, s_runningThreadIds.end());
		});

		std::thread::id id = task.get_id();
		s_runningThreadIds.push_back(id);
		m_taskList[id] = std::move(task);

		return id;
	}

	void RPC::SetProtocol(const std::string& mageProtocol) {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
#endif
		m_sProtocol = mageProtocol;
#ifndef UNITY
		msgStreamUrl_mutex.unlock();
#endif
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetDomain(const std::string& mageDomain) {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
#endif
		m_sDomain = mageDomain;
#ifndef UNITY
		msgStreamUrl_mutex.unlock();
#endif
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetApplication(const std::string& mageApplication) {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);

		msgStreamUrl_mutex.lock();
#endif
		m_sApplication = mageApplication;
#ifndef UNITY
		msgStreamUrl_mutex.unlock();
#endif
		m_pHttpClient->SetUrl(GetUrl());
	}

	void RPC::SetSession(const std::string& sessionKey) {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(sessionKey_mutex);
#endif

		m_pHttpClient->AddHeader("X-MAGE-SESSION", sessionKey);
#ifndef UNITY
		msgStreamUrl_mutex.lock();
#endif
		m_sSessionKey = sessionKey;
#ifndef UNITY
		msgStreamUrl_mutex.unlock();
#endif
	}

	void RPC::ClearSession() const {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(sessionKey_mutex);
#endif

		m_pHttpClient->RemoveHeader("X-MAGE-SESSION");
	}

	std::string RPC::GetUrl() const {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(jsonrpcUrl_mutex);
#endif

		return m_sProtocol + "://" + m_sDomain + "/" + m_sApplication + "/jsonrpc";
	}

	void RPC::Join(std::thread::id threadId) {
		if (m_taskList.count(threadId) > 0 && m_taskList[threadId].joinable()) {
			m_taskList[threadId].join();
		}
	}

	void RPC::Cancel(std::thread::id threadId) {
		if (m_taskList.count(threadId) > 0) {
			if (m_taskList[threadId].joinable()) {
				m_taskList[threadId].detach();
				s_runningThreadIds.erase(std::remove(s_runningThreadIds.begin(), s_runningThreadIds.end(), threadId)
					, s_runningThreadIds.end());
				m_taskList.erase(threadId);
			}
		}
	}

	void RPC::CancelAll() {
		for (std::map<std::thread::id, std::thread>::iterator it = m_taskList.begin(); it != m_taskList.end(); it++) {
			if (it->second.joinable()) it->second.detach();
		}
		m_taskList.clear();
		s_runningThreadIds.clear();
	}

	bool RPC::IsCancelThread(std::thread::id threadId) {
		return find(s_runningThreadIds.begin(), s_runningThreadIds.end() , threadId) == s_runningThreadIds.end();
	}
}  // namespace mage
