FROM omnetpp/omnetpp-base:u18.04 as base
LABEL maintainer="Rudolf Hornig <rudi@omnetpp.org>"

# first stage - build omnet
FROM base as builder
ARG VERSION=5.6.2
WORKDIR /root
CMD echo "Starting building a docker for Omnet-5-6-2."
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-$VERSION/omnetpp-$VERSION-src-core.tgz \
         --referer=https://omnetpp.org/ -O omnetpp-src-core.tgz --progress=dot:giga && \
         tar xf omnetpp-src-core.tgz && rm omnetpp-src-core.tgz
WORKDIR /root/omnetpp-$VERSION
RUN apt-get update -y
RUN apt-get install -y libboost-all-dev 
RUN git clone https://github.com/ofanan/Distributed_SFC_migration.git
ENV PATH /root/omnetpp-$VERSION/bin:$PATH
# remove unused files and build
RUN ./configure WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no && \
    make -j $(nproc) MODE=release base && \
    rm -r doc out test misc config.log config.status

# second stage - copy only the final binaries (to get rid of the 'out' folder and reduce the image size)
FROM base
ENV OPP_VER=5.6.2
RUN mkdir -p /root/omnetpp-$OPP_VER
WORKDIR /root/omnetpp-$OPP_VER
COPY --from=builder /root/omnetpp-$OPP_VER/ .
ENV PATH /root/omnetpp-$OPP_VER/bin:$PATH
RUN chmod 775 /root/ && \
    mkdir -p /root/models && \
    chmod 775 /root/models
WORKDIR /root/models
RUN echo 'PS1="omnetpp-$OPP_VER:\w\$ "' >> /root/.bashrc && chmod +x /root/.bashrc && \
    touch /root/.hushlogin
ENV HOME=/root/
WORKDIR /root/omnetpp-$OPP_VER/Distributed_SFC_migration
RUN chmod 755 /root/omnetpp-$OPP_VER/Distributed_SFC_migration/.myalias
CMD ["/bin/bash", "--init-file", "/root/.bashrc"]

