# Introduction

This repository includes code for building a weather station and photo frame from [Inkplate 10](https://inkplate.readthedocs.io/).

<iframe width="560" height="315" src="https://www.youtube.com/embed/PXjKgfOjtBk" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

# Usage

You could use touch pad to switch from three functions: weather station, web photo and local photo. The content will be updated every 4 hours and could be modified via `*_DELAY_US` in weather_photo_calendar.ino.

![IMG_2002](/Users/qiangzhang/Downloads/IMG_2002.JPG)

![IMG_2003](/Users/qiangzhang/Downloads/IMG_2003.JPG)

# Implementation

It is mostly built from the examples provided by InkPlate 10, including:

- [Daily_weather_station_example](https://github.com/e-radionicacom/Inkplate-Arduino-library/tree/master/examples/Inkplate10/Projects/Daily_weather_station_example)
- [Inkplate_Web_Pictures](https://github.com/e-radionicacom/Inkplate-Arduino-library/tree/master/examples/Inkplate10/Projects/Image_frame)
- [Inkplate_SD_pictures](https://github.com/e-radionicacom/Inkplate-Arduino-library/tree/master/examples/Inkplate10/Advanced_Inkplate_Features/Inkplate_SD_pictures)

Some of important changes I made:

- refactor code to isolate the logics of each of those examples;
- fetch the last touch pad event from interrupte register instead of using loop pool

# Future Work

Integrate [Google Calendar](https://github.com/e-radionicacom/Inkplate-Arduino-library/tree/master/examples/Inkplate10/Projects/Google_calendar_example) functionalities. However, It is not currently working which will causes infinitely wake-sleep-loop, though not intended.

One possible reason is the adding Google Calendar there takes too much memory that hardware could support. Removing Google Calendar example or Google Calendar alones works perfectly.

You could check [master branch](https://github.com/zhangtemplar/inkplate_weather_photo/tree/master), if you want to help.