import shutil

new_src = '..\\Release\\FrogFish.dll'
new_dbg_src = '..\\Debug\\FrogFish.dll'
new_dest = 'C:\\ProgramData\\bwapi\\starcraft\\bwapi-data\\AI\\FrogFish.dll'
new_dbg_dest = 'C:\\ProgramData\\bwapi\\starcraft\\bwapi-data\\AI\\FrogFish_dbg.dll'

shutil.copyfile(new_src, new_dest)
shutil.copyfile(new_dbg_src, new_dbg_dest)

