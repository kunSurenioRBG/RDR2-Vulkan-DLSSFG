param(
 [string]$ZigPath=(Join-Path $PSScriptRoot '.deps\toolchain\zig.exe'),
 [string]$DependencyRoot=(Join-Path $PSScriptRoot '.deps')
)
$ErrorActionPreference='Stop'
$projectDirectory=$PSScriptRoot
# Reuse the production source/dependency lists; do not enable crash fixtures.
. (Join-Path $projectDirectory 'Build.ps1') -ZigPath $ZigPath -DependencyRoot $DependencyRoot
$preview=Join-Path $buildRoot 'menu_snapshot.exe'
$extra=@((Join-Path $projectDirectory 'tools\menu_snapshot.cpp'),(Join-Path $projectDirectory 'vendor\imgui\imgui_impl_dx11.cpp'))
& $ZigPath @flags -UNDEBUG -DFGVK_MENU_PREVIEW @src @detours @extra (Join-Path $projectDirectory 'vendor\d3dcompiler.lib') -o $preview -lbcrypt -luser32 -ladvapi32 -ld3d11 -ld3d12 -ldxgi -lgdi32 -ldwmapi -lcomctl32 -limm32 -lwinhttp -lshell32 -lole32 -lversion -lwindowscodecs -luuid
if($LASTEXITCODE){throw 'Menu preview compilation failed'}
$images=Join-Path $projectDirectory 'docs\screenshots'
New-Item -ItemType Directory -Force $images | Out-Null
& $preview $images
if($LASTEXITCODE){throw 'Menu rendering failed'}
