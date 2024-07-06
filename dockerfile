FROM ubuntu

RUN apt-get update && apt-get upgrade

WORKDIR /app

RUN apt-get install -y g++

COPY . .

RUN g++ -o nyan main.cpp

RUN chmod +x ./nyan

RUN ./nyan