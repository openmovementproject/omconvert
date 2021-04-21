# python -m pip install -e "git+https://github.com/digitalinteraction/omconvert.git#egg=openmovement&subdirectory=bindings/python"
from setuptools import setup

setup(
    name='openmovement',
    url='https://github.com/digitalinteraction/omconvert/',
    author='Dan Jackson',
    packages=['openmovement'],
    install_requires=['numpy', 'pandas'],
    version='0.1',
    license='BSD',
    description='Open Movement processing code',
    #scripts=['bin/build-omconvert.sh'],
    long_description=open('README.md').read(),
)
