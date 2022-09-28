## Troubleshooting
### RP2040 does not enumerate as a USB device when running program with USB functionality.

Make sure that tinyusb is installed by running `git submodule update --init` in the pico-sdk folder, then restarting the docker container to make sure it is copied over fresh!

Frickk wait this doesn't work, it's baked into the image. Poop.


### Getting a weird link issue.

Make sure that you added the file to the CMakeLists.txt file AND added its directory using `add_subdirectory()`!