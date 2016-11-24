#!/bin/bash

# execute from project root
# environment varables:
#   IMAGE:    name of the docker image to use
#   TARGETS:  targets to pass to `make` inside the container
#   KEEP:     file names to copy back out after all TARGETS have run

IMAGE="${IMAGE:-archlinux-build}"

image_name="localhorsttv/${IMAGE}"
image_path="scripts/docker/${IMAGE//:/-}"

xvfb_cmd="xvfb-run -a --server-args='-screen 0 1024x768x24 +extension RANDR +extension GLX'"

build_cmd="git clone /repo /workdir && cd /workdir && make -j\$(nproc) $TARGETS"

if [[ "$KEEP" != "" ]]; then
	build_cmd="${build_cmd} && cp -Rv $KEEP /repo"
fi


local_conf=""

if [[ "$TARGETS" == *codecov* ]]; then
	local_conf="$local_conf $(bash <(curl -s https://codecov.io/env))"
fi

docker build -t "${image_name}" --pull=true "${image_path}"
docker run -v "$PWD":/repo ${local_conf} "${image_name}" sh -c "$xvfb_cmd sh -c 'env ; glewinfo | head ; ${build_cmd}'"
