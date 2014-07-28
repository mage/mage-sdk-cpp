#ifndef MAGEEVENT_OBSERVER_H
#define MAGEEVENT_OBSERVER_H

#include <string>
#include <jsonrpc/rpc.h>

namespace mage {

	class EventObserver {
		public:
			EventObserver() {}
			virtual ~EventObserver() {}

			virtual void ReceiveEvent(const std::string& name,
			                          const Json::Value& data = Json::Value::null) const = 0;
	};
}  // namespace mage

#endif

