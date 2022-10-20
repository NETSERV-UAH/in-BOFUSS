# in-Band OpenFlow Software Switch (in-BOFUSS)

This project is based on Eder's implementation of the [BOFUSS](https://github.com/CPqD/ofsoftswitch13) project (an openflow 1.3 software switch), to which enhanced in-band capabilities have been added. This project was carried out by Boby Nicusor Constantin within the framework of a master's degree project at the Polytechnic School of the UAH (Spain).  

If you want to cite this work you can use the following citation:

```tex
@article{constantin2020desarrollo,
  title={Desarrollo de una soluci{\'o}n de encaminamiento para tr{\'a}fico de control in-band en entornos SDN},
  author={Constantin, Boby Nicusor and others},
  year={2020}
}
```


# Getting Started

These instructions have been tested on Ubuntu 16.04. Other distributions or versions may need different steps. 

## Before building
The switch makes use of the NetBee library to parse packets, so we need to install it first.

1. Install the following packages:

    ```
    $ sudo apt-get install cmake libpcap-dev libxerces-c3.1 libxerces-c-dev libpcre3 libpcre3-dev flex bison pkg-config autoconf libtool libboost-dev
    ```

2. Clone and build netbee

    ```
    $ git clone https://github.com/netgroup-polito/netbee.git
    $ cd netbee/src
    $ cmake .
    $ make
    ```

3. Add the shared libraries built in `/nbeesrc/bin/` to your `/usr/local/lib` directory

    ```
    $ sudo cp ../bin/libn*.so /usr/local/lib
    ```

4. Run `ldconfig`

    ```
    $ sudo ldconfig
    ```

5. Put the contens of folder `nbeesrc/include` in the `/usr/include`

    ```
    $ sudo cp -R ../include/* /usr/include/
    ```

## Building
Run the following commands in the `ofsoftswitch13` directory to build and install everything:

    $ ./boot.sh
    $ ./configure
    $ make
    $ sudo make install

```gherkin=
Feature: Guess the word

  # The first example has two steps
  Scenario: Maker starts a game
    When the Maker starts a game
    Then the Maker waits for a Breaker to join

  # The second example has three steps
  Scenario: Breaker joins a game
    Given the Maker has started a game with the word "silky"
    When the Breaker joins the Maker's game
    Then the Breaker must guess a word with 5 characters
```
## Running
1. Start the datapath:

    ```
    $ sudo udatapath/ofdatapath --datapath-id=<dpid> --interfaces=<if-list> ptcp:<port>
    ```

    This will start the datapath, with the given datapath ID, using the interaces listed. It will open a passive TCP connection on the given port. For a complete list of options, use the `--help` argument.

2. Start the secure channel, which will connect the datapath to the controller:

    ```
    $ secchan/ofprotocol tcp:<switch-host>:<switch-port> tcp:<ctrl-host>:<ctrl-port>
    ```

    This will open TCP connections to both the switch and the controller, relaying OpenFlow protocol messages between them. For a complete list of options, use the `--help` argument.

## Configuring
You can send requests to the switch using the `dpctl` utility.

* Check the flow statistics for table 0.

    ```
    $ utilities/dpctl tcp:<switch-host>:<switch-port> stats-flow table=0
    ```

* Install a flow to match IPv6 packets with extension headers hop by hop and destination and coming from port 1.

    ```
    $ utilities/dpctl tcp:<switch-host>:<switch-port> flow-mod table=0,cmd=add in_port=1,eth_type=0x86dd,ext_hdr=hop+dest apply:output=2
    ```

* Add a meter:

    ```
    $ utilities/dpctl tcp:<switch-host>:<switch-port> meter-mod cmd=add,meter=1 drop:rate=50
    ```

* Send flow to meter table

    ```
    $ utilities/dpctl tcp:<switch-host>:<switch-port> flow-mod table=0,cmd=add in_port=1 meter:1
    ```

