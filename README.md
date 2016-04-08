# LED Matrix Control for Arduino

Provides a number of functions for controlling LED matrices using an Arduino with LedControl library.

[Snake Game](img/snake.jpg)

## Functions
- Word scrolling feature with input from keyboard
- Sketching with mouse by using [this Processing sketch](https://github.com/asgaines/LED-Matrix-Processing-Sketch)
- Starry night twinkling
- LED column/row fluctuations
    
## Installing
- First install [LedControl](https://github.com/wayoda/LedControl)
- See [Arduino library install instructions](https://www.arduino.cc/en/Guide/Libraries), or try the following steps:
- Download this project ZIP file
- Unzip into Arduino sketchbook directory (often `~/Documents/Arduino`)
- Open Arduino application (close first if already open to restart)
- Select `File > Sketchbook > LED_Matrix`

## Usage
- Select one of the functions you wish to run and call function within `loop()`
- Upload code to Arduino
- If using `serialMessage()`, open Serial Monitor and type message you wish to display, then hit Enter and watch the magic scroll
- If using `serialPicture()`, make sure you have Processing and the sketch linked above installed. Open the Processing sketch and run program. A grid will appear where left clicking will select LEDs to be turned on, right click for off
    
