FROM golang:1.13.3-buster
RUN apt-get update
RUN apt-get install -y python3 python3-pip
RUN pip3 install virtualenv
RUN virtualenv venv
ADD . /app/
WORKDIR /app/
RUN bash -c "source ./venv/bin/activate && pip install -r requirements_dev.txt"