#include <mage.h>
#include <getopt.h>
#include <iostream>

using namespace mage;
using namespace std;

std::string red(std::string text){
	return "\033[0;31m" + text + "\033[0m";
}

std::string redBold(std::string text){
	return "\033[1;31m" + text + "\033[0m";
}

std::string green(std::string text){
	return "\033[0;32m" + text + "\033[0m";
}

std::string greenBold(std::string text){
	return "\033[1;32m" + text + "\033[0m";
}

std::string yellow(std::string text){
	return "\033[0;33m" + text + "\033[0m";
}

std::string yellowBold(std::string text){
	return "\033[1;33m" + text + "\033[0m";
}

std::string blue(std::string text){
	return "\033[0;34m" + text + "\033[0m";
}

std::string blueBold(std::string text){
	return "\033[1;34m" + text + "\033[0m";
}

std::string magenta(std::string text){
	return "\033[0;35m" + text + "\033[0m";
}

std::string magentaBold(std::string text){
	return "\033[1;35m" + text + "\033[0m";
}

std::string cyan(std::string text){
	return "\033[0;36m" + text + "\033[0m";
}

std::string cyanBold(std::string text){
	return "\033[1;36m" + text + "\033[0m";
}

std::string grey(std::string text){
	return "\033[0;37m" + text + "\033[0m";
}

std::string greyBold(std::string text){
	return "\033[1;37m" + text + "\033[0m";
}

void showHelp() {
	cout << magentaBold("  Usage: magecli -a [application name] -d [domain] [-p [protocol]] [-h]") << endl;
	cout << endl;
	cout << cyan("    -a\t") << grey("The name of the MAGE application you wish to access") << endl;
	cout << cyan("    -d\t") << grey("The domain name or IP address where the MAGE instance is hosted") << endl;
	cout << cyan("    -p\t") << grey("The protocol through which you wish to communicate with MAGE (default: http)") << endl;
	cout << cyan("    -h\t") << grey("Show this help screen") << endl;
	cout << endl;
}

int main(int argc, char *argv[]) {
	std::string application = "";
	std::string domain = "";
	std::string protocol = "http";

	char c;

	while((c = getopt(argc, argv, "a:d:p:h")) != -1) {
		switch (c) {
			case 'a':
				application = std::string(optarg);
				break;
			case 'd':
				domain = std::string(optarg);
				break;
			case 'p':
				protocol = std::string(optarg);
				break;
			case 'h':
				showHelp();
				return 0;
				break;
			default:
				cerr << endl;
				cerr << redBold("  Unknow parameter: ") << c << endl;
				cerr << endl;
				showHelp();
				return 1;
		}
	}

	if (application == "" || domain == "" || protocol == "") {
		cerr << endl;
		cerr << redBold("  You need to provide an application name and a domain") << endl;
		cerr << endl;
		showHelp();
		return 1;
	}

	cout << cyan("Connecting to application ") << magentaBold(application) << yellowBold("@") << magentaBold(domain) << endl;
	mage::RPC client(application, domain);

	std::string command;
	std::string userCommand;
	std::string data;

	std::size_t pos;

	Json::Reader reader;
	Json::Value res;
	Json::Value params;

	cout << cyan("Starting the MAGE CLI interactive prompt. Press") << magentaBold(" CTRl+C ") << cyan("to exit") << endl;
	cout << cyan("Usage: ") << magentaBold("[usercommand.command] ") << blueBold("[JSON object]") << endl;
	cout << endl;

	while (true) {
		//
		// Show prompt and wait
		//
		std::cout << cyanBold("mage") << yellowBold("> ");
		std::getline (std::cin,command);

		//
		// Make sure we have a command and an object
		//
		pos = command.find(' ');

		if (pos == std::string::npos) {
			userCommand = command;
			data = "{}";

			if (userCommand == "") {
				continue;
			}
		} else {
			userCommand = command.substr(0, pos);
			data = command.substr(pos + 1);
		}

		if (userCommand == "setSession") {
			client.SetSession(data);
			continue;
		}

		if (userCommand == "clearSession") {
			client.ClearSession();
			continue;
		}

		if (!reader.parse(data, params)) {
			cerr << red("Invalid JSON data") << endl;
			continue;
		}

		try {
			res = client.Call(userCommand, params);
			cout << greenBold(userCommand) << ": " << endl << res << endl;
		} catch (mage::MageRPCError e) {
			cerr << redBold("RPC error: ") << grey(e.what()) << endl;
		} catch (mage::MageErrorMessage e) {
			cerr << red(userCommand) << red(" error: ")  << grey(e.code()) << endl;
		}
	}
}
