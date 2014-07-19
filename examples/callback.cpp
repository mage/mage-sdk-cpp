#include <mage.h>
#include <future>
#include <iostream>

using namespace mage;
using namespace std;

//
// All the code (well, almost)
// is the same as simple.cpp
//

int main() {
	mage::RPC client("game", "localhost:8080");

	//
	// However, we will store the result into
	// a future object
	//
	Json::Value params;

	//
	// I put things in my Json::Value.
	//
	params["password"] = "test";

	//
	// We make the call
	//
	// The third argument is meant to define the nature of
	// the future call:
	//
	//  - false: Run at call time (when you call res.get())
	//  - true: Run asynchronously
	//
	client.Call("user.register", params, [](mage::MageError err, Json::Value res){
		if (err.type() == MAGE_RPC_ERROR) {
			cerr << "An RPC error has occured: "  << err.what() << " (code " << err.code() << ")" << endl;
			return;
		}

		if (err.type() == MAGE_ERROR_MESSAGE) {
			cerr << "mymodule.mycommand responded with an error: "  << err.code() << endl;
			return;
		}

		cout << "user.register: " << res << endl;
	}, true);
}
