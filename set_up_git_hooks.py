print("Writing files...")

for file in [ "update", "post-checkout", "post-commit", "post-merge", "post-rebase" ]:
  with open('.git/hooks/' + file, 'w') as f:
    print('* ./git/hooks/' + file)
    f.write('#!/bin/bash\n')
    f.write('touch ./touch')

input("... done")