# SOTA-Minifilter Driver

A C-based minifilter driver for State of the Art (SOTA) group.

## External Code

Files that were not written by me:

"uthash.h" and "utlist.h" from [troydhanson/uthash](https://github.com/troydhanson/uthash) **with the necessary kernel-space modifications made by be**.

## Description

This minifilter tries to identify malicious group of processes by validating rules against all IRP sent to the filesystem.

## What is a minifilter driver?

Minifilter or any other File System Filter Driver intercepts and modifies requests directed at a file system or another filter driver. By intercepting these requests before they reach their intended destination, the filter driver can enhance or replace the functionality provided by the original target. These services are accessible via the Windows Filter Manager.

Example macro architecture of file system drivers:

![filter-manager-architecture-1](https://github.com/not4rt/SOTA-minifilter/assets/128330203/b55870ce-580a-4734-b639-60bb3b7b8e26)

## Build
### Pre-requisites to build
| Name | Reason |
| ---------------------------------------------- | -- |
| [**Microsoft Visual Studio Build Tools for C++**](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16) | To develop and build C++ code, install "Desktop development with C++ |
| [**Windows SDK**](https://go.microsoft.com/fwlink/?linkid=2166460) | WDK needs a matching version of Windows SDK. |
| [**Windows Driver Kit (WDK)**](https://go.microsoft.com/fwlink/?linkid=2166289) | The base kit to develop a driver |
| (optional) [**LLVM and Clang**](https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/LLVM-12.0.1-win64.exe) | In the rust version of the driver, LLVM is needed to generate bindings for the Windows Driver API (bindgen) |

### Steps
1. Open the "MinifilterSOTA.sln" with [Visual Studio](https://visualstudio.microsoft.com/) and click "Build Solution".
2. The default output is "x64\Debug\MinifilterSOTA" directory.

   
## Instalation
### Pre-requisites to install and use it
| Name | Reason |
| ---------------------------------------------- | -- |
| [**Disable Secure Boot**](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/the-testsigning-boot-configuration-option) | To make Microsoft Windows accepts self-signed drivers |
| [**Enable Driver Testing Mode**](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/the-testsigning-boot-configuration-option) | To make Microsoft Windows accepts self-signed drivers |
| (optional) [**Sysinternals DebugView**](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview) | DebugView to see kernel debug messages |

### Steps
1. Deploy the 3 files from the release page (or the files you built) in the target machine.
2. Right-click on "MinifilterSOTA.inf" and select "Install". Alternatively you can run "pnputil -i -a "<path>/MinifilterSOTA.inf" on power shell.
3. Reboot

