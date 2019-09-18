#pragma once
#include "PortConnector.h"

class PortAggregator : public PortConnector
{
public:
	PortAggregator() : Logger("PORTS") {}
	virtual ~PortAggregator() {}

	void Connect(PortConnector &ports);
};

