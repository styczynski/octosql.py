from ctypes import *
import os
import subprocess
import platform

__LIB__ = None

__lib_versions__ = {
    "windows": "-4.0-",
    "linux": "",
    "darwin": "-10.6-"
};

__lib_exts__ = {
    "windows": ".dll",
    "linux": ".so",
    "darwin": ".dylib"
};


def __get_platform_binary_name__():
    global __lib_versions__
    global __lib_exts__

    system = platform.system()
    normalized_system_name = None
    if system == "Windows":
        normalized_system_name = "windows"
    elif system == "Linux":
        normalized_system_name = "linux"
    elif system == "Darwin":
        normalized_system_name = "darwin"

    arch = platform.architecture()
    normalized_arch_name = None
    if arch[0] == "64bit":
        normalized_arch_name = "amd64"
    elif arch[0] == "32bit":
        normalized_arch_name = "386"

    if normalized_system_name != None and normalized_arch_name != None:
        normalized_version = ""
        if normalized_system_name in __lib_versions__:
            normalized_version = __lib_versions__[normalized_system_name]
        normalized_ext = ""
        if normalized_system_name in __lib_exts__:
            normalized_ext = __lib_exts__[normalized_system_name]
        return "src-" + normalized_system_name + normalized_version + normalized_arch_name + normalized_ext

    return "local_build"

class OctoSQL:
    def __init__(self):
        global __LIB__
        if __LIB__ is None:
            base_path = os.path.dirname(os.path.abspath(__file__))
            libs_path = os.path.abspath(base_path + "/../../libs")
            lib_to_load = os.path.abspath(libs_path + "/" + __get_platform_binary_name__())

            try:
                __LIB__ = cdll.LoadLibrary(lib_to_load)
                __LIB__.test()
            except:
                try:
                    build_cmd = 'cd ' + libs_path + ' && go build -o local_build -buildmode=c-shared ../src/lib.go'
                    subprocess.check_call(build_cmd, shell=True, executable='/bin/bash')
                    lib_to_load = os.path.abspath(libs_path + "/local_build")
                    __LIB__ = cdll.LoadLibrary(lib_to_load)
                    __LIB__.test()
                except:

                    go_ver_info = "There is no golang installed. Please install GO executable."
                    go_version_check_command = "go version"
                    try:
                        subprocess.check_call(go_version_check_command, shell=True, executable='/bin/bash')
                        go_ver_info = ""
                    except:
                        go_ver_info = go_ver_info + "\n"

                    print(go_ver_info)
                    print("Failed to load binary library.")
                    print("  Platform: " + platform.system())
                    print("  Arch:     " + platform.architecture()[0])
                    print("  Binary:   " + __get_platform_binary_name__())
                    print("")
                    print("Try to run \"" + build_cmd + "\" manually.")

                    raise Exception("Failed to load binary library for octosql.py")

        self.lib = __LIB__

    def lolxd(self):
        self.lib.lolxd()