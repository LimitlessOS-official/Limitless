param(
    [Parameter(Mandatory=$true)]
    [string]$InputDir
)

$rootImg = Join-Path $InputDir "root.img"
if (!(Test-Path $rootImg)) { Write-Error "root.img not found in $InputDir"; exit 1 }

# Compute SHA-256 of root.img
$sha = [System.Security.Cryptography.SHA256]::Create()
$fs = [System.IO.File]::OpenRead($rootImg)
try {
    $hash = $sha.ComputeHash($fs)
} finally {
    $fs.Close()
}

# HMAC-SHA256 using fixed key to match kernel UPDATE_SIGN_KEY
$keyBytes = [byte[]](0x42,0x4f,0x58,0x2d,0x4b,0x45,0x59,0x21,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22)
$hmac = New-Object System.Security.Cryptography.HMACSHA256($keyBytes)
$sig = $hmac.ComputeHash($hash)

# Write signature hex to ota.sig
$outPath = Join-Path $InputDir "ota.sig"
$hex = ($sig | ForEach-Object { $_.ToString("x2") }) -join ""
Set-Content -LiteralPath $outPath -Value $hex -NoNewline
Write-Host "Wrote $outPath"
