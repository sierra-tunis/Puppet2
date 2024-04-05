#pragma once

#ifndef PUPPET_CONNECTORCLUSTER
#define PUPPET_CONNECTORCLUSTER

#include "motion_constraint.h"

template<Connector... connectors>
class ConnectorCluster {

	tuple<connectors&...> connectors_;


public:

	ConnectorCluster(connectors&... connector_refs) : connectors_(connector_refs) {

	}

	/*ConnectorCluster(connectors&...connector_refs, connectors*...connector_bases) {

	}*/

};

#endif