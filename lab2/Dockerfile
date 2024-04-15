FROM prota13/system-architecture-base

RUN pip install faker psycopg2-binary 

ADD . /opt/arch/lab2

WORKDIR /opt/arch

RUN cmake -S ./lab2 -B ./lab2/build && cd ./lab2/build && make