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

TODO



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
-Theodore Glavas
-Nardo Edward Jean Gilles
-Jacoby Roy
-Yongde Yu


## Licensing

The licensing of each file is indicated inside them. The STM32 files are licensed under the STM32 license. In the case where the file doesn't contain any license, it is licensed under the repository license.


