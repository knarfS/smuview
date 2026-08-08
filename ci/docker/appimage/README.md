# AppImage Build Image for SmuView

## Description

This image contains everything needed to build a SmuView AppImage for Linux.
It is based on Ubuntu 22.04 LTS, the oldest Ubuntu LTS release still supported
at the time of writing. Using an older LTS maximizes compatibility, allowing
the generated AppImage to run on a wider range of Linux distributions.

The image is used in the GitHub CI pipeline to build SmuView AppImages, but can
also be used to build AppImages locally.

The GitHub Container Registry (GHCR) provides a multi-architecture image for
`linux/amd64` and `linux/arm64`:

`ghcr.io/knarfs/smuview/appimage-builder:latest`

## Using locally

The image can be used locally in combination with the CI build scripts located
in `ci/scripts/linux`. For more information, see the `Readme.md` file in
`ci/scripts/linux`.

To start the image:
```bash
docker run --rm -it ghcr.io/knarfs/smuview/appimage-builder:latest /bin/bash
```

## Content

- Ubuntu 22.04 LTS
- Qwt 6.3.0
- All dependencies required to build libserialport, libsigrok, libsigrokcxx and
  SmuView

## Environment variables

- `BASE_DIR="/opt"`
- `APPIMAGE_EXTRACT_AND_RUN="1"` (Required when running AppImages inside Docker
  containers)

## Building

Ensure Docker BuildKit is enabled when using `docker buildx`.

### Building locally

```bash
docker build \
  -f Dockerfile \
  -t smuview/appimage-builder:snapshot \
  .
```

### Cross-Building an arm64 image locally

Prepare the host for cross-compilation::
```bash
docker run --privileged --rm tonistiigi/binfmt --install arm64
docker buildx create --use
```

or:
```bash
sudo apt install qemu-user-static binfmt-support
docker buildx create --use
```

Building:
```bash
docker buildx build \
  -f Dockerfile \
  -t smuview/appimage-builder:snapshot-arm64 \
  --platform linux/arm64 \
  --load \
  .
```

### Building a multi-arch image locally

```bash
docker buildx build \
  -f Dockerfile \
  -t smuview/appimage-builder:snapshot \
  --platform linux/amd64,linux/arm64 \
  --output=type=image \
  .
```

The image is exported as an OCI image and is not loaded into the local Docker
image store. To make it available locally, use `--load` (single-platform only)
or push it directly with `--push`.

## Notes

Starting with Qwt 6.4.0 we can disable some of the stuff with:
```bash
export QWT_NO_EXAMPLES=1
export QWT_NO_PLAYGROUND=1
export QWT_NO_TESTS=1
```

## TODO

- Do we really have to change `QWT_INSTALL_PREFIX`?
- Switch from `python3.10-full` and `libpython3.10-dev` to `python3-full` and
  `libpython3-dev`. `ci/scripts/linux/build_appimage.sh` must be updated
  accordingly.
