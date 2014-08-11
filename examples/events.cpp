#include <mage.h>
#include <future>
#include <iostream>

using namespace mage;
using namespace std;

class ExampleEventObserver : public mage::EventObserver {
	public:
		explicit ExampleEventObserver(mage::RPC* client) : m_pClient(client) {}
		virtual void ReceiveEvent(const std::string& name,
		                          const Json::Value& data = Json::Value::null) const {
			std::cout << "Receive event: " << name << std::endl;
			if (data != Json::Value::null) {
				std::cout << "data: " << data.toStyledString() << std::endl;
			}

			// Set the session when it receive the session.set event
			if (name == "session.set") {
				HandleSessionSet(data);
			}
		}

		void HandleSessionSet(const Json::Value& data) const {
			m_pClient->SetSession(data["key"].asString());
		}

	private:
		mage::RPC* m_pClient;
};

int main() {
	mage::RPC client("game", "localhost:8080");

	// Initialize the EventObserver
	ExampleEventObserver eventObserver(&client);
	// Attach our EventObserver to the MAGE RPC client
	client.AddObserver(&eventObserver);

	// Login using the anonymous engine
	std::future<Json::Value> loginRes;
	Json::Value auth;
	auth["engineName"] = "anonymous";
	auth["credentials"] = Json::Value::null;
	auth["options"]["access"] = "user";

	try {
		loginRes = client.Call("ident.login", auth, true);
		loginRes.wait();
	} catch (mage::MageRPCError e) {
		cerr << "Could not login, an RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
		return 1;
	} catch (mage::MageErrorMessage e) {
		cerr << "Login failed: "  << e.code() << endl;
		return 1;
	}


	//
	// Start the polling loop in a background thread
	//
	client.StartPolling();

	//
	// From here, all the calls you will be doing
	// are authenticated.
	//

	std::future<Json::Value> res;
	Json::Value params;

	try {
		res = client.Call("mymodule.mycommand", params, true);

		// Trigger the event handlers to avoid breaking the output
		// They will be proceded at the first call of wait() or get()
		res.wait();

		// Handle the command response
		cout << "mymodule.mycommand (authenticated): " << res.get() << endl;
	} catch (mage::MageRPCError e) {
		cerr << "An RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageErrorMessage e) {
		cerr << "mymodule.mycommand responded with an error: "  << e.code() << endl;
	}

	// Wait for an user input before closing the application
	std::cout << "Type \"quit\" to quit the example." << std::endl;
	std::string command;
	do {
		std::getline(std::cin, command);
	} while (command != "quit");

	std::cout << "The example will now stop." << std::endl
	          << "Waiting for the end of the polling thread..." << std::endl;

	// Stop the polling loop
	client.StopPolling();

	std::cout << "Now exiting" << std::endl;
}
