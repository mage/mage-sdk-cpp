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
	std::future<Json::Value> res;

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
	try {
		client.Call("user.register", params, [](Json::Value res){
			cout << "user.register: " << res << endl;
		}, true);
	} catch (mage::MageRPCError e) {
		cerr << "An RPC error has occured: "  << e.what() << " (code " << e.code() << ")" << endl;
	} catch (mage::MageErrorMessage e) {
		cerr << "mymodule.mycommand responded with an error: "  << e.code() << endl;
	}
}
