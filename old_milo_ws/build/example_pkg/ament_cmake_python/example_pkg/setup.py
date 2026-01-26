from setuptools import find_packages
from setuptools import setup

setup(
    name='example_pkg',
    version='0.0.0',
    packages=find_packages(
        include=('example_pkg', 'example_pkg.*')),
)
