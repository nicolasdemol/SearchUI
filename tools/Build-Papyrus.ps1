param(
    [Parameter(Mandatory = $true)][string]$GamePath,
    [Parameter(Mandatory = $true)][string[]]$ImportPaths,
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\build\papyrus')
)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $GamePath 'Papyrus Compiler\PapyrusCompiler.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw "Papyrus compiler not found: $compiler" }
$source = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\Scripts\Source')).Path
$imports = @($source) + @($ImportPaths | ForEach-Object { (Resolve-Path -LiteralPath $_).Path })
if (!($imports | Where-Object { Test-Path -LiteralPath (Join-Path $_ 'TESV_Papyrus_Flags.flg') })) {
    throw 'ImportPaths must include TESV_Papyrus_Flags.flg.'
}
$output = (New-Item -ItemType Directory -Force -Path $OutputPath).FullName
foreach ($script in 'SearchAPI', 'SearchUIController', 'SearchMCM') {
    & $compiler (Join-Path $source "$script.psc") "-i=$($imports -join ';')" "-o=$output" '-f=TESV_Papyrus_Flags.flg' '-op'
    if ($LASTEXITCODE -ne 0) { throw "Papyrus compilation failed: $script ($LASTEXITCODE)" }
}
