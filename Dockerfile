FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN sudo apt-get update &&\
    sudo apt-get install -y git python3 pip iputils-ping gcc-12 libpq-dev postgresql-client wrk libssl-dev zlib1g-dev librdkafka-dev mysql-client libmysqlclient-dev libboost-all-dev\
    && sudo apt-get clean

RUN git clone -b poco-1.12.4-release https://github.com/pocoproject/poco.git
RUN cd poco &&\
    mkdir cmake-build &&\
    cd cmake-build &&\
    cmake .. &&\
    cmake --build . --config Release &&\
    sudo cmake --build . --target install &&\
    cd &&\
    rm poco -rf 

RUN git clone https://github.com/tdv/redis-cpp.git
RUN cd redis-cpp &&\
    mkdir build &&\ 
    cd build &&\ 
    cmake .. &&\ 
    make &&\ 
    sudo make install &&\
    cd &&\
    rm redis-cpp -rf 

RUN ldconfig

RUN pip install faker psycopg2

WORKDIR /opt/arch

ENTRYPOINT ["/usr/bin/bash"]