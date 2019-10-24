#!/bin/bash

python setup.py install --force --verbose && echo "2137 NOWPYTEST!" && pytest -s -v -ra --basetemp="$1"