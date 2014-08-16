#include <mage.h>
#include <iostream>

using namespace mage;
using namespace std;

int main() {
	//
	// Here, you specify the MAGE app to connect to, and where
	// you can find it.
	//
	mage::RPC client("game", "localhost:8080");

	//
	// MAGE works with JSON-RPC: so
	// we create variables for the request's
	// parameters as well as for the response
	//
	Json::Value auth;
	Json::Value user;

	//
	// When logging in, you will need to submit:
	//
	// 1. An engine name, provided to you by the server-side developer
	// 2. A username
	// 3. A password
	//
	auth["engineName"] = "cms";
	auth["credentials"]["username"] = "username";
	auth["credentials"]["password"] = "password";

	//
	// Login will return you a user and a session. We take the
	// session and set it as our own.
	//
	// Note: It is possible that you need to call a different login
	// method. If it is the case, you should still expect to receive
	// the same data structure as with the standard ident module.
	//
	try {
		user = client.Call("ident.login", auth);

		cout << "Login succeeded, you are: ";
		cout << user["user"]["displayName"];
		cout << " (id: " << user["user"]["userId"];
		cout << ")" << endl;

		//
		// We set the session
		//
		client.SetSession(user["session"]["key"].asString());
	} catch (mage::MageClientError e) {
		cerr << "MAGE returned the following error: " << e.what() << " (code " << e.code() << ")" << endl;
		return 1;
	} catch (mage::MageRPCError e) {
		cerr << "Could not login, an RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
		return 1;
	} catch (mage::MageErrorMessage e) {
		cerr << "Login failed: "  << e.code() << endl;
		return 1;
	}

	//
	// From here, all the calls you will be doing
	// are authenticated.
	//
	Json::Value params;
	Json::Value res;

	params["somethings"] = "test";
	params["one"]["two"]["three"] = 4;

	try {
		res = client.Call("mymodule.mycommand", params);
		cout << "mymodule.mycommand (authenticated): " << res << endl;
	} catch (mage::MageClientError e) {
		cerr << "MAGE returned the following error: " << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageRPCError e) {
		cerr << "An RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageErrorMessage e) {
		cerr << "mymodule.mycommand responded with an error: "  << e.code() << endl;
	}

	//
	// To clear the session:
	//
	client.ClearSession();
}
