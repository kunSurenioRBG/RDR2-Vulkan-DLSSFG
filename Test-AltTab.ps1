param(
 [string]$ZigPath=(Join-Path $PSScriptRoot '.deps/toolchain/zig.exe'),
 [string]$DependencyRoot=(Join-Path $PSScriptRoot '.deps')
)
$ErrorActionPreference='Stop'
$build=Join-Path $DependencyRoot 'rdr2-alt-tab-checks'
New-Item -ItemType Directory -Force $build | Out-Null
$flags=@('c++','-target','x86_64-windows-gnu','-std=c++20','-O2','-UNDEBUG','-DVK_NO_PROTOTYPES','-DNOMINMAX','-DWIN32_LEAN_AND_MEAN','-Wno-nullability-completeness',
 "-I$(Join-Path $DependencyRoot 'Vulkan-Headers/include')","-I$(Join-Path $DependencyRoot 'Streamline-2.14.1/include')","-I$(Join-Path $PSScriptRoot 'src')")
foreach($name in @('bridge_focus','input_layout','message_input','contracts','xess_history','bridge_recreation','fps_meter','bridge_recovery')){
 $binary=Join-Path $build "$name.exe"
 & $ZigPath @flags (Join-Path $PSScriptRoot "tests/$name.cpp") -o $binary -luser32 -ldxgi -ld3d12
 if($LASTEXITCODE){throw "Check compilation failed: $name"}
 & $binary
 if($LASTEXITCODE){throw "Check failed: $name"}
}
# Both baselines must reproduce the old failure without crashing a fixture.
foreach($name in @('input_layout','xess_lifecycle')){
 $define=if($name -eq 'input_layout'){'-DFGVK_BASELINE_LAYOUT'}else{'-DFGVK_BASELINE_FOCUS'}
 $binary=Join-Path $build "$name-before.exe"
 $extra=@();if($name -eq 'xess_lifecycle'){$extra+=Join-Path $PSScriptRoot 'vendor/xemfg-reference/host_unlock.cpp'}
 & $ZigPath @flags $define (Join-Path $PSScriptRoot "tests/$name.cpp") @extra -o $binary -luser32 -ldxgi -ld3d12 -lversion
 if($LASTEXITCODE){throw "Baseline compilation failed: $name"}
 & $binary
 if($LASTEXITCODE -ne 1){throw "Baseline did not reproduce the expected regression: $name"}
 Write-Output "Expected baseline failure reproduced: $name"
}
# Reproduce the previous queue-routing behavior without a crashing assertion.
# Only the new host-message guard is removed from this isolated header copy.
$routingBefore=Join-Path $build 'input-routing-before'
New-Item -ItemType Directory -Force $routingBefore | Out-Null
$routingHeader=[IO.File]::ReadAllText((Join-Path $PSScriptRoot 'src/message_input.h'))
if(!$routingHeader.Contains('consumed && CanConsume(msg->message)')){throw 'Routing baseline witness changed.'}
$routingHeader=$routingHeader.Replace('consumed && CanConsume(msg->message)','consumed')
[IO.File]::WriteAllText((Join-Path $routingBefore 'message_input.h'),$routingHeader,[Text.UTF8Encoding]::new($false))
$routingFlags=@('c++',"-I$routingBefore")+$flags[1..($flags.Count-1)]
$routingBinary=Join-Path $build 'message-input-before.exe'
& $ZigPath @routingFlags (Join-Path $PSScriptRoot 'tests/message_input.cpp') -o $routingBinary -luser32 -ldxgi -ld3d12
if($LASTEXITCODE){throw 'Routing baseline compilation failed.'}
& $routingBinary
if($LASTEXITCODE -ne 1){throw 'The old routing path did not reproduce host-input loss.'}
Write-Output 'Expected baseline failure reproduced: host raw-input/release preservation'
$global:LASTEXITCODE=0
