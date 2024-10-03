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

## Known issues
1. After editing and saving a file, status is not updated as it is overwritten by the SDK
