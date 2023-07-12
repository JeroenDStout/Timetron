import datetime
import subprocess
import sys

# File out
output_path_cpp = sys.argv[1]
output_path_txt = sys.argv[2]

# The compile stamp is the date of compilation and the compiler(s) used
compileElements = [ [ "C++", sys.argv[3] ],
                    [ "Python", ".".join([ str(i) for i in sys.version_info]) ]
                  ]
timestamp       = datetime.datetime.now(datetime.timezone.utc).strftime("%d/%m/%Y, %H:%M UTC")

stampPrefixLen  = max( [ len(elem[0]) for elem in compileElements ] )
compileStamp    = (   [ f"Compiled on {timestamp}" ]
                    + [ f"{elem[0].ljust(stampPrefixLen)} : {elem[1]}" for elem in compileElements ] )

# The essential version is this commit relative to the last tag
stream = subprocess.check_output('git describe --tags'.split(' '))
essentialVersion = stream.decode(sys.stdout.encoding).strip()

print()
print("  Version: " + essentialVersion)
print("  " + "\n  ".join(compileStamp))
print()

# Get a shortened list of the last 100 commits
stream = subprocess.check_output('git log -50 --pretty="%h$$%(describe)$$%cd$$%D$$%s" --decorate --date=format:%y-%m-%d'.split(' '))
detailedVersionRaw = stream.decode(sys.stdout.encoding)

# Put all relevant git commits in a list
gitVersionData = []

fullDetailLines    = 10
versionDetailLines = 10
anyTagsSeen        = False

longestBranchVersionPrefix = 0
longestBranchTagPrefix     = 0

allLines = [ list(map(str.strip, s[1:-1].split("$$"))) for s in detailedVersionRaw.splitlines() ]
for (gid, desc, date, info, msg) in allLines:
  # Extract information
  branchInfo = list(filter(None, map(str.strip, info.split(','))))
  tagInfo    = [ x[5:] for x in branchInfo if x.startswith('tag: ') ]
  branchInfo = [ x.replace('HEAD ->', '').strip()
                 for x in branchInfo if     not x.startswith('origin')
                                        and not x.startswith('tag:')
                                        and not x == 'release'
               ]
  
  # If we are out of detail lines, skip if there is no important information
  fullDetailLines -= 1
  if fullDetailLines < 0:
    if len(tagInfo) == 0 and len(branchInfo) == 0:
      continue
    versionDetailLines -= 1
      
  # If we are out of version lines and we have seen a tag, break out
  if versionDetailLines < 0:
    if not anyTagsSeen and len(tagInfo) == 0:
      break
      
  # Extract goas specific branch names
  goasInfo   = [ x.split('--')[0].upper() for x in branchInfo if ('--' in x) ]
  branchInfo = [ x for x in branchInfo if not ('--' in x) ]
  
  # Extract branch and version prefixes
  branchPrefix  = ' '.join(tagInfo + branchInfo + goasInfo)
  versionPrefix = desc.split('-')
  versionPrefix = '' if len(versionPrefix) < 2 else versionPrefix[-2]
  
  # Add data
  longestBranchVersionPrefix = max(longestBranchVersionPrefix, len(versionPrefix))
  longestBranchTagPrefix     = max(longestBranchTagPrefix, len(branchPrefix))
  gitVersionData.append([ gid, versionPrefix, branchPrefix, f"{date} {msg}" ])
  
  # Have we seen a tag?
  anyTagsSeen = len(tagInfo) > 0
  
# Turn into git timestamps
gitstamp = [ f"{d[0]} {d[1].rjust(longestBranchVersionPrefix)} {d[2].ljust(longestBranchTagPrefix)} {d[3]}" for d in gitVersionData ]

with open(output_path_cpp, 'w') as f:
  f.write( "/***************************************************************\n")
  f.write( " *                                                             *\n")
  f.write( " *            This file was automatically generated            *\n")
  f.write( " *                Please do not edit by hand                   *\n")
  f.write( " *                                                             *\n")
  f.write( " ***************************************************************/\n")
  f.write( "\n\n")
  f.write( "namespace gaos::version {\n")
  f.write( "\n\n")
  f.write( "    constexpr char const * get_compile_stamp() {\n")
  f.write( "        return (\n          \""
    + "\\n\"\n          \"".join(compileStamp) + "\"\n")
  f.write( "        );\n")
  f.write( "    }\n")
  f.write( "\n\n")
  f.write( "    constexpr char const * get_git_essential_version() {\n")
  f.write(f"        return (\n          \"{essentialVersion}\"\n")
  f.write( "        );\n")
  f.write( "    }\n")
  f.write( "\n\n")
  f.write( "    constexpr char const * get_git_history() {\n")
  f.write( "        return (\n          \""
    + "\\n\"\n          \"".join(gitstamp) + "\"\n")
  f.write( "        );\n")
  f.write( "    }\n")
  f.write( "\n\n")
  f.write( "}\n")

with open(output_path_txt, 'w') as f:
  f.write("\n".join(compileStamp) + "\n")
  f.write("\n")
  f.write("\n".join(gitstamp) + "\n")