# Setup Instructions

Run these commands from the `docker` directory.

NOTE: Cloning git repos onto windows may result in files with CR+LF line endings. Docker does NOT like these, and they will break everything. Make sure that you set `git config --global core.autocrlf false` before cloning repos that will get added or mounted to a Docker container.

## Build the Docker Image

From this directory, run the following shell command.

```bash
docker build -t pico-dev-image .
```

## Run the Docker Container

Starting an interactive docker container on Linux or Mac. Mounts the `firmware` directory to `/root/firmware`.

```bash
docker run --name pico-dev-container -it --mount type=bind,source="$(pwd)"/firmware,target=/root/firmware pico-dev-image
```

Starting an interactive docker container on Windows. Mounts the `firmware` directory to `/root/firmware`.

```bash
winpty docker run --name pico-dev-container -it --mount type=bind,source="$(pwd)"/firmware,target=/root/firmware pico-dev-image
```

## Remove the Docker Image

```bash
docker image rm pico-dev-image
```
