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

<! THIS ONE SHOULD BE IN A LATER STEP - The line in the general configuration keeps the routingTable from adding netmask routes that come from interfaces. Since these routes are added by the configurator as well, to keep things simpler,
we instruct the routingTable not to add any routes. This is added to the general configuration so it applies to the config of all steps.>
<!TODO: more on this - how does it work?>
The configuration for Step 1 is empty. The configurator configures addresses according to its default parameters, and using the default xml configuration.

<! not all parameters are set to default, dumpaddresses set to true in general config!>

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
This is false by default, but its set to true in the <i>General</i> configuration at the begining of omnetpp.ini (along with other setting, which
will be discussed later).

@dontinclude omnetpp.ini
@skipline General
@until ####

An XML configuration can be supplied with the <strong>config</strong> parameter. When the user doesn't specify an xml configuration,
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

<!is this a function?>

The xml configuration is supplied to the configurator as inline xml in this step. Xml configurations contain one <i><config></i> element. Under this root element there can be
multiple configuration elements, such as the <i><interface></i> element here.
The interface element can contain selector attributes, which limit the scope of what interfaces are affected by the <interface> element.
They can also contain parameter attributes, which deal with what parameters those selected interfaces will have, like IP addresses and
netmasks. The 'x' in the IP address and netmask signify that the value is not fixed, but the configurator should choose it automatically.
With these address templates it is possible to leave everything to the configurator or specify everything, and anything in between. <!this last one is not clear, rewrite>

<! need to explain host="" names="" address="" specifically><! do we need names=""? they have only one interface><!about xml="">
<! when there is a supplied configuration, and it doesnt specify all the addresses manually,
the entry for the default configuration must be included in order for the configurator to assign addresses to all hosts>

The xml configuration for this step contains two rules for setting the IP addresses of the two special hosts,
and the rule of the default configuration, which tells the configurator to assign the rest of the addresses automatically.

Note that the configuration is processed sequentially, thus the order of the configuration elements is important.

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

The xml configuration is supplied in an external file (step3.xml), using the xmldoc (TODO: what is this called?)

@include step3.xml

v1.
The first 3 entries instruct the configurator to assign addresses to interfaces of hosts 0 to 2 according the 10.0.0.x template,
to interfaces of hosts 3 to 5 according to the 10.0.1.x template, and so on.

v2. 
The first 3 entries instruct the configurator to assign addresses to hosts 0 to 2 with the 10.0.0 network prefix,
to hosts 3 to 5 with the 10.0.1 network prefix, and so on.

The <i>towards</i> selector can be used to easily select the interfaces that are connected towards a certain host (or set of hosts?)
The next 3 entries specify that the router's interface connecting to the subnet of hosts should be on the same subnet as those hosts.

The last entry sets the network network prefix of interfaces of all routers to be 10.1. Since the addresses for the interfaces
connected towards the hosts are already specified by a previous entry, this effects only the rest of interfaces, those facing the other routers.

<! is that correct to say network prefix?>
@section s3results Results

The assigned addresses are shown on the following image.

<img src="step3address.png" width=850px>



@fixupini

*/