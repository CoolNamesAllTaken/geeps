## Troubleshooting
### RP2040 does not enumerate as a USB device when running program with USB functionality.

Make sure that tinyusb is installed by running `git submodule update --init` in the pico-sdk folder, then restarting the docker container to make sure it is copied over fresh!

Frickk wait this doesn't work, it's baked into the image. Poop.