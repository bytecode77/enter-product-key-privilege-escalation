# Enter Product Key Volatile Environment LPE

| Exploit Information |                                   |
|:------------------- |:--------------------------------- |
| Date                | 21.10.2017                        |
| Patched             | Windows 10 RS3 (16299)            |
| Tested on           | Windows 10, x86 and x64           |

## Description

Windows 10 introduces the brand new "Settings" app, which provides us with fresh and new auto-elevated binaries. The entry point of this app is SystemSettings.exe and the "Enter Product Key" dialog is in SystemSettingsAdminFlows.exe, which is auto-elevated and vulnerable to environment variable injection.

A load attempt to %SYSTEMROOT%\System32\msctf.dll will be performed from this auto-elevated process.

How to change %systemroot%?
Simple: Through Volatile Environment.
Define your own %systemroot% in HKEY_CURRENT_USER\Volatile Environment and SystemSettingsAdminFlows.exe will load msctf.dll from there.

We need to execute SystemSettingsAdminFlows.exe, which is dependent on SystemSettings.exe running. So, we first start the Settings app, then inject the environment variable and then execute SystemSettingsAdminFlows.exe with the "EnterProductKey" argument. The second executable will load our DLL.

For PoC, this is sufficient. However, this is a UI heavy PoC that causes visual disturbances to the user. There are plenty of ways to mitigate visibility and possibly avoid SystemSettings.exe altogether. However, I will not investigate further than what is necessary to provide a functioning PoC, because this is the sixth auto-elevation exploit that I've discovered and I'm focusing on different techniques meanwhile posting anything I come across, like this here.

## Expected Result

When everything worked correctly, Payload.exe should be executed, displaying basic information including integrity level.

![](https://bytecode77.com/images/pages/enter-product-key-privilege-escalation/result.png)

## Downloads

Compiled binaries with example payload:

[![](http://bytecode77.com/public/fileicons/zip.png) EnterProductKeyVolatileEnvironmentLPE.zip](https://downloads.bytecode77.com/EnterProductKeyVolatileEnvironmentLPE.zip)
(**ZIP Password:** bytecode77)

## Project Page

[![](https://bytecode77.com/public/favicon16.png) bytecode77.com/enter-product-key-privilege-escalation](https://bytecode77.com/enter-product-key-privilege-escalation)