#!/usr/bin/env python3
# extract_lst.py
import sys
import re
import glob

lineRegex = re.compile(r"""^00:([0-9A-F]{8}) ([0-9A-F]+)\s+(\d+):\s*(.*)$""")

def readListFile(inFile, outFile):
    try:
        while True:
            line = next(inFile).rstrip('\r\n')

            match = lineRegex.fullmatch(line)
            if match:
                outFile.write(f'{match.group(2)}\t{match.group(4)}\n')
    except StopIteration:
        return None

def main(argv):
    extension = 'o.lst'
    destExtension = 'tst'
    path = './*.' + extension
    targetPath = '../data/'
    lstFiles = glob.glob(path)
    for lstFile in lstFiles:
        dest = targetPath + lstFile.removesuffix(extension).removeprefix('.\\') + destExtension
        print(lstFile, ' => ', dest)
        with open(lstFile, 'r') as inFile, open(dest, 'w') as outFile:
            readListFile(inFile, outFile)


if __name__ == '__main__':
    MIN_PYTHON = (3, 6)
    if sys.version_info < MIN_PYTHON:
        sys.exit("Python %s.%s or later is required.\n" % MIN_PYTHON)
    main(sys.argv)