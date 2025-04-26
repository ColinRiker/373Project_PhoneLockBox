# [Locked in] The Phone Lock Box #
**An EECS 373 Embedded Systems Project - WN2025**

This project is a lock box for your phone to reduce distractions and help keep
students focused. Here we detail the various modules, provide a system description, and details regarding our hardware setup. Specific function and their implementations should be viewed in their relevant modules and aren't detailed here.

## Module Overview ##
<img src="https://github.com/ColinRiker/373Project_PhoneLockBox/blob/d6381f0d4689542a73fa41ce20f82c38fbb8303c/graphics/SystemDiagram.png" alt="System Diagram" style="width:50%; height:auto;"><br>
Below are our main modules that we implemented and designed with brief descriptions of each. See each file for further details on their workings (for non-trivial functions that is)<br><br>
`Shared.h` - Contains project wide definitions, data types, and some debug specific functions.<br>
`accelerometer.c` - Holds all the implementation details relevant to the accelerometer and magnetometer, including their event functions.<br>
`audio.c` - Where all the audio detection and unlocking functions exist. The header file contains the tuning parameters.<br>
`event_controller.c` - This is our very minimal real time operating system. It implements a very bare-bones scheduler that takes in all of our non interrupt based events/processes and runs them either periodically, once, for some n times. <br>
`lock_timer.c` - Our main box timer and lock implementation are housed here. The global lock timer is separated from our event system to preserve some of our schedulers invariants. <br>
`main.c` - Contains the main system loop and calls a few initialization functions. Also has the code for driving the dial to timer interaction when users are inputting the locking time<br>
`nfc.c` - The wrapper on top of the PN532 library (credit to Yehui), where our event system entry point and flag interactions live.<br>
`rotary_encoder.c` -  <br>
`Screen_driver.c` - The modified library and our display code for driving our TFT-LCD display <br>
`state_machine.c` - The main driver of our program, where events are scheduled, flags are checked, and box outputs are controlled based on our state (see below for our state diagram)<br>

## System Description ##
Here the systems general program flow is describe from box initialization all the way through a typical use. Below is the state diagram that describes our entire system.
<img src="https://github.com/ColinRiker/373Project_PhoneLockBox/blob/d6381f0d4689542a73fa41ce20f82c38fbb8303c/graphics/StateDriagram.png" alt="State Diagram" style="width:50%; height:auto;">

### Startup ###
Immediately on power up the various modules have their initialization functions called where various I2C, SPI connections are used to send relevant information to our NFC, Accelerometer, and the Display. After which our internal systems such as the state machine, event system and master timer are set up to their default states. Finally, we put disable all our interrupts, so they can be re-enabled only in the states we need.

Our state machine's entry point is a sleep state where the box schedules events relevant to wake up (accelerometer and rotary encoder polling). This is done by calling our eventRegister() and passing in the relevant sensor event/entry functions. We also enable our rotary encoder interrupt. 

### Unlocked States ###
Once the box is woken up from its initial sleep state the box enters into a state where the user can set the timer which we handle a differently then a lot of our systems by directly sampling the decoded count and passing that into timer 2 cnt register.

Once a phone is set into the box our NFC board detects and sets the NFC_PHONE_PRESENT Flag, telling the state machine to move onto into prompting the use they would like to lock the box. Once the switch interrupt has triggered and inserted the appropriate flag the box transitions into the cancelable lock state, finally after the 5-second timer calls back and inserts the timer complete flag moving into the locked states, trigger timer 2 to start counting down and engaging the solenoid.

### Locked States ###


## Hardware Overview ##
<img src="" alt="Pinout diagram" style="width:50%; height:auto;">