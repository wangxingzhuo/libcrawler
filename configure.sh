#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir=${BUILD_DIR:-"$root/build"}
vcpkg_root=$(pwd)/vcpkg_installed
os=$(uname -s)
arch=$(uname -m)
platform_ldlibs=

if [ -z "${VCPKG_TARGET_TRIPLET:-}" ]; then
    case "$os:$arch" in
        Darwin:arm64) triplet=arm64-osx ;;
        Darwin:x86_64) triplet=x64-osx ;;
        Linux:x86_64) triplet=x64-linux ;;
        Linux:aarch64) triplet=arm64-linux ;;
        *)
            printf '%s\n' "error: unsupported platform: $os ($arch)" >&2
            exit 1
            ;;
    esac
else
    triplet=$VCPKG_TARGET_TRIPLET
fi

case "$os" in
    Darwin)
        platform_ldlibs='-framework Security -framework CoreFoundation -framework CoreServices -licucore -liconv'
        shared_ext=dylib
        shared_flags=-dynamiclib
        ;;
    Linux)
        shared_ext=so
        shared_flags=-shared
        ;;
    *)
        printf '%s\n' "error: unsupported platform: $os ($arch)" >&2
        exit 1
        ;;
esac
impersonate_dir=${LIBCRAWLER_LIBCURL_IMPERSONATE_DIR:-}
impersonate_name=${LIBCRAWLER_LIBCURL_IMPERSONATE_NAME:-}

if [ -z "$impersonate_dir" ]; then
    printf '%s\n' 'error: set LIBCRAWLER_LIBCURL_IMPERSONATE_DIR' >&2
    exit 1
fi

if [ ! -d "$vcpkg_root/$triplet/include/lexbor" ]; then
    if [ ! -x "$vcpkg_root/vcpkg" ]; then
        printf '%s\n' "error: vcpkg was not found at $vcpkg_root/vcpkg" >&2
        exit 1
    fi

    printf '%s\n' "Lexbor is not installed for $triplet; installing dependencies..."
    "$vcpkg_root/vcpkg" install --triplet "$triplet"

    if [ ! -d "$vcpkg_root/$triplet/include/lexbor" ]; then
        printf '%s\n' "error: Lexbor installation failed for $triplet" >&2
        exit 1
    fi
fi

# if [ -z "$impersonate_name" ]; then
#     if [ -f "$impersonate_dir/libcurl-impersonate.dylib" ]; then
#         impersonate_name=libcurl-impersonate.dylib
#     elif [ -f "$impersonate_dir/libcurl.a" ]; then
#         impersonate_name=libcurl.a
#     else
#         printf '%s\n' "error: no libcurl-impersonate.dylib or libcurl.a in $impersonate_dir" >&2
#         exit 1
#     fi
# fi

mkdir -p "$build_dir"
cat > "$build_dir/config.mk" <<EOF
PROJECT_ROOT := $root
BUILD_DIR := $build_dir
VCPKG_ROOT := $vcpkg_root
VCPKG_TRIPLET := $triplet
LEXBOR_INCLUDE_DIR := $vcpkg_root/$triplet/include/lexbor
LEXBOR_LIBRARY := $vcpkg_root/$triplet/lib/liblexbor_static.a
IMPERSONATE_DIR := $impersonate_dir
IMPERSONATE_LIBRARY := $impersonate_dir/$impersonate_name
PLATFORM_LDLIBS := $platform_ldlibs
SHARED_EXT := $shared_ext
SHARED_FLAGS := $shared_flags
EOF

printf 'configured: %s\n' "$build_dir/config.mk"