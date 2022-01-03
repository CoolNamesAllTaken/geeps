# geeps
Pronounced Gee-Pee-Ess.

## Docker Setup Instructions

Run these commands from the `docker` directory.

NOTE: Cloning git repos onto windows may result in files with CR+LF line endings. Docker does NOT like these, and they will break everything. Make sure that you set `git config --global core.autocrlf false` before cloning repos that will get added or mounted to a Docker container.

### Build the Docker Image

From this directory, run the following shell command.

```bash
docker build -t pico-dev-image .
```

### Run the Docker Container

Starting an interactive docker container on Linux or Mac. Mounts the `firmware` directory to `/root/firmware`.

```bash
docker run --name pico-dev-container -it --mount type=bind,source="$(pwd)"/firmware,target=/root/firmware --mount type=bind,source="$(pwd)"/modules/googletest,target=/root/modules/googletest --mount type=bind,source="$(pwd)"/test,target=/root/test pico-dev-image
```

Starting an interactive docker container on Windows. Mounts the `firmware` directory to `/root/firmware`.

```bash
winpty docker run --name pico-dev-container -it --mount type=bind,source="$(pwd)"/firmware,target=/root/firmware --mount type=bind,source="$(pwd)"/modules/googletest,target=/root/modules/googletest --mount type=bind,source="$(pwd)"/test,target=/root/test pico-dev-image
```

### Remove the Docker Image

```bash
docker image rm pico-dev-image
```

## Using VS Code inside the Docker Container

1. Install the Docker VS Code extension.
2. Right click on the available pico-dev-container and select "Attach Visual Studio Code" from the dropdown menu.
3. Open the attached VS Code, and wait for it to finish installing docker stuff.
4. In the attached visual studio code, install Cortex-Debug and the C/C++ extension.