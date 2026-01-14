# GPIO mapping notes (Orange Pi Zero 2W)

This project uses libgpiod line offsets.

Quick inspection:
```bash
ls -l /dev/gpiochip*
gpioinfo /dev/gpiochip0 | less
gpioinfo /dev/gpiochip1 | less
```

If you know the header pin you wired (e.g., physical pin 22), you still need the
corresponding **line offset** on a gpiochip. The mapping depends on the board’s
device-tree configuration and kernel.

Practical approach:
1. Pick the likely chip (start with /dev/gpiochip0).
2. In `gpioinfo`, look for lines whose names match the SoC’s GPIO bank naming.
3. Temporarily request a line with a short test program (or use `gpioset` if present)
   and verify the line shows a consumer label.

Ubuntu packages that help:
```bash
sudo apt install -y gpiod
# provides: gpioinfo, gpioset, gpioget
```
