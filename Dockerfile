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

# Ensure build and runtime dirs exist
RUN mkdir -p /opt/rom/obj /opt/rom/bin /opt/rom/log /opt/rom/player /opt/rom/gods /opt/rom/json/areas /opt/rom/json/config /opt/rom/json/help

EXPOSE 4000
CMD ["bash", "-lc", "./run.sh"]
