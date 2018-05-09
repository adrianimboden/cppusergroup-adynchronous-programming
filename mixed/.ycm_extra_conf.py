import os
import json
import shlex


def DirectoryOfThisScript():
    return os.path.dirname(os.path.abspath(__file__))


build_dir = os.environ['BUILD_DIR']

database_path = os.path.join(build_dir, 'compile_commands.json')
database = json.load(open(database_path))

SOURCE_EXTENSIONS = ['.cpp', '.cxx', '.cc', '.c', '.m', '.mm']


def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
    print(flags)
    new_flags = []
    for flag in flags:
        if flag.startswith('-I'):
            path = flag[2:]
            if path.startswith('/'):
                new_flags.append('-I' + path)
            else:
                new_flags.append('-I' + os.path.join(working_directory, path))
        else:
            new_flags.append(flag)
    #new_flags.append('-stdlib=libc++')
    #new_flags.append('-I/usr/include/c++/v1/')
    #new_flags.append('-fexceptions')
    new_flags.append('-x')
    new_flags.append('c++')
    new_flags.append('-Wno-deprecated') #for header files
    return new_flags


def IsHeaderFile(filename):
    extension = os.path.splitext(filename)[1]
    return extension in ['.h', '.hxx', '.hpp', '.hh']


def get_compilation_info(filename):
    entries = [entry for entry in database if entry['file'] == filename]
    if not entries:
        return None

    print(entries[0]['command'])
    return {'flags': shlex.split(entries[0]['command'])[1:], 'dir': entries[0]['directory']}


def GetCompilationInfoForFile(filename):
    info = get_compilation_info(filename)
    if info is not None:
        return info

    if IsHeaderFile(filename):
        basename = os.path.splitext(filename)[0]
        for extension in SOURCE_EXTENSIONS:
            replacement_file = basename + extension
            if os.path.exists(replacement_file):
                info = get_compilation_info(replacement_file)
                if info is not None:
                    return info

    for (root, dir, files) in os.walk(os.path.dirname(filename)):
        for file in files:
            if file.endswith('.cpp'):
                info = get_compilation_info(os.path.join(root, file))
                if info is not None:
                    return info

    info = get_compilation_info('/home/thingdust/src/backend/src/app/main.cpp')
    if info is not None:
        return info

    return None


def FlagsForFile(filename, **kwargs):
    compilation_info = GetCompilationInfoForFile(filename)
    if compilation_info is None:
        return None

    final_flags = MakeRelativePathsInFlagsAbsolute(compilation_info['flags'], compilation_info['dir'])

    return {'flags': final_flags, 'do_cache': True}
