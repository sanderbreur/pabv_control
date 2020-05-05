# Installation
### Getting the Code and dependencies
1. Install Anaconda and open a Anaconda Prompt
2.   `conda install -y -c conda-forge git pyinstaller make pyserial`
3. `git clone https://github.com/slaclab/pabv_control`
4. `cd pabv_control`
### Downloading arduin-cli
5. `bash bootstrap.sh`
### gnumake commands should be run in a bash shell
6.  `bash`
### Building arduino targets
7. `gnumake`
### Building client stand-alone executable
8.  `gnumake distro`
### Building the arduino software installer
9.  `gnumake installer`

### Test platforms
1. Windows 10 64-bit
2. Windows 10 32-bit
3. Linux 64-bit

Excutables built on WIndows 10 32-bit may run on Windows 10 64-bit. Windows 7 or 8 support would require a native build.

### Getting Running
- In Anaconda Prompt  
  - `cd pabv_control/python_client`  
  - `python client_dual.py`  

# Arduino Wireing
test

# Useage
