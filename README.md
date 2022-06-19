It is used to play and record WAV files (games and apps) from a MicroSD card through an audio cable into where the tape deck would normally plugin thus eliminating the need for a 30-year-old tape deck. Its a play only function so no record feature. You can still record using conventional methods though. Simply find your favourite game or app and convert it to WAV (22khz/Mono/8bit)  file format, copy it to the sd card and the rest is straight forward. It is based on the firmware I developed with the same name. The firmware is opensource so you could make one of these devices yourself. 

When purchasing a microSD card do not get a cheap or a super-fast one like a class 10 card.  Operating it is simple. Once the SD is in with wavs in the root of the SD card use the up and down buttons to scroll through the wav files. Press play to play and again to pause. If you press play over a folder you will enter that folder. To exit the folder press stop. The M button enables motorcontrol if you need it. The the reset button resets the device. 

Whether the record feature added you will not be able to use conventional Arduitape device. Read the version note below. <br /> 

Record feature works by recording a wav file with an auto generate incremntal filename. It counts how many files in the folder where you are recording and uses that number to form the next filename. for example if there are 4 files in the folder the next filename would be DigiSave4.wav . <br /> 

Requires the following libraries: <br /> 
SdFat by Bill Greiman<br /> 
LiquidCrystal_I2C by Frank de Brabander <br /> 
PCM by TMRh20 <br /> 
ssd1306 by Alexey Dynda <br /> 


v1.0.10 new release with many features like dynamic SD detection - only supports OLED 128x64 or OLED 128x32 - 13/1/2020 <br /> 
v1.0.14 added motor control and button driven volume control 16/1/2020 <br />
v1.1.0 new release - added LCD(I2C) support and dynamic detection of LCD vs OLED so user doesn't have to configure that - 22/01/2020 <br /> 
v1.1.1 - tidied code up and made menu more responsive - 24/01/2020 <br />
v1.1.3.5 - added folder support - you can go 4 folders deep and back again. The complete folder path must not be more than 30 charachters so keep folder names short - 01/01/2020 <br />
v1.2.2 added record feature but had to loose dynamic LCD support for the memory. YES RECORD! but you need a MIC jack on A6 and a button on D2. Also debugged folder support. <br /> 

todo 
roadmap - Add support for Nano-ARM for 44.1khz HQ audio. Already have the code base but need to merge it in with main code base
