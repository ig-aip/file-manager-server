
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip tar pkg-config

WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git && ./vcpkg/bootstrap-vcpkg.sh

WORKDIR /app

COPY vcpkg.json .
RUN /opt/vcpkg/vcpkg install


COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
             -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*
WORKDIR /root/
COPY --from=builder /app/build/file-manager-server .
EXPOSE 55555
CMD ["./file-manager-server"]
ADxc
