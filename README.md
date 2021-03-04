# Building
```
avr-gcc -mmcu=attiny85 -Wall -Os -o circ.elf main.c
avr-objcopy -j .text -j .data -O ihex circ.elf circ.hex
```

# Flashing
```
avrdude -p attiny85 -c usbasp -e -U flash:w:circ.hex
```
