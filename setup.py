#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""The setup script."""

from setuptools import setup, find_packages, Extension
from distutils.command import build as build_module
import subprocess
import os

base_path = os.path.dirname(os.path.realpath(__file__))
libgooctosql_path = "./libs/libgooctosql"
go_src_path = "./src/lib.go"

class build(build_module.build):
  def run(self):
    subprocess.call(['go', 'install'])
    subprocess.call(['go', 'build', '-o', libgooctosql_path, '-buildmode=c-archive', go_src_path])
    build_module.build.run(self)

with open('README.rst') as readme_file:
    readme = readme_file.read()

with open('HISTORY.rst') as history_file:
    history = history_file.read()

requirements = ['Click>=7.0', ]

setup_requirements = ['pytest-runner', ]

test_requirements = ['pytest>=3', ]

setup(
    author="Piotr Styczynski",
    author_email='piotr@styczynski.in',
    python_requires='>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*, !=3.4.*',
    cmdclass = {
      'build': build,
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
    packages=find_packages(include=['octosql_py', 'octosql_py.*', 'octosql_py_native', 'octosql_py_native.*']),
    setup_requires=setup_requirements,
    test_suite='tests',
    tests_require=test_requirements,
    url='https://github.com/styczynski/octosql_py',
    version='0.2.0',
    zip_safe=False,
    ext_modules = [Extension("octosql_py_native", ["./src/python/octosql_py_native.cpp"], extra_compile_args=[], extra_link_args=[libgooctosql_path, '-framework', 'CoreFoundation', '-framework', 'Security'])]
)
