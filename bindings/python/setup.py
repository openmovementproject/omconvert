# python -m pip install -e https://github.com/digitalinteraction/omconvert.git#egg=omconvert&subdirectory=bindings/python
from setuptools import setup

setup(
    name='omconvert',
    url='https://github.com/digitalinteraction/omconvert/',
    author='Dan Jackson',
    packages=['omconvert'],
    install_requires=['numpy', 'pandas'],
    version='0.1',
    license='BSD',
    description='omconvert wrapper code',
    #scripts=['bin/build-omconvert.sh'],
    long_description=open('README.md').read(),
)
