FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN sudo apt-get update &&\
    sudo apt-get install -y git python3 pip iputils-ping gcc-12 libpq-dev postgresql-client wrk libssl-dev zlib1g-dev librdkafka-dev mysql-client libmysqlclient-dev libboost-all-dev\
    && sudo apt-get clean

RUN git clone https://github.com/pocoproject/poco.git
# RUN git clone https://gitlab.com/systemsarchitecture/poco.git
# RUN git clone --depth 1 -b poco-1.12.4-release https://github.com/pocoproject/poco.git

RUN cd poco &&\
    mkdir cmake-build &&\
    cd cmake-build &&\
    cmake .. &&\
    cmake --build . --config Release &&\
    sudo cmake --build . --target install &&\
    cd && rm poco/* -rf 

RUN ldconfig

WORKDIR /opt/arch

ENTRYPOINT ["/usr/bin/bash"]