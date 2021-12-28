FROM ubuntu
ADD setup-scripts /usr/setup
WORKDIR /usr/setup

# Install sudo to allow the use of scripts meant for use in non-root environment.
RUN ["/usr/bin/bash", "-c", "apt update && apt -y install sudo"]
RUN ["/usr/bin/bash", "-c", "apt -y install wget"]

# Install ARM toolchain and dependencies.
RUN ["/usr/bin/bash", "-c", "/usr/setup/setup_arm_none_eabi/install_arm_none_eabi.sh"]
RUN ["/usr/bin/bash", "-c", "/usr/setup/setup_arm_none_eabi/install_dependencies.sh"]

# Install JLink toolchain and dependencies.
RUN ["/usr/bin/bash", "-c", "/usr/setup/setup_jlink/install_jlink.sh"]
RUN ["/usr/bin/bash", "-c", "/usr/setup/setup_jlink/install_dependencies.sh"]

# Install Pico SDK
ADD pico-sdk /usr/local/pico-sdk
ENV PICO_SDK_PATH /usr/local/pico-sdk
RUN ["/usr/bin/bash", "-c", "/usr/setup/setup_pico_sdk/install_dependencies.sh"]
ENV CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc
ENV CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++
