FROM golang:1.13.3-buster

LABEL author="styczynski"
LABEL version="1.0"
LABEL description="Docker image for octosql.py"

RUN apt-get update
RUN apt-get -y install python3 python3-pip
RUN pip3 install virtualenv
RUN apt-get -y install python-dev
ADD . /app/
WORKDIR /app/
RUN python3 -m virtualenv venv
RUN . ./venv/bin/activate && pip3 install -r requirements_dev.txt && python3 setup.py build --force && python3 setup.py install --force