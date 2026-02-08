## Freecam is benchmark:
Freecam contains a worse case, hard and unoptimized scene. Use the default view to measure FPS and performance.

- FPS indicate effective FPS, capped at the max value in CONFIG_TARGET_RENDER_FPS.
- rFPS indicate rendering FPS, eg the amount of time the device took to render the scene.
- Tris Indicate how many triangles were rendered.

When determining what value to put in CONFIG_TARGET_RENDER_FPS, use rFPS minus 4 to 8.

### Performance table rendering freecam at 128x64 L8

|Microcontroller|Coremark (Rounded)|rFPS Performance|TARGET_RENDER_FPS|Good?|
|---|---|---|---|---|
|BL616|1000|18 FPS|14 FPS|➖|
|BL616 (Overclocked 480MHz, safe)|1600|34 FPS|25 FPS|✅|
|BL616 (Overclocked 640MHz, voltage increase 1.1v -> 1.20v)|2100|38 FPS|28 FPS|✅✅|
|RP2040 (1 CPU)|235|x|x|❌❌|
|ESP32S3 (1 CPU)|610|13 FPS|10FPS|➖|
|RP2350 M33(1 CPU)|500|8 FPS|5 FPS|❌|
|RP2350 M33 (Overclocked 375MHz)|1100|23 FPS|18 FPS|✅|
|RP2350 Hazard3 (Overclocked 375MHz)|x|20 FPS|16 FPS|➖|
|RP2350 M33 (Overclocked 500MHz)|1666|27 FPS|22 FPS|✅|
|STM32H723VG (550MHz)|2777|54 FPS|30 FPS (display limited)|✅✅|

Anything with coremark over 1000 will do fine (STM32H7, Pentium 3, Mimxrt, CH32H417...).
Games should be designed and optimized to run at least 15 fps at this performance threshold.

### Recommended Hardware

Recommended testing display: https://goldenmorninglcd.com/oled-display/2.7-inch-oled-128x64-ssd1327z-gme12864-115-green-on-black/
Available as SPI module on aliexpress.

Recommended testing input device: https://botland.store/keyboards-for-arduino/16997-membrane-4x5-keypad-20-keys-self-adhesive-5903351247863.html
Also available for cheap off aliexpress.

Luxury displays (harder to render for!): https://www.alibaba.com/product-detail/2-7-inch-OLED-display-module_1601055405516.html, https://goldenmorninglcd.com/oled-display/3.37-inch-oled-240x128-ssd1322-gme240128-02-yellow-on-black/

Any greyscale display will do fine, ST75256 or ST7586/7 for LCD, SSD132x and SSD136x for OLED etc...
Conversion code with dithering for mono is provided (recommended high resolution display), you can add conversion code for color displays in the same way, or adapt to render in color (L3_COLORTYPE and some L3 code + texture conversions)

And remember, monochrome OLEDs look much worse in pictures than in real life. 

#### Caveats of freecam board settings:

- rp2350b_core: 2.7 inches ssd1327 display requires external power, board regulator is not powerful enough (150 mA).
- esp32s3_devkitc: Using board power for display prevents flashing for some reason, unplug display power when flashing.

### Freecam in Action

Freecam on Ai-M61-32S-kit Overclocked ot 480MHz

[bl616_480M_bench.webm](https://github.com/user-attachments/assets/6efed0ed-e3ed-4376-93a3-52f24d798bfb)

