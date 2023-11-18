# adaptable-joint-control
A standardized interface to control any joint using any type of input for control.

## Introduction

The goal of this project is to create an adaptable joint controller that can be driven through multiple different input sources. This would make plug-n-play the integration of systems that can (1) drive joints to systems that can (2) provide joint control data. 

A list of examples of systems that can provide joint control data:
1. Computer vision hand tracking
2. Haptic gloves
3. Hand simulation
4. Myoelectric sensors

These systems will then be able to connect to systems that drive joints and control those joints seamlessly.

## Applications

This project was made with joints of the hand in mind (for arm prosthetic control), but it can easily be adapted to any system with joints.


## Standadized Joint Control Interface

The secret to making this project work is a standard Joint Control Interface (JCI).

This interface is implemented using a custom protocol for transferring joint data.

This interface also assumes a full-duplex connection between the two sides of the interface. Otherwise, considerations must be made.


### Streamer and Listener

The protocol works on the basis of data senders called Streamers and data receivers called Listeners.

A board can be both a Streamer AND a Listener. For example, a board that drives joints may be listening for data, but it might also be streaming back the current state of its joints based on sensor values.

### C-flow

A C-flow is when a Streamer and a Listener agrees on the header configuration such that the Streamer can start sending C-type packets, which contain no header information but only data. The C comes from the fact that the flow is Continuous.

A C-flow is only interrupted when the Streamer decides to send a new request to enter a new C-flow configuration OR if the Listener notifies the Streamer to stop the C-flow (either because it receives C-flow packet but doesn't know the C-flow configuration OR if it wants to arbitrarily cancel the C-flow).

A C-flow Streamer can send any type of packet to the Listener as long as the packe t is not a request to enter a new C-flow, in which case the current C-flow is canceled.


### Granular Control

The JCI allows for granular control of the data by specifying a joint ID for each data element sent.

For practical purpose, when the IDs are added to the payload, they are not in front of their associated data but instead put together after the data elements. A payload with granular control enabled would look as follow:

DATA1, DATA2, DATAk, ..., ID1, ID2, ..., IDk, ...

Where IDk is the joint ID for DATAk.

### Packet Types

The protocol has 4 packets types. Every type follows the following base structure:

* Header (1 to 3 bytes)
* Payload (0 to 765 bytes)
* Checksum (0 to 1 byte)

For each control bit, assume that enable is '1' and disable is '0'.

#### S type

The S type is the basic packet used by JCI. 

* Header
  * TRANS       | (1 byte) | Transaction type (always 'S')
  * CHECKSUM_EN | (1 bit)  | Enable/disable the checksum. If enabled, the packet has a checksum byte added at the end of the packet.
  * GRAN        | (1 bit)  | Enable/Disable granular control. If enabled, the ID of the joint of each data element is sent in the payload.
  * PTYPE       | (1 bit)  | Payload type. It specifices whether the data elements of the payload are unsigned 8-bit integers or unsigned 16-bit integers.
  * SOURCE      | (1 bit)  | Source type. Specifies if the packet was sent from a joint driver unit (feedback) or a control unit (control)
  * CONT        | (1 bit)  | Request for C-flow. If enabled, it is considered a new request for C-flow.
  * RESERVED    | (3 bit)  | Padding bits.
  * PSIZE       | (1 byte) | Number of data elements in the payload (one per joint). The number of joints in the payload vary from 0 to 255.
* Payload
  * Contains the data elements for the joints. Its size in bytes is calculated as follow: (PSIZE\*8)\*(1+PTYPE+GRAN)
* Checksum
  * 1 byte checksum if enabled. For the checksum calculation, see the Checksum section


#### C Type

The C type is used in C-flow.

* Header
  * TRANS       | (1 byte) | Transaction type (always 'C')
* Payload
* Checksum

#### R type

#### A type

### Checksum

The checksum is calculated by summing all the bytes in-between the TRANS header byte and the checksum byte.


## Repository Structure

1. examples : folder containing example projects using the standardized JCI.
2. images : figures used in the README
3. src : contains the source code for the standardized JCI.


## Examples

### 1-joystick-control

This example uses two boards:

1. MCU1 is the Servo Driver Unit (SDU) and it is responsible for receiving the control data, parsing it, and driving the servos accordingly.

2. MCU2 is the Control and Telemetry Unit (CTU) and it is responsible for receiving sensor data (in this case, the joysticks) and formatting the sensor data to a standard data format to drive the servos. The standard data formatting is what makes it adaptable to any control source. This board is also used to store a sequence on control data in its Flash for later playback. The CTU also uses its UART interface to provide a command-line interface (CLI) and real-time telemetry on the serial terminal of a host computer.
The two boards communicate together through an inter-face which will either be Bluetooth or UART, depending on the time available after implementing the other features.

The high-level architecture can be seen below:

![joystick-control-arch](images/joystick-control.png)



## Contributors

The list of contributors
### Standard Joint Control Interface
-Jacoby Roy


### Examples

#### 1-joystick-control
* Theodore Glavas
* Nardo Edward Jean Gilles
* Jacoby Roy
* Yongde Yu


## Licensing

The licensing of each file is indicated inside them. The STM32 files are licensed under the STM32 license. In the case where the file doesn't contain any license, it is licensed under the repository license.


