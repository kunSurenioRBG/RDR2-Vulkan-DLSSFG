param(
 [string]$ZigPath=(Join-Path $PSScriptRoot '.deps\toolchain\zig.exe'),
 [string]$DependencyRoot=(Join-Path $PSScriptRoot '.deps')
)
$ErrorActionPreference='Stop'
$build=Join-Path $DependencyRoot 'rdr2-custom-checks'
New-Item -ItemType Directory -Force $build | Out-Null
$flags=@('c++','-target','x86_64-windows-gnu','-std=c++20','-O2','-UNDEBUG','-Wno-nullability-completeness','-DNOMINMAX','-DWIN32_LEAN_AND_MEAN')
$flags+=@('-DVK_NO_PROTOTYPES',"-I$(Join-Path $DependencyRoot 'Vulkan-Headers/include')")
foreach($name in @('adapter_identity','xess_pacing','xess_lifecycle','bridge_focus','input_layout')){
 $binary=Join-Path $build "$name.exe"
 $extra=@();if($name -eq 'xess_lifecycle'){$extra+=Join-Path $PSScriptRoot 'vendor\xemfg-reference\host_unlock.cpp'}
 & $ZigPath @flags (Join-Path $PSScriptRoot "tests\$name.cpp") @extra -o $binary -ldxgi -ld3d12 -lversion
 if($LASTEXITCODE){throw "Check compilation failed: $name"}
 & $binary
 if($LASTEXITCODE){throw "Check failed: $name"}
}
