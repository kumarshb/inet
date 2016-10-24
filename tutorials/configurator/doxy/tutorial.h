/**
@mainpage IPv4Configurator Tutorial for the INET Framework

This tutorial will show how to use the <tt>IPv4NetworkConfigurator</tt> module to configure IP addresses and routing tables in wired and wireless IPv4 networks in the INET framework.
The tutorial is organized into multiple steps, each corresponding to a simulation model. The steps demonstrate how to accomplish certain
tasks with the configurator.

This is an advanced tutorial, and assumes that the reader is familiar with creating and running simulations in @opp and INET. If that wasn't the case,
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/"
target="_blank">TicToc Tutorial</a> is a good starting point to get familiar with @opp. The <a
href="../../../doc/walkthrough/tutorial.html" target="_blank">INET Walkthrough</a> is an introduction to INET and how to work with protocols.
The <a href="https://omnetpp.org/doc/inet/api-current/tutorials/wireless/" target="_blank">Wireless Tutorial</a> is another advanced tutorial, and deals with wireless features of the INET framework. There is a comprehensive description of the configurator's features in the <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html?p=inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator.html" target="_blank"><tt>IPv4NetworkConfigurator</tt> NED documentation</a>
in the INET reference.

For additional information, the following documentation should be useful:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

All simulation models are defined in the omnetpp.ini file as separate configurations. The configurations use a number of different networks,
defined in separate .NED files.

@section contents Contents

 - @ref step1
 - @ref step2
 - @ref step3

@nav{index,step1}

<!-------------------------------------------------------------------------------------------------------->

@page step1 Step 1 - Fully automatic IP address assignment

@section s1goals Goals

The goal of this step is to demonstrate that in many scenarios, the configurator can adequatelly configure IP addresses in a network with its default settings, without
any user input. This is useful when it is irrelevant what the nodes' actual IP addresses are in a simulation, because the goal is to study wireless
transmission ranges, for example.
In this step, we will show that the configurator's automatic IP address assignment is adequate for the example network
(this step deals with IP addresses only, routing will be discussed in later steps).

@section s1model The model

@subsection s1about  About the configurator

In INET simulations, configurator modules are responsible for assigning IP addresses to network nodes, and for setting up their
routing tables. Essentially, the configurator modules simulate a real life network administrator. There are various configurator
models in INET (<tt>IPv4NetworkConfigurator</tt>, <tt>FlatNetworkConfigurator</tt>, etc.), but this tutorial is about the features of the <tt>IPv4NetworkConfigurator</tt>, 
which we will refer to as <strong>configurator</strong>. The following is a broad review of the configurator's features and operation,
these and others will be discussed in detail in the following steps.

The configurator assigns IP addresses to interfaces, and sets up static routing in IPv4 networks.
It doesn't configure IP addresses and routes directly, but stores the configuration in its internal data structures.
Network nodes contain an instance of <tt>IPv4NodeConfigurator</tt>, which configures the corresponding node's interface table and routing table
based on information contained in the global <tt>IPv4NetworkConfigurator</tt> module.

The configurator supports automatic and manual network configuration, and their combination. By default,
the configuration is fully automatic, but the user can specify parts (or all) of the configuration manually, and the rest
will be configured automatically by the configurator. The configurator's various features can be turned on and off with NED parameters, and the details of the configuration can be specified in an xml configuration file.

@subsection s1configuration The configuration

The configuration for this step uses the <i>ConfiguratorA</i> network, defined in <i>ConfiguratorA.ned</i>.
Here is the NED source for that network:

@dontinclude ConfiguratorA.ned
@skipline ConfiguratorA
@until ####

The network looks like this:

<img src="step1network.png">

The network contains 3 routers, each connected to the other two. There are 3 subnetworks with <tt>StandardHosts</tt>, connected to the routers by ethernet switches.
It also contains an instance of <tt>IPv4NetworkConfigurator</tt>.

The configuration for this step in omnetpp.ini is the following: 

@dontinclude omnetpp.ini
@skip Step1
@until ####

The configuration for Step 1 is empty. The configurator configures addresses according to its default parameters, and using the default xml configuration.

The default parameters pertaining to IP address assignment are the following:

<pre>
assignAddresses = default(true);
assignDisjunctSubnetAddresses = default(true);
</pre>

- <strong>assignAddresses = true</strong> tells the configurator to assign IP addresses to interfaces. It assigns addresses based on the supplied xml configuration,
or the default xml configuration if none is specified. Since no xml configuration is specified in this step, it uses the default configuration.

- <strong>assignDisjunctSubnetAddresses = true</strong> sets that the configurator should assign different address prefixes and netmasks
to nodes on different links.

Additionally, the <strong>dumpAddresses</strong> parameter sets whether the configurator prints assigned IP addresses to the module output.
This is false by default, but its set to true in the <i>General</i> configuration at the begining of omnetpp.ini (along with other settings, which
will be discussed later).

@dontinclude omnetpp.ini
@skipline General
@until ####

An XML configuration can be supplied with the <i>config</i> parameter. When the user doesn't specify an xml configuration,
the configurator will use the following default configuration:

<code>
config = default(xml("<config><interface hosts='**' address='10.x.x.x' netmask='255.x.x.x'/></config>"));
</code>

The default xml configuration tells the configurator to assign IP addresses to all interfaces of all hosts, 
from the IP range 10.0.0.0 - 10.255.255.255 and netmask range 255.0.0.0 - 255.255.255.255.

@section Results

The IP addresses assigned to interfaces by the configurator are shown on the image below.
The switches and hosts connected to the individual routers are considered to be one the same link.
Note that the configurator assigned addresses sequentially starting from 10.0.0.1, while making sure that different subnets got different address prefixes and netmasks,
as instructed by the <strong>assignDisjunctSubnetAddresses</strong> parameter.

<img src="step1addresses.png" width=850px>

@nav{index,step2}
@fixupini

<!-------------------------------------------------------------------------------------------------------->

@page step2 Step 2 - Manually overriding individual IP addresses

@nav{step1,step3}

@section s2goals Goals

Sometimes the user may want to specify the IP addresses of some nodes with special purpose in the network and leave the rest for the automatic configuration. This helps remembering IP addresses of said nodes. This step demonstrates manually specifying individual IP addresses.

@section s2model The model

This step uses the <i>ConfiguratorA</i> network from the previous step. We will assign the 10.0.0.50 address to <i>host1</i>
and 10.0.0.100 to <i>host3</i>. The configurator will automatically assign addresses to the rest of the nodes.

@subsection s2config Configuration

The configuration in omnetpp.ini for this step is the following:

@dontinclude omnetpp.ini
@skip Step2
@until ####

The xml configuration can be supplied to the <i>config</i> parameter in one of two ways:

- Inline xml using the <i>xml()</i> function. The argument of the function is the xml code.
- External xml file useing the <i>xmldoc()</i> function. The argument of the function is the name of the xml configuration file.

The xml configuration is supplied to the configurator as inline xml in this step. Xml configurations contain one <i><config></i> element. Under this root element there can be
multiple configuration elements, such as the <i><interface></i> element here.
The interface element can contain selector attributes, which limit the scope of what interfaces are affected by the <interface> element.
They can also contain parameter attributes, which deal with what parameters those selected interfaces will have, like IP addresses and
netmasks. The 'x' in the IP address and netmask signify that the value is not fixed, but the configurator should choose it automatically.
With these address templates it is possible to leave everything to the configurator or specify everything, and anything in between. <!this last one is not clear, rewrite>

In the XML configuration for this step, the first two rules state that host3's interface named 'eth0' should get the IP address 10.0.0.100, and host1's interface 'eth0' should get IP 10.0.0.50.
The third rule is the default configuration, which tells the configurator to assign the rest of the addresses automatically.

Note that the configuration is processed sequentially, thus the order of the configuration elements is important.
Also, when there is a supplied configuration, and it doesnt specify all the addresses, the entry for the default configuration must be included in order for the configurator to assign addresses to all interfaces. For the manual address assignment rules to take effect,
the default configuration should be after the manual entries. This way the configurator first assigns the manually specified addresses,
and then automatically assigns the rest.

@section s2results Results

The assigned addresses are shown in the following image.

<img src="step2address.png" width=850px>

As in the previous step, the configurator assigned disjunct subnet addresses. Note that the configurator still assigned addresses sequentially,
that is after setting the 10.0.0.100 address to <i>host3</i>, it didnt go back to the beginning of the address pool when assigning the
remaining addresses.

@nav{step1,step3}
@fixupini

<!-------------------------------------------------------------------------------------------------------->

@page step3 Step 3 - Automatically assigning IP addresses to a subnet from a given range

@nav{step2,step4}

@section s3goals Goals

Complex networks often contain several subnetworks, the user may want to assign specific IP address ranges for them.
This step demonstrates how to assign a range of IP addresses to subnets with xml configuration templates.

@section s3model The model

This step uses the <i>ConfiguratorA</i> network, as in the previous two steps.
The three hosts connected to the router through the switch will be on the same subnet, and there are 3 such groups in the network.


The configuration is the following:

@dontinclude omnetpp.ini
@skipline Step3
@until ####

The xml configuration is supplied in an external file (step3.xml), using the xmldoc function:

@include step3.xml

- The first 3 lines assign IP addresses with different network prefixes to hosts in the 3 different subnets.

- The <i>towards</i> selector can be used to easily select the interfaces that are connected towards a certain host (or set of hosts?)
The next 3 entries specify that each router's interface that connects to the subnet should belong to the subnet.

- The last entry sets the network network prefix of interfaces of all routers to be 10.1. Since the addresses for the interfaces
connected towards the hosts are already specified by a previous entry, this effects only the rest of interfaces, those facing the other routers.

@section s3results Results

The assigned addresses are shown on the following image.

<img src="step3address.png" width=850px>

The addresses are assigned as intended.

@nav{step2,step4}
@fixupini

<!-------------------------------------------------------------------------------------------------------->

@page step4 Step 4 - Fully automatic static routing table configuration

@nav{step3,step5}

@section s4goals Goals

Just as with IP addresses, in many cases the configurator configures routes in a network adequatelly without any user input.
This step demonstrates the default configuration for routing.

@section s4model The model

This step uses the <i>ConfiguratorB</i> network, defined in ConfiguratorB.ned. This extends the previous network, <i>ConfiguratorA</i> with
the addition of a <tt>RoutingTableCanvasVisualizer</tt> module.

@dontinclude ConfiguratorB.ned
@skip ConfiguratorB
@until ####

<img src="step4network.png">

@subsection s4config Configuration

The configuration for this step in omnetpp.ini is the following:

@dontinclude omnetpp.ini
@skip Step4
@until ####

A ping app in <i>host0</i> is configured to send ping packets to <i>host7</i>, which is on the other side of the network.
This should aid in visualizing routing.

The <i>RoutingTableCanvasVisualizer</i> module can be used to visualize routes in the network.
Routes to be visualized are selected with its <i>destinationFilter</i> parameter.
All routes leading towards that destination are indicated by arrows.
The default setting is <i>""</i>, which means no routes are visualized. The <i>"*.*"</i> setting visualizes all routes
going from every node to every other node, which can make the screen cluttered.
In this step the <i>destinationFilter</i> is set to visualize all routes heading towards <i>host7</i>.

The IP address assignment is fully automatic, and the resulting addresses should be the same as in Step 1.

@subsection s4defaults Configurator routing parameters

The configurator's default parameters concerning routing are the following:

<pre>
addStaticRoutes = default(true)
addDefaultRoutes = default(true)
addSubnetRoutes = default(true)
optimizeRoutes = default(true)
</pre>

The configuration for this step didn't set any of these parameters, thus the default values will take effect.

- <i>addStaticRoutes</i>: the configurator adds static routes to the routing table of all nodes in the network, 
with routes leading to all destination interfaces. This should be turned off when the xml configuration contains
manual routes.
- <i>addDefaultRoutes</i>: Add a default route if all routes from a node go through the same gateway.
This is often the case with hosts, which usually connect to a network via a single interface. This parameter
is not used if <i>addStaticRoutes = false</i>.
- <i>addSubnetRoutes</i>: Reach nodes in own subnet directly (ie. the gateway is the same as the destination), rather than with separate destination interface routes. This is only used where applicable, and not used if <i>addStaticRoutes = false</i>.
- <i>optimizeRoutes</i>: Optimize routing tables by matching entries where possible. Not used if <i>addStaticRoutes = false</i>.

Additionally, the <i>dumpTopology</i>, <i>dumpLinks</i> and <i>dumpRoutes</i> parameters are set to true in the <i>General</i> configuration.
These set the configurator to print to the module output the topology of the network, the recognized network links, and the routing tables of all nodes, respectively.

@dontinclude omnetpp.ini
@skip General
@until ####

The <i>General</i> configuration also sets GlobalARP to keep the packet exchanges simpler. GlobalARP fills the ARP tables of all nodes in advance,
so when the simulation begins no ARP exchanges are necessary. The <i>**.routingTable.netmaskRoutes = ""</i> keeps the routing table modules from
adding netmask routes to the routing tables. Netmask routes mean that nodes with the same netmask but different IP should reach each other directly.
There routes are also added by the configurator, so netmaskRoutes are turned off for the sake of simplicity.

@section Results

The visualized routes are displayed on the following image:

<img src="step4routes.png" width=850px>

Note that routes from all nodes to host7 are visualized.

The routing tables are the following:

<pre>
Node ConfiguratorB.host0
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.0         255.255.255.248  *                eth0 (10.0.0.1) 0
*                *                10.0.0.4         eth0 (10.0.0.1) 0

Node ConfiguratorB.host1
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.0         255.255.255.248  *                eth0 (10.0.0.3) 0
*                *                10.0.0.4         eth0 (10.0.0.3) 0

Node ConfiguratorB.host2
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.0         255.255.255.248  *                eth0 (10.0.0.2) 0
*                *                10.0.0.4         eth0 (10.0.0.2) 0

Node ConfiguratorB.host3
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.8         255.255.255.248  *                eth0 (10.0.0.9) 0
*                *                10.0.0.10        eth0 (10.0.0.9) 0

Node ConfiguratorB.router0
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.18        255.255.255.255  *                eth1 (10.0.0.17) 0
10.0.0.22        255.255.255.255  *                eth2 (10.0.0.21) 0
10.0.0.25        255.255.255.255  10.0.0.22        eth2 (10.0.0.21) 0
10.0.0.0         255.255.255.248  *                eth0 (10.0.0.4) 0
10.0.0.32        255.255.255.248  10.0.0.22        eth2 (10.0.0.21) 0
10.0.0.0         255.255.255.224  10.0.0.18        eth1 (10.0.0.17) 0

Node ConfiguratorB.router2
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.18        255.255.255.255  10.0.0.26        eth0 (10.0.0.25) 0
10.0.0.21        255.255.255.255  *                eth2 (10.0.0.22) 0
10.0.0.26        255.255.255.255  *                eth0 (10.0.0.25) 0
10.0.0.8         255.255.255.248  10.0.0.26        eth0 (10.0.0.25) 0
10.0.0.32        255.255.255.248  *                eth1 (10.0.0.33) 0
10.0.0.0         255.255.255.224  10.0.0.21        eth2 (10.0.0.22) 0

Node ConfiguratorB.router1
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.17        255.255.255.255  *                eth0 (10.0.0.18) 0
10.0.0.22        255.255.255.255  10.0.0.25        eth2 (10.0.0.26) 0
10.0.0.25        255.255.255.255  *                eth2 (10.0.0.26) 0
10.0.0.8         255.255.255.248  *                eth1 (10.0.0.10) 0
10.0.0.32        255.255.255.248  10.0.0.25        eth2 (10.0.0.26) 0
10.0.0.0         255.255.255.224  10.0.0.17        eth0 (10.0.0.18) 0

Node ConfiguratorB.host4
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.8         255.255.255.248  *                eth0 (10.0.0.11) 0
*                *                10.0.0.10        eth0 (10.0.0.11) 0

Node ConfiguratorB.host5
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.8         255.255.255.248  *                eth0 (10.0.0.12) 0
*                *                10.0.0.10        eth0 (10.0.0.12) 0

Node ConfiguratorB.host6
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.32        255.255.255.248  *                eth0 (10.0.0.34) 0
*                *                10.0.0.33        eth0 (10.0.0.34) 0

Node ConfiguratorB.host7
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.32        255.255.255.248  *                eth0 (10.0.0.35) 0
*                *                10.0.0.33        eth0 (10.0.0.35) 0

Node ConfiguratorB.host8
-- Routing table --
Destination      Netmask          Gateway          Iface                  Metric
10.0.0.32        255.255.255.248  *                eth0 (10.0.0.36) 0
*                *                10.0.0.33        eth0 (10.0.0.36) 0
</pre>

@fixupini

*/