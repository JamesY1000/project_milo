from setuptools import find_packages
from setuptools import setup

setup(
    name='milo_bringup',
    version='0.0.0',
    packages=find_packages(
        include=('milo_bringup', 'milo_bringup.*')),
)
