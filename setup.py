#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""The setup script."""

from setuptools import setup, find_packages, Extension
from distutils.command import build as build_module
from setuptools.command.install import install
from setuptools.command.develop import develop
from setuptools.command.egg_info import egg_info

import subprocess
import sys
import os
import errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

base_path = os.path.dirname(os.path.realpath(__file__))
libgooctosql_directory = os.path.abspath(base_path + "/libs")
libgooctosql_path = libgooctosql_directory + "/libgooctosql"
libgooctosql_local = "./libs/native_octosql"
go_src_path = "./src/lib.go"


def custom_build_hook():
    print("Will install native library in ["+libgooctosql_path+"]")
    try:
        mkdir_p(libgooctosql_directory)
    except:
        print("ok")
    print("remove existing files")
    subprocess.call(['rm', '-r', '-f', libgooctosql_local])
    print("run native bash setup")
    subprocess.call(['bash', './setup_native.sh', libgooctosql_local, go_src_path])
    print('Remove existing files: '+libgooctosql_path)
    subprocess.call(['rm', '-r', '-f', libgooctosql_path])
    print('Copy files '+libgooctosql_local+' to '+libgooctosql_path)
    subprocess.call(['cp', libgooctosql_local, libgooctosql_path])


class CustomBuildCommand(build_module.build):
    def run(self):
        custom_build_hook()
        build_module.build.run(self)

class CustomInstallCommand(install):
    def run(self):
        install.run(self)
        custom_build_hook()


class CustomDevelopCommand(develop):
    def run(self):
        develop.run(self)
        custom_build_hook()


class CustomEggInfoCommand(egg_info):
    def run(self):
        egg_info.run(self)
        custom_build_hook()

with open('README.rst') as readme_file:
    readme = readme_file.read()

with open('HISTORY.rst') as history_file:
    history = history_file.read()

requirements = ['Click>=7.0', ]

setup_requirements = ['pytest-runner', ]

test_requirements = ['pytest>=3', ]

native_link_flags = [libgooctosql_path]
if sys.platform.startswith('darwin'):
    native_link_flags = [libgooctosql_path, '-framework', 'CoreFoundation', '-framework', 'Security']

setup(
    author="Piotr Styczynski",
    author_email='piotr@styczynski.in',
    python_requires='>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*, !=3.4.*',
    cmdclass={
        'build': CustomBuildCommand,
        'install': CustomInstallCommand,
        'develop': CustomDevelopCommand,
        'egg_info': CustomEggInfoCommand,
    },
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Natural Language :: English',
        "Programming Language :: Python :: 2",
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
    ],
    description="Octosql bindings for Python",
    entry_points={
        'console_scripts': [
            'octosql_py=octosql_py.cli:main',
        ],
    },
    install_requires=requirements,
    license="MIT license",
    long_description=readme + '\n\n' + history,
    include_package_data=True,
    keywords='octosql_py',
    name='octosql_py',
    py_modules=['octosql_py', 'octosql_py_native'],
    packages=find_packages(include=['octosql_py', 'octosql_py.*', 'octosql_py_native', 'octosql_py_native.*']),
    setup_requires=setup_requirements,
    test_suite='tests',
    tests_require=test_requirements,
    url='https://github.com/styczynski/octosql_py',
    version='0.3.0',
    zip_safe=False,
    ext_modules = [Extension("octosql_py_native", ["./src/python/octosql_py_native.cpp"], extra_compile_args=['-std=c++11'], extra_link_args=native_link_flags)]
)
