#include <getopt.h>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

#include <mage.h>

using namespace mage;
using namespace std;

enum Color {
	RED     = 31,
	GREEN   = 32,
	YELLOW  = 33,
	BLUE    = 34,
	MAGENTA = 35,
	CYAN    = 36,
	GREY    = 37
};

std::string format(const std::string& text, Color color, bool bold = false) {
	std::stringstream ss;
	char* colorStr = (char*)malloc(2 * sizeof(char));
	sprintf(colorStr, "%d", color);
	ss << RL_PROMPT_START_IGNORE
	   << "\033["
	   << (bold ? "1" : "0")
	   << ";"
	   << colorStr
	   << "m"
	   << RL_PROMPT_END_IGNORE
	   << text
	   << RL_PROMPT_START_IGNORE
	   << "\033[0m"
	   << RL_PROMPT_END_IGNORE;
	free(colorStr);
	return ss.str();
}

std::string red(const std::string& text) {
	return format(text, RED);
}

std::string redBold(const std::string& text) {
	return format(text, RED, true);
}

std::string green(const std::string& text) {
	return format(text, GREEN);
}

std::string greenBold(const std::string& text) {
	return format(text, GREEN, true);
}

std::string yellow(const std::string& text) {
	return format(text, YELLOW);
}

std::string yellowBold(const std::string& text) {
	return format(text, YELLOW, true);
}

std::string blue(const std::string& text) {
	return format(text, BLUE);
}

std::string blueBold(const std::string& text) {
	return format(text, BLUE, true);
}

std::string magenta(const std::string& text) {
	return format(text, MAGENTA);
}

std::string magentaBold(const std::string& text) {
	return format(text, MAGENTA, true);
}

std::string cyan(const std::string& text) {
	return format(text, CYAN);
}

std::string cyanBold(const std::string& text) {
	return format(text, CYAN, true);
}

std::string grey(const std::string& text) {
	return format(text, GREY);
}

std::string greyBold(const std::string& text) {
	return format(text, GREY, true);
}

void showHelp() {
	cout << magentaBold("  Usage: magecli -a [application name] -d [domain] [-p [protocol]] [-h]") << endl;
	cout << endl;

	cout << cyan("    -a\t");
	cout << grey("The name of the MAGE application you wish to access");
	cout << endl;

	cout << cyan("    -d\t");
	cout << grey("The domain name or IP address where the MAGE instance is hosted");
	cout << endl;

	cout << cyan("    -p\t");
	cout << grey("The protocol through which you wish to communicate with MAGE (default: http)");
	cout << endl;

	cout << cyan("    -h\t");
	cout << grey("Show this help screen");
	cout << endl;

	cout << endl;
}

class CliEventObserver : public mage::EventObserver {
	public:
		explicit CliEventObserver(mage::RPC* client) : m_pClient(client) {}
		virtual void ReceiveEvent(const std::string& name,
								  const Json::Value& data = Json::Value::null) const {
			std::cout << greenBold("Receive event") << ": " << name << std::endl;
			if (data != Json::Value::null) {
				std::cout << cyanBold("data") << ": " << data.toStyledString() << std::endl;
			}
			if (name == "session:set") {
				HandleSessionSet(data);
			}
		}
		void HandleSessionSet(const Json::Value& data) const {
			m_pClient->SetSession(data["key"].asString());
		}

	private:
		mage::RPC* m_pClient;
};

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

	cout << cyan("Connecting to application ");
	cout << magentaBold(application);
	cout << yellowBold("@");
	cout << magentaBold(domain);
	cout << endl;

	mage::RPC client(application, domain);

	CliEventObserver eventObserver(&client);
	client.AddObserver(&eventObserver);

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

	// Readline variables
	char *buf;
	std::stringstream prompt;
	prompt << cyanBold("mage") << yellowBold("> ");

	// disable auto-complete
	rl_bind_key('\t',rl_insert);

	// Show prompt and wait
	while((buf = readline(prompt.str().c_str())) != NULL) {
		command = buf;
		free(buf);

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

		// Add the command to the history
		add_history(command.c_str());

		if (userCommand == "setSession") {
			client.SetSession(data);
			continue;
		}

		if (userCommand == "clearSession") {
			client.ClearSession();
			continue;
		}

		if (userCommand == "pullEvents") {
			if (data == "longpolling") {
				client.PullEvents(LONGPOLLING);
			} else {
				client.PullEvents();
			}
			continue;
		}

		if (userCommand == "startPolling") {
			if (data == "shortpolling") {
				client.StartPolling(SHORTPOLLING);
			} else {
				client.StartPolling();
			}
			continue;
		}

		if (userCommand == "exit") {
			break;
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
