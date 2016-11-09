#!/bin/bash

# execute from project root
# environment varables:
#   IMAGE:    name of the docker image to use
#   TARGETS:  targets to pass to `make` inside the container
#   PASS_ENV: names of environment variables to import into
#             the container during build

IMAGE="${IMAGE:-archlinux-build}"

image_name="localhorsttv/${IMAGE}"
image_path="scripts/docker/${IMAGE//:/-}"

build_cmd="cd /repo && make -j\$(nproc) $TARGETS"

local_conf=""

if [[ "$TARGETS" == *codecov* ]]; then
	local_conf="$local_conf $(bash <(curl -s https://codecov.io/env))"
fi

if [ -e "${image_path}/env" ]; then
	local_conf="$local_conf --env-file ${image_path}/env"
fi

docker build -t "${image_name}" --pull=true "${image_path}"
docker run -v "$PWD":/repo ${local_conf} "${image_name}" /bin/bash -c "${build_cmd}"
