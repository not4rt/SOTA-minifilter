# SOTA-Minifilter Driver

A C-based minifilter driver for State of the Art (SOTA) group.

## External Code

Files that were not written by me:
Utilizes "uthash.h" and "utlist.h" from [troydhanson/uthash](https://github.com/troydhanson/uthash) **with the necessary kernel-space modifications made by be**.

## Description

This minifilter tries to identify malicious group of processes by validating rules against all IRP sent to the filesystem.

## What is a minifilter driver?
Minifilter or any other File System Filter Driver intercepts and modifies requests directed at a file system or another filter driver. By intercepting these requests before they reach their intended destination, the filter driver can enhance or replace the functionality provided by the original target. These services are accessible via the Windows Filter Manager.

Example macro architecture of file system drivers:
![filter-manager-architecture-1](https://github.com/not4rt/SOTA-minifilter/assets/128330203/b55870ce-580a-4734-b639-60bb3b7b8e26)
