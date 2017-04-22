import os
import sys
import re
import hashlib
import shutil

def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

solution_dir = os.path.abspath(os.path.join(__file__, '..', '..'))

conf = "Debug" if "debug" in sys.argv[1] else "Release"

from_dir = os.path.join(solution_dir, "x64", conf + "Dll")
from_filename = "SimHair.dll"
from_file_md5 = md5(os.path.join(from_dir, from_filename))

print("Copy from: %s, md5: %s" % (os.path.join(from_dir, from_filename), from_file_md5))

to_dir = os.path.join(solution_dir, "HairUnity", "Assets", "Plugins")

def suffix_from_index(idx):
    return "" if idx == 0 else "%05d" % idx

def index_from_suffix(suffix):
    return 0 if suffix == "" else int(suffix)

max_idx = -1
for filename in os.listdir(to_dir):
    r = re.match(r"SimHair(.*)\.dll", filename)
    if (r):
        if md5(os.path.join(to_dir, filename)) == from_file_md5:
            print("Same md5 in", os.path.join(to_dir, filename))
            sys.exit(0)
        max_idx = max(max_idx, index_from_suffix(r.group(1)))

to_filename = "SimHair" + suffix_from_index(max_idx + 1) + ".dll"

print("Copy file %s to %s..." % (os.path.join(from_dir, from_filename), os.path.join(to_dir, to_filename)))
shutil.copyfile(os.path.join(from_dir, from_filename), os.path.join(to_dir, to_filename))

script_path = os.path.join(solution_dir, "HairUnity", "Assets", "Scripts", "HairEngine", "HairEngineMethod.cs")

with open(script_path, "r") as f:
    pattern = re.compile("SimHair([0-9]*)(\.dll)*")
    replace_str = to_filename[:to_filename.rfind('.dll')]
    lines = [re.sub(pattern, replace_str, line) for line in f]
    print("\n".join(map(lambda x: str(x[0]) + ' ' + str(x[1].strip()), filter(lambda x: replace_str in x[1], enumerate(lines)))))

with open(script_path, "w") as f:
    f.writelines(lines)
