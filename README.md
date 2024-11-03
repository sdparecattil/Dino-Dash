# Dino Dash - Inspired by The Dinosaur Game
![image](https://github.com/user-attachments/assets/eeff3720-5271-42d8-8b99-a201f667553c)

## Overview
This project recreates the well-known endless runner dinosaur game that runs on Google Chrome when no internet connection is detected. One player will control a joystick that allows the dinosaur to jump or duck to avoid the oncoming obstacles. The OLED display is configured so that graphics can be shown and updated constantly on it. When the player loses, an active buzzer sounds notifying the player the game is over. A capacitive touch sensor is used to reset the display. 

On startup, press down the joystick to see the dinosaur running forever while the player tries to dodge the endless onslaught of obstacles approaching. The player's score increases every time they dodge an obstacle. Upon hitting an obstacle, the player loses,  activating a buzzer and displaying their final score to the screen. Hitting the touch sensor will reset the game allowing the game to be played again.

## Demo
<a href="https://www.youtube.com/watch?v=9sKp2ngDMcU">
    <img src="https://github.com/user-attachments/assets/cf7077a8-7c2f-470b-ae21-2115b84666a2" alt="image" width="400"/>
</a>

## System Overview
![image](https://github.com/user-attachments/assets/c0cf0752-82ef-457f-bdca-aa1d350c5f5a)

Shown to the right is a schematic of the system. The Arduino Uno ATmega 328p(1) runs the game and displays it to the SSD1306 i2c OLED display(2). The joystick(3) receives inputs from the player to make the dinosaur jump and duck. The LED(6) stays on as long as the T-Rex is in the jump animation. A capacitive touch sensor(5) is used to turn the display on and off as well as reset it. At the conclusion of the game, an active buzzer(4) sounds denoting the player has lost.

## Full Wire Diagram
![image](https://github.com/user-attachments/assets/0066859e-6add-433a-b9f5-7198562942d8)




