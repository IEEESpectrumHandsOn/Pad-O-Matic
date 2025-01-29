# Pad-O-Matic

Arduino code for the Pad-O-Matic, a stand-alone machine for printing sets of cryptographic one-time pads. The Pad-O-Matic was created for the February 2025 Hands On column of IEEE Spectrum. 

  Reads an 8-bit random number from a noise source that has been digitised
  and debiased. Then throws away byte if it is greater than 250, and does
  modular division by 10 on any other bytes to get digits from 0-9. 

  The digits are then used to create one time pads with 250 digits in each,
  grouped in blocks of five digits. digits are stored then sent to a thermal 
  printer to be printed twice, one copy for each user. 

  To see the accompanying IEEE Spectrum Hands On article, visit:

  https://spectrum.ieee.org/diy-one-time-pad-machine

  IEEE Spectrum is cool. IFYKYK. Geeks get it, you should too!

  Microcontroller target: Uno R4

  Requires the Adafruit Adruino Library for Small Thermal Printers: an oldie but still a goodie.
  Install from Arduino library manager or get from here: 

  https://github.com/adafruit/Adafruit-Thermal-Printer-Library

  This code incorporates material from the thermal printer library example. Additional
  code licensed under MIT License.
