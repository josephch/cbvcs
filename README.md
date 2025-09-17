# Code Blocks Version Control plugin ( CBVCS )
CBVCS integrates version control systems to CodeBlocks file manager. CBVCS supports only git at the moment.

## Git features supported
1. Status of project files is shown in the CB file manager
2. Git operations menu on right clicking file manager item(s).
   1. Add files
   2. Remove files
   3. Commit  
   4. Revert changes
   5. Diff
   6. Refresh status
## Dependencies
This fork of CBVCS uses libgit2 to do git operations. Details of installation and usage of libgit2 is avaliable [here]( https://libgit2.org/docs/guides/build-and-link/)

## Getting Started
### Getting the source
```
git clone https://github.com/josephch/cbvcs.git
```

### Build using cmake on unix
Make sure codeblocks.pc is present in default package config search paths or export PKG_CONFIG_PATH with its location

```
mkdir cmake-build
cd cmake-build
cmake ../
make
```
### Build using Code::Blocks
Build after opening project in Code::Blocks

### Installation
Install the plugin by clicking ```Install New``` in ```Plugins->Manage Plugin``` menu and selecting cbvcs.cbplugin. Reopen active project/ restart code::blocks.
