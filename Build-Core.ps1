param(
 [string]$ZigPath=(Join-Path $PSScriptRoot '.deps\toolchain\zig.exe'),
 [string]$DependencyRoot=(Join-Path $PSScriptRoot '.deps')
)
$ErrorActionPreference='Stop'
$coreRoot=Join-Path $PSScriptRoot 'vendor\ampere-core'
$streamline=Join-Path $DependencyRoot 'Streamline-2.14.1'
if(!(Test-Path -LiteralPath $ZigPath)){throw 'Pass -ZigPath pointing to Zig 0.16.0.'}
if(!(Test-Path -LiteralPath (Join-Path $streamline 'include\sl.h'))){throw 'Run Build.ps1 first to obtain the pinned Streamline dependency.'}
$build=Join-Path $DependencyRoot 'rdr2-fg-custom-core-build'
New-Item -ItemType Directory -Force $build | Out-Null
$common=@('-target','x86_64-windows-gnu','-O2','-Wno-nullability-completeness','-DUNICODE','-D_UNICODE','-DNOMINMAX','-DWIN32_LEAN_AND_MEAN',"-ffile-prefix-map=$PSScriptRoot=.","-ffile-prefix-map=$DependencyRoot=dependencies")
& $ZigPath c++ @common -std=c++20 -UNDEBUG (Join-Path $PSScriptRoot 'tests\adapter_identity.cpp') -o (Join-Path $build 'adapter_identity.exe')
if($LASTEXITCODE){throw 'Adapter policy test compilation failed'}
& (Join-Path $build 'adapter_identity.exe')
if($LASTEXITCODE){throw 'Adapter policy test failed'}
$objects=@()
foreach($name in @('buffer','hook','trampoline','hde/hde64')){
 $object=Join-Path $build (($name.Replace('/','_'))+'.obj')
 & $ZigPath cc @common -c (Join-Path $coreRoot "third_party\minhook\src\$name.c") -o $object
 if($LASTEXITCODE){throw "MinHook compilation failed: $name"}
 $objects+=$object
}
$source=@('patcher','preset_runtime','midpoint_fix','ampere_policy','ampere_bundle','sm75_ptx','turing_runtime','ampere_experiment','flip_metering','dlssg_provider_policy','entry_detour','nvidia_mfg_policy','temporal_interval_trace') | ForEach-Object {Join-Path $coreRoot "$_.cpp"}
$binary=Join-Path $build 'RTX40MFGCore.dll'
& $ZigPath c++ @common -std=c++20 -fms-extensions -shared -Wno-deprecated-declarations -DMFG_UNLOCK_UNIVERSAL_CONFIG=1 "-I$streamline\include" "-I$streamline\external\ngx-sdk\include" "-I$coreRoot\third_party\minhook\include" @source @objects (Join-Path $coreRoot 'entry_detour.S') (Join-Path $coreRoot 'exports.def') -lbcrypt -lversion -ldxgi -o $binary
if($LASTEXITCODE){throw 'Core compilation failed'}
Get-FileHash -LiteralPath $binary -Algorithm SHA256 | Select-Object Hash,Path
