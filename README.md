# test_context_switch

This repo includes two simple test senarios(on two directory) to test the new opensbi domain
context switch feature:
 - [bare-metal multi-core app](https://github.com/Shang-QY/test_context_switch/blob/master/bare_metal_test/README.md)
 - [linux multi-core app](https://github.com/Shang-QY/test_context_switch/blob/master/linux_test/README.md)

It can be compiled and generate two domain payload:
s-hello.bin and {ns-hello.bin / linux}, corresponding to secure domain and non-secure
domain.

When events happen (secure service requirements, in this test), harts can
switch between non-secure and secure domain. Each domain has hart contexts
as many as platform hart count (2 harts in this test), which can be switched
in (running) or out (suspended) simultaneously.

## Test Output

### bare-metal multi-core app:

![image](https://github.com/Shang-QY/test_context_switch/assets/55442231/bf18d960-b4c4-490a-8643-7a8dea3ca354)

### linux multi-core app:

![image](https://github.com/Shang-QY/test_context_switch/assets/55442231/b91221f6-1c17-43bf-a173-30c6da3a4f80)

## Test Setup

### Select a workspace and Download this project
```
export WORKDIR=`pwd`
git clone https://github.com/Shang-QY/test_context_switch.git
```

### Follow actions for two different senario tests:

 - [bare-metal multi-core app](https://github.com/Shang-QY/test_context_switch/tree/master/bare_metal_test#test-setup)
 - [linux multi-core app](https://github.com/Shang-QY/test_context_switch/tree/master/linux_test#test-setup)
