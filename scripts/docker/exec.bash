#!/bin/bash

# execute from project root
# environment varables:
#   IMAGE:    name of the docker image to use
#   TARGETS:  targets to pass to `make` inside the container
#   PASS_ENV: names of environment variables to import into
#             the container during build

IMAGE="${IMAGE:-archlinux-build}"

local_conf=""

if [[ "$TARGETS" == *codecov* ]]; then
	local_conf="$local_conf $(bash <(curl -s https://codecov.io/env))"
fi

if [ -e scripts/docker/"${IMAGE}"/env ]; then
	local_conf="$local_conf --env-file scripts/docker/${IMAGE}/env"
fi

docker build -t "blank/${IMAGE}" scripts/docker/"${IMAGE}"
docker run -v "$PWD":/repo ${local_conf} "blank/${IMAGE}" /bin/bash -c "cd /repo && make -j\$(nproc) $TARGETS"
