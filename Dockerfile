FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    csh \
    ca-certificates \
    bash \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/rom
COPY . .

# Sanity checks
RUN ls -la /opt/rom
RUN test -f /opt/rom/Makefile

# Build using the repo-root Makefile
RUN make -k

# Ensure runtime dirs exist (compose bind-mounts will also create them on host)
RUN mkdir -p /opt/rom/log /opt/rom/player /opt/rom/json/areas

EXPOSE 4000
CMD ["bash", "-lc", "./run.sh"]
