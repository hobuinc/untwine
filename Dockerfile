FROM condaforge/mambaforge:latest as build
MAINTAINER Howard Butler <howard@hobu.co>

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

RUN conda create -n untwine -y
ARG GITHUB_SHA
ARG GITHUB_REPOSITORY="hobuinc/untwine"
ARG GITHUB_SERVER_URL="https://github.com"

SHELL ["conda", "run", "-n", "untwine", "/bin/bash", "-c"]

RUN mamba install -c conda-forge git compilers conda-pack cmake make ninja sysroot_linux-64=2.17 && \
    mamba install --yes -c conda-forge untwine --only-deps

RUN git clone "${GITHUB_SERVER_URL}/${GITHUB_REPOSITORY}" untwine && \
    cd untwine ; \
    git checkout ${GITHUB_SHA}

RUN mkdir -p untwine/build && \
    cd untwine/build  && \
    CXXFLAGS="-Werror=strict-aliasing" LDFLAGS="-Wl,-rpath-link,$CONDA_PREFIX/lib" cmake -G Ninja  \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_LIBRARY_PATH:FILEPATH="$CONDA_PREFIX/lib" \
        -DCMAKE_INCLUDE_PATH:FILEPATH="$CONDA_PREFIX/include" \
        -DCMAKE_INSTALL_PREFIX="$CONDA_PREFIX" \
        ..

RUN cd untwine/build  && \
    ninja

RUN cd untwine/build  && \
    ninja install

RUN conda-pack -n untwine --dest-prefix=/opt/conda/envs/untwine -o  /tmp/env.tar && \
     mkdir /venv && cd /venv && tar xf /tmp/env.tar  && \
     rm /tmp/env.tar

FROM condaforge/miniforge3

ENV CONDAENV "/opt/conda/envs/untwine"
COPY --from=build /venv "/opt/conda/envs/untwine"

ENV PROJ_NETWORK=TRUE
ENV PROJ_DATA="${CONDAENV}/share/proj"
ENV GDAL_DATA="${CONDAENV}/share/gdal"
ENV LD_LIBRARY_PATH="${CONDAENV}/lib"
ENV GEOTIFF_CSV="${CONDAENV}/share/epsg_csv"
ENV GDAL_DRIVER_PATH="${CONDAENV}/lib/gdalplugins"
ENV PATH $PATH:${CONDAENV}/bin
ENV GTIFF_REPORT_COMPD_CS=TRUE
ENV REPORT_COMPD_CS=TRUE
ENV OAMS_TRADITIONAL_GIS_ORDER=TRUE


SHELL ["conda", "run", "--no-capture-output", "-n", "untwine", "/bin/sh", "-c"]

