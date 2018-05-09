#!/usr/bin/python3

import subprocess
import os
from jinja2 import Template
import shlex
import shutil
from distutils.dir_util import copy_tree

default_cxx_flags = "-std=c++17 -fPIC -g -fno-omit-frame-pointer -fno-limit-debug-info -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -D_LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE"  # nopep8
default_c_flags = "-std=c11 -g -fPIC -fno-omit-frame-pointer -fno-limit-debug-info"  # nopep8
default_link_flags = "-g -fPIC -fno-omit-frame-pointer -fno-limit-debug-info -ldl"  # nopep8

os.environ['CC'] = 'clang'
os.environ['CXX'] = 'clang++'

def render_tpl(src, dst, variables):
    with open(src, 'rb') as src_f:
        with open(dst, 'wb') as dst_f:
            dst_f.write(
                Template(src_f.read().decode('utf-8')).render(variables)
                .encode('utf-8'))


basedir = '/prebuilt'
instdir = os.path.join(basedir, 'inst')
builddir = os.path.join(basedir, 'build')
srcdir = os.path.join(basedir, 'src')


def ensure_dir(dir):
    os.makedirs(dir, exist_ok=True)


ensure_dir(instdir)
ensure_dir(builddir)
ensure_dir(srcdir)


def clone_git(src, rev, dst):
    real_dst = os.path.abspath(os.path.join(srcdir, dst))
    subprocess.check_call(['git', 'clone', src, real_dst])
    prevcwd = os.getcwd()
    os.chdir(real_dst)
    subprocess.check_call(['git', 'reset', '--hard', rev])
    os.chdir(prevcwd)


def clone_hg(src, rev, dst):
    real_dst = os.path.abspath(os.path.join(srcdir, dst))
    subprocess.check_call(['hg', 'clone', '-r', rev, src, real_dst])

clone_hg('https://scm.svc.firenet.ch/thingdust/external/boost',
         '5d450b2dbc62be17fda0e46f5b9fc8f6e6178380', 'boost')

clone_git('https://scm.svc.firenet.ch/thingdust/external/googletest',
          '42bc671f47b122fad36db5eccbc06868afdf7862', 'googletest')

clone_git('https://scm.svc.firenet.ch/thingdust/external/GSL',
          '1f87ef73f1477e8adafa8b10ccee042897612a20', 'GSL')


def make_lib(name, flavor, options, cxx_flags, c_flags, ld_library_path):
    build_dir = os.path.join(builddir, flavor, name)
    ensure_dir(build_dir)

    os.chdir(build_dir)

    env = os.environ.copy()
    if ld_library_path:
        env['LD_LIBRARY_PATH'] = ld_library_path

    subprocess.check_call(
        [
            'cmake',
        ] + options + [
            os.path.join(srcdir, name),
        ], env=env)
    subprocess.check_call(['ninja', 'install'], env=env)


def make_boost(flavor, cxx_flags, c_flags, link_flags, boost_flags,
               ld_library_path):
    boost_dir = os.path.join(srcdir, 'boost')
    boost_bin_dir = os.path.join(boost_dir, 'bin.v2')
    if os.path.isdir(boost_bin_dir):
        shutil.rmtree(boost_bin_dir)

    env = os.environ.copy()
    if ld_library_path:
        env['LD_LIBRARY_PATH'] = ld_library_path
    subprocess.check_call(
        [
            './bootstrap.sh',
            '--prefix=' + os.path.join(instdir, flavor),
        ],
        cwd=boost_dir,
        env=env,
    )
    subprocess.check_call(
        [
            './b2', 'install', 'toolset=clang',
            'cxxflags={}'.format(cxx_flags), 'linkflags={}'.format(link_flags),
            'link=static', 'variant=release', 'threading=multi',
        ] + boost_flags + [
            '-sICU_PATH={}'.format(os.path.join(instdir, flavor)),
            '--with-thread',
            '--with-system',
            '--with-context',
            '-j4',
            '-q',  # fail early
            '-a',  # rebuild
        ],
        cwd=boost_dir,
        env=env,
    )


def make_icu(name, flavor, cxx_flags, c_flags, link_flags, ld_library_path):
    build_dir = os.path.join(builddir, flavor, name, 'source')
    shutil.copytree(
        os.path.join(srcdir, name),
        os.path.join(builddir, flavor, name),
    )

    env = os.environ.copy()
    env['CFLAGS'] = c_flags
    env['CXXFLAGS'] = cxx_flags
    env['LDFLAGS'] = link_flags
    env['ASAN_OPTIONS'] = 'detect_leaks=0'
    env['LSAN_OPTIONS'] = 'detect_leaks=0'
    env['VERBOSE'] = '1'
    if ld_library_path:
        env['LD_LIBRARY_PATH'] = ld_library_path
    subprocess.check_call(
        [
            './runConfigureICU',
            '--disable-release',
            'Linux',
            '--enable-static',
            '--disable-shared',
            '--prefix={}'.format(os.path.join(instdir, flavor)),
        ],
        cwd=build_dir,
        env=env)
    subprocess.check_call(['make', 'clean'], cwd=build_dir, env=env)
    subprocess.check_call(['make', '-j2'], cwd=build_dir, env=env)
    subprocess.check_call(['make', 'install'], cwd=build_dir, env=env)


def make_all(flavor, options, cxx_flags, c_flags, link_flags, boost_flags,
             ld_library_path):
    install_dir = os.path.join(instdir, flavor)
    ensure_dir(install_dir)
    ensure_dir(os.path.join(instdir, flavor, 'cmake_modules'))

    default_cmake_flags = [
        '-GNinja',
        '-DCMAKE_CXX_FLAGS={}'.format(default_cxx_flags),
        '-DCMAKE_C_FLAGS={}'.format(default_c_flags),
        '-DCMAKE_INSTALL_PREFIX={}'.format(install_dir),
        '-DCMAKE_MODULE_PATH={}'.format(
            os.path.join(install_dir, 'cmake_modules')),
    ]

    cmake_flags = [
        '-GNinja',
        '-DCMAKE_CXX_FLAGS={}'.format(cxx_flags),
        '-DCMAKE_C_FLAGS={}'.format(c_flags),
        '-DCMAKE_INSTALL_PREFIX={}'.format(install_dir),
        '-DCMAKE_MODULE_PATH={}'.format(
            os.path.join(install_dir, 'cmake_modules')),
    ] + options
    with open(os.path.join(instdir, flavor, 'cxx_flags'), 'w') as f:
        f.write(cxx_flags)

    with open(os.path.join(instdir, flavor, 'c_flags'), 'w') as f:
        f.write(c_flags)

    with open(os.path.join(instdir, flavor, 'link_flags'), 'w') as f:
        f.write(link_flags)

    with open(os.path.join(instdir, flavor, 'cmake_flags'), 'w') as f:
        f.write(' '.join([shlex.quote(flag) for flag in cmake_flags]))

    make_boost(
        flavor,
        cxx_flags,
        c_flags,
        link_flags,
        boost_flags,
        ld_library_path,
    )

    make_lib(
        'googletest',
        flavor,
        cmake_flags,
        cxx_flags,
        c_flags,
        ld_library_path,
    )



sanitize_address = "-fsanitize=address -fsanitize=undefined -fsanitize=leak -fsanitize-address-use-after-scope"  # nopep8


def make_flavor(flavor, build_type, base_compile_flags, base_link_flags,
                shared_lib_link_flags, cmake_flags, boost_flags,
                compiler_flavor):
    compile_flags = '{base} -Wno-unused-command-line-argument -nostdinc++ -isystem /usr/{flavor}/include/c++/v1/ -nostdlib++ -L/usr/{flavor}/lib -lc++'.format(
        base=base_compile_flags,
        flavor=compiler_flavor) if compiler_flavor else base_compile_flags
    link_flags = '{base} -Wl,-rpath,/usr/{flavor}/lib'.format(
        base=base_link_flags,
        flavor=compiler_flavor) if compiler_flavor else base_link_flags
    make_all(
        flavor,
        [
            '-DCMAKE_BUILD_TYPE={}'.format(build_type),
            '-DCMAKE_EXE_LINKER_FLAGS={} {}'.format(default_link_flags,
                                                    link_flags),
            '-DCMAKE_SHARED_LINKER_FLAGS={} {}'.format(default_link_flags,
                                                       shared_lib_link_flags),
            '-DCMAKE_MODULE_LINKER_FLAGS={} {}'.format(default_link_flags,
                                                       link_flags),
            '-D_CMAKE_TOOLCHAIN_PREFIX=llvm-',
        ] + cmake_flags,
        '{} {}'.format(default_cxx_flags, compile_flags),
        '{} {}'.format(default_c_flags, compile_flags),
        '{} {}'.format(default_link_flags, link_flags),
        boost_flags,
        '/usr/{}/lib'.format(compiler_flavor),
    )

make_flavor(
    'debug',
    'Debug',
    '-O0 -DSPDLOG_DEBUG_ON',
    '-O0',
    '-O0',
    [],
    [],
    None,
)

shutil.rmtree(builddir)
shutil.rmtree(os.path.join(srcdir, 'boost', 'bin.v2'))
shutil.rmtree(os.path.join(srcdir, 'boost', 'doc'))

repos = []
for root, dirs, files in os.walk(srcdir):
    repos += [
        os.path.join(root, name) for name in dirs
        if name == '.hg' or name == '.git'
    ]

repos.reverse()
for repo in repos:
    shutil.rmtree(repo)

test_code = []
for root, dirs, files in os.walk(os.path.join(srcdir)):
    test_code += [os.path.join(root, name) for name in dirs if name == 'test']

test_code.reverse()
for dir in test_code:
    shutil.rmtree(dir)
