#!/usr/bin/python3

import os
import subprocess
import shlex


def prep_build(name, flavor, additional_cmake_flags=[]):
    target = os.path.join('/home/thingdust/workdir/build/', name)
    if not os.path.isdir(target):
        os.makedirs(target)
    flags = ""
    with open('/prebuilt/inst/{}/cmake_flags'.format(flavor), 'r') as f:
        flags = f.read()
    splitted_flags = shlex.split(flags)

    subprocess.call(
        ['cmake'] + splitted_flags + additional_cmake_flags +
        ['-DCMAKE_LINK_DEPENDS_NO_SHARED=1',
         os.path.join(os.getcwd(), 'src')],
        cwd=target)


prep_build('debug', 'debug')
prep_build('release', 'release')
prep_build('clangtidy', 'debug', ['-DUSE_CLANG_TIDY=YES'])
