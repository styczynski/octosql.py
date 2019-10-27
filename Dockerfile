FROM golang:1.13.3-buster
RUN apt-get update
RUN apt-get -y install python3 python3-pip
RUN pip3 install virtualenv
RUN apt-get -y install python-dev
RUN virtualenv venv
ADD . /app/
WORKDIR /app/
RUN bash -c "source ./venv/bin/activate && pip3 install -r requirements_dev.txt"