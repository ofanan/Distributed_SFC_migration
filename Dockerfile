FROM omnetpp/omnetpp-base:u18.04
ARG VERSION=5.6.2
WORKDIR /containers
CMD echo "Starting building a docker for Omnet-5-6-2."
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-$VERSION/omnetpp-$VERSION-src-core.tgz \
         --referer=https://omnetpp.org/ -O omnetpp-src-core.tgz --progress=dot:giga && \
         tar xf omnetpp-src-core.tgz && rm omnetpp-src-core.tgz
WORKDIR /containers/omnetpp-$VERSION
RUN apt-get update -y
RUN apt-get install -y libboost-all-dev 
RUN git clone https://github.com/ofanan/Distributed_SFC_migration.git
ENV PATH /containers/omnetpp-$VERSION/bin:$PATH
# remove unused files and build
RUN ./configure WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no && \
    make -j $(nproc) MODE=release base && \
    rm -r doc out test misc config.log config.status
ARG PROJ_DIR=/containers/omnetpp-$VERSION/Distributed_SFC_migration
RUN chmod 777 $PROJ_DIR
RUN mkdir $PROJ_DIR/bin $PROJ_DIR/lib $PROJ_DIR/out $PROJ_DIR/out/clang-release
RUN chmod 777 $PROJ_DIR/bin $PROJ_DIR/lib $PROJ_DIR/out $PROJ_DIR/out/clang-release
RUN chmod 777 $PROJ_DIR/.myalias
CMD LANG = C
RUN ldconfig
CMD ["/bin/bash", "--init-file", "/home/ofanan/.bashrc"]

