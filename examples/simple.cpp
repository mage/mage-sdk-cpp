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
	Json::Value params;
	Json::Value res;

	//
	// I put things in my Json::Value.
	//
	params["somethings"] = "test";
	params["one"]["two"]["three"] = 4;

	//
	// We make the call
	//
	// 1. If it succeed, we get a Json::Value
	// 2. If there is a connection or transport error, you will get a MageRPCError
	// 3. If the module's user command returns an error, you will get a MageErrorMessage
	//
	try {
		res = client.Call("mymodule.mycommand", params);
		cout << "mymodule.mycommand: " << res << endl;
	} catch (mage::MageClientError e) {
		cerr << "MAGE returned the following error: " << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageRPCError e) {
		cerr << "An RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageErrorMessage e) {
		cerr << "mymodule.mycommand responded with an error: "  << e.code() << endl;
	}
}
