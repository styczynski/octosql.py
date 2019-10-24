#!/bin/bash

python setup.py install --force --verbose && python ./quicktest.py && echo "2137 NOWPYTEST!" && pytest -s -v -ra --basetemp="$1"