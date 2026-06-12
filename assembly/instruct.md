### Mechanical Assembly (the way I did it)
1. Press fit pivot, motor mount and IMU mount onto the carbon fiber tube, use some form of adhesion if necessary. Ensure alignment (especially pivot with motor mount) is perfect before any form of attempt at adhesion (which is why I avoided it). Even the slightest deviation can affect results of control.
2. Wrap motor with one layer of foam tape. Insert N20 motor (500 RPM, 6V) into the motor mount slot, encoder facing down. Zip tie motor down the center to prevent movement. Attach Flywheel to motor shaft.
3. Place IMU (with soldered right-angle header pins) into the IMU mount. The slot design of the IMU mount was an oversight paired with the fact that I soldered the header pins facing the opposite way; I had to shave off that side of the mount and added a thick foam block to work around my underwhelming design. You don't have to do this, just print a more functional mount in the first place. IMU was secured using foam tape and scotch tape.

### Electronic Assembly (the way I did it)
1. Soldering: the IMU pins to header pins, the TB6612FNG motor driver module to header pins to insert into breadboard and the motor encoder wires if you're sure they won't break down when you're coding like it frustratingly did for me. For the motor encoder wires connection, I just twist and tape it with a jumper wire after ripping off one side to expose the wire.
2. Connect the wiring as shown in the circuit diagram.
3. Not shown in diagram but AIN1 and AIN2 connects to M1 and M2 on motor encoder respectively. 
4. Power: connect ESP32 to laptop power. In breadboard power module, turn on 5V but turn off 3.3V line. 5V line is powered by a wall adapter to barrel jack source. 3.3V line powered via ESP32. All components use 3.3V except for motor. Circuit diagram goes in detail.
5. It is critical you use enough wire length to have some slack when connected to the reaction wheel inverted pendulum body. 
6. In the circuit diagram, visuals might be innaccurate (as its unverified AI generation), however you can trust the text written connections in the same image. 