# Me:      "Can we use Jenkins"
# Also me: "We have Jenkins at home"
#
# The Jenkins at home:
print("Jerkins file packer - 'its free gratis!'");

import os
import pathlib
import sys
import zipfile

# Get arguments

mode      = "Release"
config    = "Release"
info_file = ""
out       = ""

for arg in sys.argv[1:]:
  if not ":" in arg:
    print("Bad argument: " + arg)
    
  (command, value) = arg.split(":", 1)
  
  if command == "mode":
    mode = value
  elif command == "config":
    config = value
  elif command == "info":
    info_file = os.path.realpath(value)
  elif command == "out":
    out = os.path.realpath(value)
    
bin_folder = os.path.realpath(os.path.dirname(info_file) + "/bin/" + config)
    
print("Mode : " + mode);
print("Info : " + info_file);
print("Bin  : " + bin_folder);
print("Out  : " + out);
    
# Read file

executables  = []
dependencies = []
misc_files   = []

acceptable_executable_formats = [ ".exe", ".dll" ]
if mode == "Debug":
  acceptable_executable_formats += [ ".pdb" ]

with open(info_file, 'r') as f:
  info_data = f.readlines()
  
for line in map(str.strip, info_data):
  if len(line) == 0:
    continue

  if not ":" in line:
    print("Bad argument: " + line)
    continue
    
  (command, value) = map(str.strip, line.split(":", 1))
  
  if command == "executable":
    executables.append(value)
  elif command == "dependency":
    dependencies.append(value)
  elif command == "file":
    if not "->" in value:
      print("Bad argument: " + value)
      continue
    (src, dest) = map(str.strip, value.split("->", 1))
    misc_files.append([ src, dest ])
    
# Process

pathlib.Path(os.path.dirname(out)).mkdir(parents=True, exist_ok=True)

print("Writing to " + out)
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zipf:
  # Add executables
  for root, dirs, files in os.walk(bin_folder):
    for file in files:
      suffixes = ''.join(pathlib.Path(file).suffixes)
      stem     = file[:-len(suffixes)]
    
      if suffixes not in acceptable_executable_formats:
        continue
      if stem not in executables:
        continue
      zipf.write(os.path.join(root, file), file)
      
  if "qt6" in dependencies:
    for qt_folder in [ "iconengines", "imageformats", "networkinformation", "platforms", "styles", "tls", "translations" ]:
      for root, dirs, files in os.walk(bin_folder + "/" + qt_folder):
        for file in files:
          zipf.write(os.path.join(root, file), os.path.relpath(os.path.join(root, file), bin_folder))
          
    for file in os.listdir(bin_folder):
      path = os.path.join(bin_folder, file)
    
      if not os.path.isfile(path):
        continue
        
      suffixes = ''.join(pathlib.Path(path).suffixes)
      if suffixes != ".dll":
        continue
        
      if not file.startswith("Qt6") and not file.startswith("D3Dcompiler") and not file.startswith("opengl32"):
        continue
        
      zipf.write(path, file)
      
  for file in misc_files:
    zipf.write(file[0], file[1])

print("Done")