#!/usr/bin/python3

import os
import subprocess
import shlex

DIR = os.path.dirname(os.path.realpath(__file__))


def do_build(name, build_only=False):
    print('build and test {}'.format(name))
    target = os.path.join('/home/thingdust/workdir/build/', name)
    result = subprocess.call(['ninja'], cwd=target)
    if result == 0 and not build_only:
        subprocess.call(
            [
                os.path.join(target, 'app_test'),
                '--gtest_throw_on_failure',
                '--gtest_catch_exceptions=0',
            ],
            cwd=target)


do_build('clangtidy', True)
do_build('debug')
do_build('release')
