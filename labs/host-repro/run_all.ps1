# Host reproductions for OPL parser bugs on malformed input (Windows, MSVC with AddressSanitizer).
#
#   powershell -ExecutionPolicy Bypass -File labs\host-repro\run_all.ps1 [-Opl <OPL checkout>]
#
# Each harness copies the functions under test out of the OPL checkout (extract.py), so the same
# run works on master and on a fix branch. Every test is built twice: plain (/GS-, so a small stack
# overrun does not abort) for the output, and with /fsanitize=address for memory errors.
# -Images <dir> also prints the game ID OPL derives from every .iso image in <dir>.
# -Only cheat,iso,find,zso,cnf,sb,name,crc,cfg runs only those groups.
param([string]$Opl = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path, [string]$Images = "", [string[]]$Only = @())
$ErrorActionPreference = "Stop"
$here = $PSScriptRoot
$build = Join-Path $here "build"
$inputs = Join-Path $build "in"
New-Item -ItemType Directory -Force $build | Out-Null

$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
$vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars64.bat"
$asanDir = (Get-ChildItem (Join-Path $vs "VC\Tools\MSVC") -Recurse -Filter "clang_rt.asan_dynamic-x86_64.dll" |
        Where-Object { $_.FullName -like "*Hostx64\x64*" } | Select-Object -First 1).DirectoryName
$env:PATH = "$asanDir;$env:PATH"
python -I (Join-Path $here "make_inputs.py") $inputs | Out-Null

function Want([string]$Group) { return ($Only.Count -eq 0) -or ($Only -contains $Group) }

function Build([string]$Harness, [string]$Tag, [string]$Source, [string[]]$Funcs, [string]$Source2 = "", [string[]]$Funcs2 = @(), [string]$Source3 = "", [string[]]$Funcs3 = @(), [string[]]$Patch = @(), [string]$Defines = "") {
    $text = ""
    if ($Source3) { $text += (python -I (Join-Path $here "extract.py") (Join-Path $Opl $Source3) @Funcs3 | Out-String) }
    if ($Source2) { $text += (python -I (Join-Path $here "extract.py") (Join-Path $Opl $Source2) @Funcs2 | Out-String) }
    $text += (python -I (Join-Path $here "extract.py") (Join-Path $Opl $Source) @Funcs | Out-String)
    foreach ($p in $Patch) {
        $a, $b = $p -split "=>", 2
        $text = $text.Replace($a, $b)
    }
    Set-Content -Encoding ascii (Join-Path $build ($Harness + "_extracted.c")) $text
    $src = Join-Path $here ($Harness + "_harness.c")
    $exe = Join-Path $build "$Harness-$Tag"
    $common = "/nologo /W3 /Od /Zi /wd4996 $Defines /I`"$build`" `"$src`""
    cmd /c "`"$vcvars`" >nul 2>nul && cl $common /GS- /Fe:`"$exe.exe`" /Fo:`"$exe.obj`" /Fd:`"$exe.pdb`" >`"$exe.log`" && cl $common /fsanitize=address /Fe:`"$exe-asan.exe`" /Fo:`"$exe-asan.obj`" /Fd:`"$exe-asan.pdb`" >>`"$exe.log`""
    if ($LASTEXITCODE -ne 0) { Get-Content "$exe.log"; throw "build of $Harness failed" }
}

function Run([string]$Exe, [string[]]$Arguments) {
    $out = Join-Path $build "last.out"
    $quoted = @($Arguments | ForEach-Object { if ($_ -match "\s") { "`"$_`"" } else { $_ } })
    $p = Start-Process -FilePath (Join-Path $build "$Exe.exe") -ArgumentList $quoted -NoNewWindow -PassThru -RedirectStandardOutput $out -RedirectStandardError "$out.err"
    $null = $p.Handle
    if (-not $p.WaitForExit(10000)) { $p.Kill(); return "TIMEOUT: endless loop" }
    $lines = @(Get-Content $out | Select-Object -Last 3)
    $asan = Get-Content "$out.err" | Select-String -Pattern "ERROR: AddressSanitizer: ([\w-]+)" | Select-Object -First 1
    $frame = Get-Content "$out.err" | Select-String -Pattern "#\d+ 0x\w+ in (\w+) .*(extracted|harness)" | Select-Object -First 1
    if ($asan) { return "ASAN " + $asan.Matches[0].Groups[1].Value + " in " + $frame.Matches[0].Groups[1].Value }
    if ($p.ExitCode -ne 0) { return ("CRASH 0x{0:X8}" -f $p.ExitCode) }
    return ($lines -join " | ")
}

function Test([string]$Name, [string]$Exe, [string[]]$Arguments) {
    "{0,-34} plain: {1}" -f $Name, (Run $Exe $Arguments)
    "{0,-34} asan:  {1}" -f "", (Run "$Exe-asan" $Arguments)
}

"OPL tree: $Opl"
if (Want "cheat") {
    $cheatFuncs = "make_code", "is_cheat_code", "parse_line", "is_cmt_str", "chr_idx", "term_str", "is_empty_str", "trim_str", "is_empty_substr", "parse_buf", "CHEAT_LINE_MAX"
    Build cheat t "src\cheatman.c" $cheatFuncs
    foreach ($t in "cht_plain", "cht_indented", "cht_251", "cht_long_last", "cht_no_lf") { Test "cheat $t" cheat-t (Join-Path $inputs "$t.cht") }
    if ((Get-Content -Raw (Join-Path $Opl "src\cheatman.c")).Contains("char digits[CODE_DIGITS];")) {
        "(make_code() overruns digits[] on every code; again with only that line fixed, to see the other bugs)"
        Build cheat iso "src\cheatman.c" $cheatFuncs -Patch "char digits[CODE_DIGITS];=>char digits[CODE_DIGITS + 1];"
        foreach ($t in "cht_251", "cht_long_last", "cht_no_lf") { Test "cheat $t (digits fixed)" cheat-iso (Join-Path $inputs "$t.cht") }
    }
}

if (Want "iso") {
    Build iso ee "modules\isofs\isofs.c" "dirTocEntry", "cdvdman_locatefile"
    Build iso ingame "modules\iopcore\cdvdman\searchfile.c" "cdvdman_locatefile" -Source2 "modules\iopcore\cdvdman\internal.h" -Funcs2 "dirTocEntry" -Defines "/DINGAME"
    foreach ($t in "ok", "zero", "xa", "long") {
        Test "isofs locatefile $t" iso-ee $t
        Test "cdvdman locatefile $t" iso-ingame $t
    }
}

if (Want "find") {
    Build find t "modules\isofs\isofs.c" "dirTocEntry", "cdvdman_trimspaces", "cdvdman_locatefile", "cdvdman_findfile"
    foreach ($n in "\SYSTEM.CNF;1", "/SYSTEM.CNF;1", "\A_VERY_LONG_LEVEL2_FILENAME.TXT;1") { Test "isofs findfile $n" find-t $n }
}

if (Want "zso") {
    $zsoDefine = if ((Get-Content -Raw (Join-Path $Opl "modules\isofs\zso.c")).Contains("int ziso_init(")) { "/DINIT_RETURNS_INT" } else { "" }
    Build zso t "modules\isofs\isofs.c" "longLseek", "read_raw_data" -Source2 "modules\isofs\zso.c" -Funcs2 "ziso_init" -Defines $zsoDefine
    foreach ($a in "0", "11", "12", "255") { Test "zso align $a" zso-t (Join-Path $inputs "zero.bin"), $a }
}

if (Want "cnf") {
    Build cnf t "src\ps2cnf.c" "CNF_LEN_MAX", "CNFGetToken", "CNFAdvanceLine", "CNFGetKey", "ps2cnfGetBootFile" -Patch 'fopen(path, "r")=>fopen(path, "rb")'
    foreach ($t in "cnf_ok", "cnf_no_eq") { Test "ps2cnf $t" cnf-t (Join-Path $inputs "$t.cnf") }
}

if (Want "sb") {
    Build sb t "src\supportbase.c" "game_list_t", "game_cache_list", "sbReadList", "updateISOGameList" -Defines "/DHAVE_SBREADLIST /DHAVE_UPDATE"
    foreach ($c in "ul64 0", "ul65 0", "ul129 0", "ul65 2", "ul129 2") {
        $a = $c.Split(" ")
        Test "sbReadList $c" sb-t "ul", (Join-Path $inputs $a[0]), $a[1]
    }
    foreach ($c in "2 2", "3 3 swap", "1 2") { Test "updateISOGameList $c" sb-t (@("cache", (Join-Path $inputs "cachedir")) + $c.Split(" ")) }
}

if (Want "name") {
    Build name t "src\supportbase.c" "isValidIsoName"
    Test "isValidIsoName" name-t "..", "x.iso", "SLUS_200.62.Title.iso"
}

if (Want "crc") {
    Build crc t "src\system.c" "USBA_crc32"
    $names = '""', "41", "4772616e64205468656674204175746f20494949", "c8e3f0e0", "d098d0b3d180d0b0"
    Test "USBA_crc32" crc-t $names
    "{0,-34} ref:   {1}" -f "", ((python -I (Join-Path $here "usba_ref.py") @names) -join " | ")
}

if (Want "cfg") {
    # void * arithmetic in CheckPS2Logo() is a GCC extension
    $cfgDefine = if ((Get-Content -Raw (Join-Path $Opl "src\config.c")).Contains("size_t prefixmax")) { "/DPREFIX_MAX" } else { "" }
    Build cfg t "src\config.c" "strToColor", "isWS", "splitAssignment", "parsePrefix", "configReadLegacyIP" -Source2 "src\util.c" -Funcs2 "openFileBuffer", "readFileBuffer", "closeFileBuffer", "min", "fromHex", "CheckPS2Logo" -Patch "void *buffer = logo;=>u8 *buffer = logo;" -Defines $cfgDefine
    Test "readFileBuffer plain cfg" cfg-t "lines", (Join-Path $inputs "cfg_plain.cfg")
    Test "readFileBuffer '#' last line, no LF" cfg-t "lines", (Join-Path $inputs "cfg_comment_end.cfg")
    Test "readFileBuffer read() fails" cfg-t "lines", (Join-Path $inputs "cfg_plain.cfg"), "readerror"
    Test "strToColor #112233" cfg-t "color", "#112233"
    Test "strToColor #11223344556677" cfg-t "color", "#11223344556677"
    Test "splitAssignment 40-char key" cfg-t "split", ("k" * 40 + "=value")
    Test "parsePrefix 60-char prefix" cfg-t "prefix", ("A" * 60 + ": no equals sign")
    Test "configReadLegacyIP 300 bytes" cfg-t "legacyip", (Join-Path $inputs "IPCONFIG.DAT")
    Test "CheckPS2Logo HDD" cfg-t "logo"
}

if (Want "cheatsel") {
    Build cheatsel t "src\cheatconfig.c" "normalizeCheatName", "cheatConfigIsMasterCode", "cheatNamesMatch", "cheatConfigLoadSelections", "cheatConfigSaveSelections"
    Test "cheat picks, distinct names" cheatsel-t "+Mastercode", "+Infinite Health", "Max Money", "+Moon Jump"
    Test "cheat picks, duplicate names" cheatsel-t "+Mastercode", "+Infinite Health", "+Infinite Health", "Max Money"
}

if (Want "vmc") {
    Build vmc t "src\system.c" "sysCheckVMC"
    $dir = Join-Path $build "vmc"
    New-Item -ItemType Directory -Force (Join-Path $dir "VMC") | Out-Null
    # card size in bytes (-1: no card), createSize as the VMC menu passes it
    foreach ($c in @(@(4194304, 8, "4 MiB card, create 8 MB"), @(8650752, 8, "PCSX2 .ps2 size, create 8 MB"), @(8388608, 8, "8 MiB card, create 8 MB"), @(-1, 8, "no card, create 8 MB"), @(4194304, -1, "4 MiB card, delete"))) {
        foreach ($v in "plain", "asan") {
            $card = Join-Path $dir "VMC\keep.bin"
            Remove-Item $card -ErrorAction SilentlyContinue
            if ($c[0] -ge 0) { [IO.File]::WriteAllBytes($card, (New-Object byte[] $c[0])) }
            $exe = if ($v -eq "plain") { "vmc-t" } else { "vmc-t-asan" }
            "{0,-34} {1}: {2}" -f $(if ($v -eq "plain") { "sysCheckVMC " + $c[2] } else { "" }), $v.PadRight(5), (Run $exe $dir, "keep", "$($c[1])")
        }
    }
}

if ($Images) {
    Build gameid t "src\supportbase.c" "GetStartupExecName" -Source2 "modules\isofs\isofs.c" -Funcs2 "dirTocEntry", "cdvdman_trimspaces", "cdvdman_locatefile" -Source3 "src\ps2cnf.c" -Funcs3 "CNF_LEN_MAX", "CNFGetToken", "CNFAdvanceLine", "CNFGetKey", "ps2cnfGetBootFile" -Patch 'fopen(path, "r")=>fopen(path, "rb")'
    foreach ($f in Get-ChildItem $Images -File | Where-Object { $_.Extension -in ".iso" }) { Test "gameid $($f.Name)" gameid-t $f.FullName }
}
