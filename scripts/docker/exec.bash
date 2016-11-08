#!/bin/bash

# execute from project root
# environment varables:
#   IMAGE:    name of the docker image to use
#   TARGETS:  targets to pass to `make` inside the container
#   PASS_ENV: names of environment variables to import into
#             the container during build

IMAGE="${IMAGE:-archlinux-build}"

if [[ "$TARGETS" == *codecov* ]]; then
	ci_env=`bash <(curl -s https://codecov.io/env)`
fi

docker build -t "blank/${IMAGE}" scripts/docker/"${IMAGE}"
docker run -v "$PWD":/repo ${ci_env} "blank/${IMAGE}" /bin/bash -c "cd /repo && make -j\$(nproc) $TARGETS"
