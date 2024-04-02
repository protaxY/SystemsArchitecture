FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN sudo apt-get update &&\
    sudo apt-get install -y git python3 pip gcc-12 libpq-dev postgresql-client \
    && sudo apt-get clean

RUN git clone -b poco-1.12.4-release https://github.com/pocoproject/poco.git

RUN cd poco &&\
    mkdir cmake-build &&\
    cd cmake-build &&\
    cmake .. &&\
    sudo cmake --build . --target install &&\
    cd && rm poco/* -rf 

RUN ldconfig

RUN pip install faker psycopg2-binary 

ADD . /opt/arch/lab2

WORKDIR /opt/arch

RUN cmake -S ./lab2 -B ./lab2/build && cd ./lab2/build && make

ENTRYPOINT ["lab2/build/app"]