APPLICATION = super_bin

BOARD ?= nucleo-f401re

RIOTBASE ?= $(CURDIR)/../..

USEMODULE += periph_gpio
USEMODULE += xtimer
USEMODULE += shell_commands
USEMODULE += emcute

# Include packages that pull up and auto-init the link layer.
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
# Modules for ethos (ethernet over serial) and uhcp(micro host configuration protocol)
USEMODULE += stdio_ethos
USEMODULE += gnrc_uhcpc

CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=5

IPV6_PREFIX ?= 2000:2::/64 #insert your global prefix
STATIC_ROUTES ?= 1
#Connectivity throught tap0 interface
UPLINK ?= ethos
ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)
TAP ?= tap0

# The Broker address and port
BROKER_ADDR = fec0:affe::1
BROKER_PORT = 1885

CFLAGS += -DSERVER_ADDR='"$(SERVER_ADDR)"'
CFLAGS += -DSERVER_PORT=$(SERVER_PORT)

#Script that sets up a tap device, configures a prefix and starts a uhcpd server serving that prefix towards the tap device
host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

FEATURES_REQUIRED = periph_gpio
FEATURES_REQUIRED = periph_adc

include $(RIOTBASE)/Makefile.include