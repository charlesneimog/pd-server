# - name: look for libcrypto-1_1-x64.dll inside C:\msys64\ recursively

import os
import sys

def find_file(name, path):
    for root, dirs, files in os.walk(path):
        if name in files:
            return os.path.join(root, name)
        
    
if __name__ == '__main__':
    print(find_file('libcrypto-1_1-x64.dll', 'C:\\msys64\\'))