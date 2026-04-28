# mars-sign-v2
Mars cheer signs (4004)

These are the 4004 led signs we made for use in the stands by our teammates and spectators.
They are based on the ESP32-S3 microcontroller from Espressif (Espressif ESP32-S3-DevKitC-1).

The signs are comprised of a series of WS2812 addressable RGB leds 264 pixels long. The strip is addressed using the FastLED library and is seperated in code to be 4 segments.
The "4" is 60 pixels in length and the "0" is 72 pixe3ls in length.

The signs connect automatically to a small handheld controller that passes a "mode" to the connected signs. That mode is received and case logic is used to determine the function to display.


LEt's have some fun creating various annimations to display on these!
