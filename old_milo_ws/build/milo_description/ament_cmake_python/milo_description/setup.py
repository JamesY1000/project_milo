from setuptools import find_packages
from setuptools import setup

setup(
    name='milo_description',
    version='0.0.0',
    packages=find_packages(
        include=('milo_description', 'milo_description.*')),
)
