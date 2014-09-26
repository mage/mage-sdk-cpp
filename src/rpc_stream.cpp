#include "rpc.h"

#include <curl/curl.h>

#include <iostream>
#ifndef UNITY
	#include <chrono>
	#include <thread>
	#ifndef SHORTPOLLING_INTERVAL_SECS
		#define SHORTPOLLING_INTERVAL_SECS 5
	#endif
#endif

using namespace jsonrpc;

namespace mage {

	void RPC::ReceiveEvent(const std::string& name, const Json::Value& data) const {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(observerList_mutex);
#endif

		std::list<EventObserver*>::const_iterator citr;
		for(citr = m_oObserverList.begin();
		    citr != m_oObserverList.end(); ++citr) {
			(*citr)->ReceiveEvent(name, data);
		}
	}

	void RPC::AddObserver(EventObserver* observer) {
#ifndef UNITY
		std::lock_guard<std::mutex> lock(observerList_mutex);
#endif

		m_oObserverList.push_back(observer);
	}

	const std::list<EventObserver*>& RPC::GetObservers() const {
		return m_oObserverList;
	}

	static int writer(char *data, size_t size, size_t nmemb,
	                  std::string *writerData) {
		if (writerData == NULL) return 0;
		writerData->append(data, size*nmemb);
		return size * nmemb;
	}

	void RPC::DoHttpGet(std::string *buffer, const std::string& url) const {
		CURL* c = curl_easy_init();
		if (!c) {
			throw MageClientError("Unable to pull events. "
			                      "Unable to initialize curl.");
		}

		curl_easy_setopt(c, CURLOPT_URL, url.c_str());
		curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(c, CURLOPT_WRITEDATA, buffer);

		CURLcode res = curl_easy_perform(c);

		curl_easy_cleanup(c);

		if(res != CURLE_OK) {
			throw MageClientError(std::string("Unable to pull events. "
			                                  "Curl error: ") +
                                  curl_easy_strerror(res));
		}
	}

	void RPC::ExtractEventsFromMsgStreamResponse(const std::string& response) {
		Json::Reader reader;
		Json::Value messages;
		if (!reader.parse(response.c_str(), messages)) {
			throw MageClientError("Unable to parse the received content from "
			                      "the message stream.");
		}

		bool hasInvalidFormatError = false;

		std::vector<std::string> members = messages.getMemberNames();
		std::vector<std::string>::const_iterator citr;
		for (citr = members.begin();
		    citr != members.end();
		    ++citr) {
			for (int i = 0; i < messages[(*citr)].size(); ++i) {
				Json::Value event = messages[(*citr)][i];
				switch (event.size()) {
					case 1:
						ReceiveEvent(event[0u].asString());
						break;
					case 2:
						ReceiveEvent(event[0u].asString(), event[1u]);
						break;
					default:
						hasInvalidFormatError = true;
						break;
				}
			}
#ifndef UNITY
			msgStreamUrl_mutex.lock();
#endif
			m_oMsgToConfirm.push_back(*citr);
#ifndef UNITY
			msgStreamUrl_mutex.unlock();
#endif
		}

		if (hasInvalidFormatError) {
			throw new MageClientError("One of the received events has an invalid format.");
		}
	}

	void RPC::PullEvents(Transport transport) {
		const std::string url = GetMsgStreamUrl(transport);
		std::string buffer;

		DoHttpGet(&buffer, url);

		// The previous messages were confirmed
#ifndef UNITY
		msgStreamUrl_mutex.lock();
#endif
		m_oMsgToConfirm.clear();
#ifndef UNITY
		msgStreamUrl_mutex.unlock();
#endif

		// No messages to read
		if (buffer.empty()) {
			return;
		}

		// Heartbeat sent by MAGE
		if (buffer == "HB") {
			return;
		}

		ExtractEventsFromMsgStreamResponse(buffer);
	}

#ifndef UNITY
	void RPC::StartPolling(Transport transport) {
		sessionKey_mutex.lock();
		if (m_sSessionKey.empty()) {
			sessionKey_mutex.unlock();
			throw MageClientError("No session key registered.");
		}
		sessionKey_mutex.unlock();

		if (m_pPollingThread != nullptr &&
		    m_pPollingThread->joinable() == true) {
			throw MageClientError("A polling thread is already running.");
		}

		if (m_pPollingThread != nullptr) {
			delete m_pPollingThread;
		}

		auto f = [this, transport](){
			std::unique_lock<std::mutex> lock(pollingThread_mutex);

			std::chrono::seconds duration;
			// In case of shortpolling we have to wait
			if (transport == SHORTPOLLING) {
				duration = std::chrono::seconds(SHORTPOLLING_INTERVAL_SECS);
			} else {
				duration = std::chrono::seconds::zero();
			}

			// Wait for duration
			// If a notification is received, it will execute the lamdba
			// If it returns true, it will stop
			// else it should continue
			while (m_bShouldRunPollingThread &&
			       pollingThread_cv.wait_for(lock, duration, [this]() {
				// To stop the waiting, we should return true
				return !m_bShouldRunPollingThread;
			}) == false) {
				try {
					PullEvents(transport);
				} catch (MageClientError error) {
					std::cerr << error.what() << std::endl;
				}
			}
		};

		// Execute the lambda in a new thread
		m_bShouldRunPollingThread = true;
		m_pPollingThread = new std::thread(f);
	}

	void RPC::StopPolling() {
		m_bShouldRunPollingThread = false;
		pollingThread_cv.notify_all();
		m_pPollingThread->join();
	}
#endif

	std::string RPC::GetConfirmIds() const {
#ifndef UNITY
		std::lock_guard<std::recursive_mutex> lock(msgStreamUrl_mutex);
#endif

		std::stringstream ss;

		bool first = true;

		std::list<std::string>::const_iterator citr;
		for (citr = m_oMsgToConfirm.begin();
			 citr != m_oMsgToConfirm.end();
			 ++citr) {
			if (!first) {
				ss << ",";
			} else {
				first = false;
			}
			ss << (*citr);
		}

		return ss.str();
	}

	std::string RPC::GetMsgStreamUrl(Transport transport) const {
#ifndef UNITY
		std::lock_guard<std::recursive_mutex> lock(msgStreamUrl_mutex);
#endif

		std::stringstream ss;
		ss << m_sProtocol
		   << "://"
		   << m_sDomain
		   << "/msgstream?transport=";

		switch (transport) {
			case SHORTPOLLING:
				ss << "shortpolling";
				break;
			case LONGPOLLING:
				ss << "longpolling";
				break;
			default:
				throw MageClientError("Unsupported transport.");
		}

#ifndef UNITY
		sessionKey_mutex.lock();
#endif
		if (m_sSessionKey.empty()) {
#ifndef UNITY
			sessionKey_mutex.unlock();
#endif
			throw MageClientError("No session key registered.");
		}

		ss << "&sessionKey="
		   << m_sSessionKey;

#ifndef UNITY
		sessionKey_mutex.unlock();
#endif

		if (!m_oMsgToConfirm.empty()) {
			ss << "&confirmIds="
			   << GetConfirmIds();
		}

		return ss.str();
	}
}  // namespace mage
