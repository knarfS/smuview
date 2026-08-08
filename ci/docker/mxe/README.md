# MXE Build Image for SmuView

## Description

This image contains everything needed to cross-compile SmuView for Windows. It
uses MXE (M cross environment) for cross-compiling SmuView and its dependencies
for Windows.

The currently supported target is `x86_64-w64-mingw32.static.posix`.

The image is used in the GitHub CI pipeline to build SmuView for Windows, but
can also be used to build SmuView for Windows locally.

The GitHub Container Registry (GHCR) provides an image for `linux/amd64`:

`ghcr.io/knarfs/smuview/mxe-builder:latest`

## Supported architectures

- Host: `linux/amd64`
- Target: `x86_64-w64-mingw32.static.posix`

## Using locally

The image can be used locally in combination with the CI build scripts located
in `ci/scripts/mxe`. For more information, see the `Readme.md` file in
`ci/scripts/mxe`.

To start the image:
```bash
docker run --rm -it ghcr.io/knarfs/smuview/mxe-builder:latest /bin/bash
```

## Content

- Qwt 6.3.0
- All dependencies required to build libserialport, libsigrok, libsigrokcxx and
  SmuView

## Environment variables

- `BASE_DIR="/opt"`
- `MXE_DIR="${BASE_DIR}/mxe"`

## Building locally

```bash
docker build \
  -f Dockerfile \
  -t smuview/mxe-builder:snapshot \
  .
```

## Notes

- MXE patch for updating check (see patch file for details)
- MXE patch for updating libusb1 (see patch file for details)
- MXE patch for updating Qwt to the latest version (see patch file for details)
- ToDo: Add packets required for building PulseView (`scons`, `sdcc`)
- Package `doxygen is a dependency for libsigrok C++, but is not available in
  MXE. The host `doxygen` installation is sufficient for building
