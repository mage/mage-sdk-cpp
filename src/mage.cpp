#include "mage.h"

using namespace jsonrpc;

namespace mage {

	static std::vector<std::thread::id> s_runningThreadIds;

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
		this->CancelAll();

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

		/**
		 * Todo?:
		 *   foreach Event
		 *     call event callback
		 */
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
