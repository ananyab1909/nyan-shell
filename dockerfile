FROM ubuntu

RUN apt-get update && apt-get upgrade

WORKDIR /app

RUN apt-get install -y g++

RUN apt-get install -y neofetch

RUN apt-get install -y inotify-tools

RUN apt-get install -y xsysinfo

COPY . .

RUN g++ -o nyan main.cpp

RUN chmod +x ./nyan

RUN ./nyan


