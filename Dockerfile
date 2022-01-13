# docker build -t untwine:latest .
# docker run -it -v "$PWD":/untwine untwine:latest --files=/untwine/test.las --output_dir=/untwine/test.ept.copc --single_file --stats

FROM connormanning/entwine:dependencies
MAINTAINER Howard Butler <howard@hobu.co>

RUN cd /opt && \
 git clone -b 3.0.0 https://github.com/verma/laz-perf.git && \
 cd /opt/laz-perf && \
 mkdir build && \
 cd build && \
 cmake .. && \
 make && \
 make install && \
 cd ..

ENV LAZPERF_DIR /opt/laz-perf/build/cpp
ENV LD_LIBRARY_PATH /opt/laz-perf/build/cpp

RUN cd /opt && \
    git clone -b main https://github.com/hobu/untwine.git && \
    cd /opt/untwine && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

RUN chmod -R go+rX /opt/laz-perf /opt/untwine

RUN useradd --create-home --no-log-init --shell /bin/bash untwine
USER untwine
RUN cd $HOME

ENTRYPOINT ["untwine"]
